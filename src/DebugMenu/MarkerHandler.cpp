#include "MarkerHandler.h"
#include "DebugMenu.h"


namespace DebugMenu
{
	MarkerHandler::MarkerHandler()
	{
		logger::debug("Initialized MarkerHandler");
	}

	void MarkerHandler::InitPostDataLoaded()
	{
		CopyEffectShaderMaterial("marker_light.nif"s, "marker_light:0"s, lightBulbMaterial);
	}

	void MarkerHandler::CopyEffectShaderMaterial(const std::string& a_filepath, const std::string& a_shapeName, RE::BSEffectShaderMaterial*& a_materialOut)
	{
		RE::NiPointer<RE::NiNode> model;
		RE::BSModelDB::DBTraits::ArgsType args;
		RE::BSResource::ErrorCode errorCode = RE::BSModelDB::Demand(a_filepath.c_str(), model, args);
		if (errorCode == RE::BSResource::ErrorCode::kNone && model)
		{
			if (auto shape = model->AsNode()->GetObjectByName(a_shapeName))
			{
				if (auto shaderProperty = netimmerse_cast<RE::BSEffectShaderProperty*>(shape->AsGeometry()->GetGeometryRuntimeData().properties[RE::BSGeometry::States::kEffect].get()))
				{
					// inspired by https://github.com/powerof3/LightPlacer/blob/830e15d7e5fa1fbccb7427852024458564eccc0b/src/LightData.cpp#L396
					if (const auto shaderMaterial = static_cast<RE::BSEffectShaderMaterial*>(shaderProperty->material))
					{
						if (a_materialOut = static_cast<RE::BSEffectShaderMaterial*>(shaderMaterial->Create()))
						{
							a_materialOut->CopyMembers(shaderMaterial);
						}
					}
				}
			}
		}
	}

	float MarkerHandler::GetRange()
	{
		return MCM::settings::markersRange;
	}

	void MarkerHandler::Reset()
	{
		HideAllMarkers();
	}

	void MarkerHandler::Draw()
	{
		if (MCM::settings::showMarkers) 
		{
			DrawMarkers();
		}

	}

	void MarkerHandler::DrawMarkers()
	{
		auto origin = GetCenter();

		// Debug duplicates
		
		//int numberOfDuplicates = 0;
		//std::map<RE::FormID, uint32_t> formIDcounts;
		//for (const auto& marker : visibleMarkers)
		//{
		//	if (formIDcounts.contains(marker->ref->formID))
		//	{
		//		formIDcounts[marker->formID]++;
		//		logger::info("  {:X}", marker->formID);
		//		numberOfDuplicates++;
		//	}
		//	else
		//		formIDcounts[marker->formID] = 1;
		//}
		//
		//logger::info("currently {}/{} markers are drawn. {} Duplicates", visibleMarkers.size(), visibleMarkersIDs.size(), numberOfDuplicates);
		//
		//logger::info("currently {} markers are drawn", visibleMarkers.size());


		if (MCM::settings::updateVisibleMarkers)
		{
			MCM::settings::updateVisibleMarkers = false;
			HideAllMarkers();

		#ifdef TRACEOBJECTS
			logger::debug("Update visible markers");
		#endif
		}

		for (int i = visibleMarkers.size() - 1; i > -1; i--)
		{
			std::unique_ptr<Marker>& marker = visibleMarkers[i];

			if (!IsMarkerLoaded(marker)) // whenever worldspaces change, the reference is still loaded in memory, and may still be in range, but there is no reason to keep them in the list
			{
				HideMarker(i);
				continue;
			}

			float dx = origin.x - marker->ref->GetPositionX();
			float dy = origin.y - marker->ref->GetPositionY();

			float range = GetRange();
			if (marker->drawWhenFar) range = drawWhenFarRange;

			if (dx * dx + dy * dy > range * range)
			{
				HideMarker(i);
			}
		}

		Utils::ForEachCellInRange(origin, drawWhenFarRange, [&](const RE::TESObjectCELL* a_cell)
		{
			a_cell->ForEachReference([&](RE::TESObjectREFR* ref)
			{
				if (!ref) return RE::BSContainer::ForEachResult::kContinue;

				#ifdef TRACEOBJECTS

					auto base = ref->GetBaseObject();
					auto pos = ref->GetPosition();

					if (ref->formID == debugFormID)
					{
						logger::debug("");
						logger::debug("Tracing object {:X} edid: <{}> address: {:X}", ref->formID, ref->GetFormEditorID(), reinterpret_cast<uintptr_t>(ref));
						logger::debug(" |-IsInitiallyDisabled? {}; Disabled? {}", ref->IsInitiallyDisabled(), ref->IsDisabled());
						logger::debug(" |-Base object: {:X} edid: <{}>", base->formID, base->GetFormEditorID());
						logger::debug(" |-Cell: {:X} edid: <{}> address: {:X}", a_cell->formID, a_cell->GetFormEditorID(), reinterpret_cast<uintptr_t>(a_cell));
						logger::debug(" |-|-Cell game flags: {:016b}", a_cell->cellGameFlags);
						logger::debug(" |-|-Cell state: {:08b}", a_cell->cellState.underlying());
						logger::debug(" |-|-Cell form flags {:032b}", a_cell->formFlags);
						logger::debug(" |-|-Cell form flags {:016b}", a_cell->inGameFormFlags.underlying());
						logger::debug(" |-|-Cell is initialized? {}", a_cell->IsInitialized());
					}
				#endif


				bool shouldMarkerBeDrawnWhenFar = ShouldMarkerBeDrawnWhenFar(ref);

				float range = GetRange();
				if (shouldMarkerBeDrawnWhenFar) range = drawWhenFarRange;

				float dx = origin.x - ref->GetPositionX();
				float dy = origin.y - ref->GetPositionY();

				if (dx * dx + dy * dy > range * range) return RE::BSContainer::ForEachResult::kContinue;

				#ifdef TRACEOBJECTS
					if (ref->formID == debugFormID) logger::debug(" |-Object withing range");
				#endif

				uint32_t numberOfDrawnMarkers = visibleMarkers.size();

				ShowMarker(ref);


				if (shouldMarkerBeDrawnWhenFar && visibleMarkers.size() > numberOfDrawnMarkers)
				{
					visibleMarkers[numberOfDrawnMarkers]->drawWhenFar = true;
				}

				return RE::BSContainer::ForEachResult::kContinue;
			});
		});
	}

	void MarkerHandler::AddVisibleMarker(RE::TESObjectREFR* a_ref, RE::BSFixedString a_markerName, bool a_cullWhenHiding)
	{
		#ifdef TRACEOBJECTS
			if (a_ref->formID == debugFormID)
			{
				debugMarkerIndex = visibleMarkers.size();
				logger::debug(" |-|-AddVisibleMarker; Marker name: {}; Marker index = {}", a_markerName, debugMarkerIndex);

			}
		#endif

		visibleMarkers.push_back(std::make_unique<Marker>(a_ref, a_markerName));
		visibleMarkers.back().get()->cullWhenHiding = a_cullWhenHiding;
		visibleMarkersIDs.insert(a_ref->formID);
	}

	void MarkerHandler::RemoveVisibleMarker(uint32_t a_markerIndex)
	{
		visibleMarkersIDs.erase(visibleMarkers[a_markerIndex]->formID);
		visibleMarkers.erase(visibleMarkers.begin() + a_markerIndex);
	}

	void MarkerHandler::HideAllMarkers()
	{
		for (int i = visibleMarkers.size() - 1; i > -1; i--)
		{
			HideMarker(i);
		}
	}

	void MarkerHandler::HideMarker(uint32_t a_markerIndex)
	{
		#ifdef TRACEOBJECTS
			if (a_markerIndex == debugMarkerIndex)
			{
				logger::debug(" |-Hiding Marker; MarkerIndex: {}; VisibleMarkersSize? {}", a_markerIndex, visibleMarkers.size());
				if (a_markerIndex < visibleMarkers.size())
				{
					logger::debug(" |-|-IsMarkerLoaded? {}", IsMarkerLoaded(visibleMarkers[a_markerIndex]));
				}
			}
		#endif

		if (a_markerIndex >= visibleMarkers.size()) return;

		std::unique_ptr<Marker>& marker = visibleMarkers[a_markerIndex];



		if (!IsMarkerLoaded(marker))
		{
			RemoveVisibleMarker(a_markerIndex);
			return;
		}

		marker->SetDefaultState();

		#ifdef TRACEOBJECTS
			if (marker->ref->formID == debugFormID)
			{
				logger::debug(" |-Hiding Marker. Culltree after setting default:");
				marker->PrintCullTree(" |-|-");
			}
		#endif

		RemoveVisibleMarker(a_markerIndex);
	}

	void MarkerHandler::ShowMarker(RE::TESObjectREFR* a_ref)
	{
		#ifdef TRACEOBJECTS
			if (a_ref->formID == debugFormID) logger::debug(" |-Showing object");
		#endif

		if (auto baseObject = a_ref->GetBaseObject())
		{
			#ifdef TRACEOBJECTS
				if (a_ref->formID == debugFormID)
				{
					logger::debug(" |-|-Formtype: {}", baseObject->GetFormType());
					logger::debug(" |-|-ShouldBeDrawn: {}", ShouldMarkerBeDrawn(a_ref));
				}
			#endif
			// if the object itself is the marker (like x-markers), call ShouldMarkersBeDrawn first
			// else, if the marker is attached to the object, check the setting inside the corresponding ShowHide<...>Marker function (typically added to the "if (!a_show) return;")
			switch (baseObject->GetFormType())
			{
				case RE::FormType::Furniture:
				{
					if (MCM::settings::GetShowFurnitureMarkers())
						ShowFurnitureMarker(a_ref);
					break;
				}
				case RE::FormType::Light:
				{
					if (MCM::settings::GetShowLightMarkers())
						ShowLightMarker(a_ref);
					break;
				}
				case RE::FormType::Sound:
				{
					if (MCM::settings::GetShowSoundMarkers())
						ShowSoundMarker(a_ref);
					break;
				}
				case RE::FormType::Door:
				{
					if (MCM::settings::GetShowDoorTeleportMarkers())
						ShowDoorTeleportMarker(a_ref);
					break;
				}
				case RE::FormType::MovableStatic:
				case RE::FormType::IdleMarker:
				case RE::FormType::Activator:
				case RE::FormType::NPC:
				case RE::FormType::Hazard:
				case RE::FormType::TextureSet:
				case RE::FormType::Static:
				{
					if (ShouldMarkerBeDrawn(a_ref))
					{
						#ifdef TRACEOBJECTS
							if (a_ref->formID == debugFormID) logger::debug(" |-Calling ShowHideStaticMarker");
						#endif
						ShowStaticMarker(a_ref);
					}
					break;
				}
			}
		}
	}

	// Main 3D should have its world pos = its local pos = the position it should be
	// its children can then have local pos = 0 if its ontop of the main 3D, otherwise an offset

	MarkerHandler::MarkerInfo MarkerHandler::GetMarkerInfo(RE::TESObjectREFR* a_ref)
	{
		MarkerInfo info;

		auto baseObject = a_ref->GetBaseObject();

		if (baseObject->GetFormType() == RE::FormType::Sound)
		{
			info.path = "marker_sound.nif";
			info.name = "SoundMarkerVis";
		}

		std::string path = ""s;

		switch (baseObject->GetFormType())
		{
			case RE::FormType::MovableStatic:
				if (const auto& movableStatic = baseObject->As<RE::BGSMovableStatic>()) path = movableStatic->model;
				break;
			case RE::FormType::IdleMarker:
				if (const auto& idleMarker = baseObject->As<RE::BGSIdleMarker>()) path = idleMarker->model;
				break;
			case RE::FormType::Activator:
				if (const auto& activator = baseObject->As<RE::TESObjectACTI>()) path = activator->model;
				break;
			case RE::FormType::Hazard:
				if (const auto& hazard = baseObject->As<RE::BGSHazard>()) path = hazard->model;
				break;
			case RE::FormType::Static:
				if (const auto& stat = baseObject->As<RE::TESObjectSTAT>()) path = stat->model;
				break;
		}
		if (!path.empty())
		{
			info.path = path;
			std::string filename = path;
			if (path.rfind("\\") != std::string::npos)
			{
				filename = std::string(path.substr(path.rfind("\\") + 1));
			}
			info.name = filename.substr(0, filename.size() - 4); // remove '.nif' from name
		}

		return info;
	}

	void MarkerHandler::Marker::SetDefaultState()
	{
		if (auto obj = ref->Get3D())
		{
			//Utils::SetNodeTreeDefaultAppCull(obj);

			if (auto node = obj->AsNode())
			{
				Utils::DetachChildrenByName(ref->Get3D()->AsNode(), markerName);

				bool cullNode = true;
				bool cullEditorMarker = true;
				if (!cullWhenHiding) cullNode = false;
				Utils::CullNode(node, cullNode, cullEditorMarker); // MUST be after children have been detached
			}
		}
	}

	RE::NiNode* MarkerHandler::TrySet3DByName(RE::TESObjectREFR* a_ref, const char* a_modelName, const RE::BSFixedString a_markerName)
	{
		RE::NiPointer<RE::NiNode> markerModel_;
		RE::BSModelDB::DBTraits::ArgsType args;
		RE::BSResource::ErrorCode errorCode = RE::BSModelDB::Demand(a_modelName, markerModel_, args);

		if (errorCode != RE::BSResource::ErrorCode::kNone || !markerModel_ || !markerModel_.get()) return nullptr;

		auto clone = markerModel_->Clone();

		RE::NiNode* node = clone->AsNode();
		node->name = a_markerName;


		if (!node) return nullptr;

		a_ref->Set3D(node, true);
		a_ref->Update3DPosition(true);
		node->CullNode(true); // node must be culled by default, so the culltree will know that it should be hidden when deactivated

		if (!node->parent)
		{
			auto cellStaticNode = Utils::GetCellStaticNode(a_ref->GetParentCell());
			if (cellStaticNode)
			{
				Utils::AttachChildNode(cellStaticNode, node);
			}
		}

		return node;

	}

	RE::NiNode* MarkerHandler::GetNodeFromRef(RE::TESObjectREFR* a_ref)
	{
		if (auto obj = a_ref->Get3D())
		{
			if (auto node = obj->AsNode())
			{
				#ifdef TRACEOBJECTS
					if (a_ref->formID == debugFormID) logger::debug(" |-|-|-ref has 3D");
				#endif
				return node;
			}
		}
		else
		{
			MarkerInfo info = GetMarkerInfo(a_ref);
			if (!info.path.empty())
			{
				#ifdef TRACEOBJECTS
					if (a_ref->formID == debugFormID) logger::debug(" |-|-|-getting ref 3D from path: {}; name: {}", info.path, info.name);
				#endif
				return TrySet3DByName(a_ref, info.path.c_str(), info.name);
			}
		}
		#ifdef TRACEOBJECTS
			if (a_ref->formID == debugFormID) logger::debug(" |-|-ref has no 3D");
		#endif
		return nullptr;
	}

	bool MarkerHandler::IsMarkerLoaded(const std::unique_ptr<Marker>& a_marker)
	{
		return a_marker->ref && a_marker->ref->parentCell && a_marker->ref->Is3DLoaded();
	}

	bool MarkerHandler::ShowNodeIfNeeded(RE::TESObjectREFR* a_ref)
	{
		auto node = a_ref->Get3D()->AsNode();
		if (ShouldMarkerBeDrawn(a_ref))
		{
			Utils::CullNode(node, false);
			return true;
		}
		return false;
	}

	void MarkerHandler::ShowFurnitureMarker(RE::TESObjectREFR* a_ref)
	{
		#ifdef TRACEOBJECTS
			if (a_ref->formID == debugFormID) logger::debug(" |-|-ShowFurnitureMarker");
		#endif

		auto node = GetNodeFromRef(a_ref);
		if (!node) return;

		#ifdef TRACEOBJECTS
			if (a_ref->formID == debugFormID)
			{
				logger::debug(" |-|-|-node? {}; HasChildrenOfName({})? {}", node ? true : false, furnitureMarkerName, Utils::HasChildrenOfName(node, furnitureMarkerName));
				logger::debug(" |-|-|-|-Name: {}", node->name);

			}
		#endif

		RE::NiExtraData* furnitureMarkerNode_ = a_ref->Get3D()->GetExtraData("FRN");
		RE::BSFurnitureMarkerNode* furnitureMarkerNode = reinterpret_cast<RE::BSFurnitureMarkerNode*>(furnitureMarkerNode_);
		if (!furnitureMarkerNode) return;

		uint8_t markerType = 0;

		if (!Utils::HasChildrenOfName(node, furnitureMarkerName))
		{
			for (const auto& marker : furnitureMarkerNode->markers)
			{
				RE::TESObjectSTAT* markerStatic = nullptr;

				if (marker.animationType.any(RE::BSFurnitureMarker::AnimationType::kSit) && MCM::settings::GetShowSitMarkers())
				{
					markerStatic = RE::TESForm::LookupByID<RE::TESObjectSTAT>(0x64);
					markerType |= 0b001;
				}

				else if (marker.animationType.any(RE::BSFurnitureMarker::AnimationType::kSleep) && MCM::settings::GetShowSleepMarkers())
				{
					markerStatic = RE::TESForm::LookupByID<RE::TESObjectSTAT>(0x65);
					markerType |= 0b010;
				}

				else if (marker.animationType.any(RE::BSFurnitureMarker::AnimationType::kLean) && MCM::settings::GetShowLeanMarkers())
				{
					markerStatic = RE::TESForm::LookupByID<RE::TESObjectSTAT>(0x66);
					markerType |= 0b100;
				}

				if (!markerStatic) continue;

				RE::BSFixedString markerModelPath = markerStatic->model;

				if (markerModelPath.empty()) continue;

				RE::NiAVObject* markerModel = nullptr;


				auto pos = a_ref->GetPosition();

				if (!Utils::TryAttachChildByName(node, markerModelPath.c_str(), furnitureMarkerName, markerModel)) continue;

				//RE::NiUpdateData updateData;
				//updateData.flags = static_cast<RE::NiUpdateData::Flag>(0x2); // seems to be flagged 2 when run by the game
				//updateData.time = 0.0f;

				//markerModel->UpdateDownwardPass(updateData, 0); // necessary for most markers to show (and to get proper transforms), args = 0

				markerModel->local.translate = marker.offset;
				markerModel->local.rotate = RE::NiMatrix3(0.0f, 0.0f, marker.heading);

				// apparently, not all local transformations move the marker mesh, unless this is called :^)
				a_ref->InitNonNPCAnimation(*markerModel->AsNode());

				/*
				// draw cubes

				RE::NiAVObject* markerModel2 = nullptr;
				if (!TryAttachChildByName(node, "markers\\rigidbodydummy.nif", furnitureMarkerName, markerModel2)) continue;
				if (!markerModel2) continue;
				markerModel2->GetObjectByName("EditorMarker")->CullNode(false);
				markerModel2->local.translate = marker.offset;
				markerModel2->local.rotate = RE::NiMatrix3(45.0f, 35.0f, 30.0f);
				a_ref->InitNonNPCAnimation(*markerModel2->AsNode()); // the square moves to the bounds of the ref (or not)

				//a_ref->Set3D(markerModel->Clone());
				//a_ref->Update3DPosition(true);
				*/

			}
		}

		uint32_t numberOfAttachedMarkers = Utils::HowManyChildrenOfName(node, furnitureMarkerName);

		#ifdef TRACEOBJECTS
			if (a_ref->formID == debugFormID)
			{
				logger::debug(" |-|-|-HasAttachedMarkers? {}; does marker already exist? {}; makeCullTreeFromFile? {}", hasAttachedMarkers, markerAlreadyExist, makeCullTreeFromFile);
			}
		#endif

		if (numberOfAttachedMarkers > 0 && !visibleMarkersIDs.contains(a_ref->formID))
		{
			//bool isNodeShown = ShowNodeIfNeeded(a_ref); // check if marker should be drawn (such as wallLeanMarker)
			bool cullNodeWhenHiding = false;
			if (node->GetObjectByName("EditorMarker"))
			{
				// If the EditorMarker is the only node (besides the attached furniture marker nodes) then the main node
				// should be hidden, when the EditorMarker is culled, otherwise not.

				if (node->GetChildren().size() == 1 + numberOfAttachedMarkers)
				{
					cullNodeWhenHiding = true;
				}
				Utils::CullNode(node, false);

			}
			AddVisibleMarker(a_ref, furnitureMarkerName, cullNodeWhenHiding);
		}
	}

	void MarkerHandler::ShowStaticMarker(RE::TESObjectREFR* a_ref)
	{
		RE::NiNode* node = GetNodeFromRef(a_ref);
		if (!node) return;

		auto baseObject = a_ref->GetBaseObject();

		#ifdef TRACEOBJECTS
			if (a_ref->formID == debugFormID)
			{
				logger::debug(" |-|-Inside ShowHideStaticMarker");
				logger::debug(" |-|-IsNodeVisible: {}", Utils::IsNodeVisible(node));

			}
		#endif


		if (!visibleMarkersIDs.contains(a_ref->formID))
		{
			Utils::CullNode(node, false);
			AddVisibleMarker(a_ref);
		}
		return;
	}

	void MarkerHandler::ShowDoorTeleportMarker(RE::TESObjectREFR* a_ref)
	{
		auto teleportDoorRef = a_ref->extraList.GetTeleportLinkedDoor().get();
		if (!teleportDoorRef) return;

		RE::ExtraTeleport* teleportExtraData = teleportDoorRef->extraList.GetByType<RE::ExtraTeleport>();
		if (!teleportExtraData) return;

		auto node = GetNodeFromRef(a_ref);
		if (!node) return;

		if (!Utils::HasChildrenOfName(node, doorTeleportMarkerName))
		{
			auto localPosition = teleportExtraData->teleportData->position - a_ref->GetPosition();
			auto localRotation = teleportExtraData->teleportData->rotation;

			RE::NiAVObject* markerModel = nullptr;
			if (!Utils::TryAttachChildByName(node, "MarkerTeleport.nif", doorTeleportMarkerName, markerModel)) return;

			auto parentRotation = a_ref->Get3D()->world.rotate;

			// When the marker is attached to the door, it will be rotated according to the doors rotation. So we inverse the rotation of the door here (R(theta)^-1 = R(theta)^T)
			markerModel->local.translate = (parentRotation.Transpose() * localPosition) / a_ref->GetScale();
			markerModel->local.rotate = parentRotation.Transpose() * RE::NiMatrix3(localRotation);
		}

		if (!visibleMarkersIDs.contains(a_ref->formID))
		{
			bool isNodeShown = ShowNodeIfNeeded(a_ref);
			AddVisibleMarker(a_ref, doorTeleportMarkerName, isNodeShown);
		}
	}

	void MarkerHandler::ShowLightMarker(RE::TESObjectREFR* a_ref)
	{
		auto node = GetNodeFromRef(a_ref);
		if (!node) return;

		RE::TESObjectLIGH* light = a_ref->GetBaseObject()->As<RE::TESObjectLIGH>();

		const char* modelName = "";
		const char* shapeName = "";

		if (light)
		{
			if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kHemiShadow)) // WhiterunDragonreachBasement
			{
				if (!MCM::settings::GetShowShadowHemiMarkers()) return;
				modelName = "marker_halfomni.nif";
				shapeName = "marker_halfomni:0";
			}
			else if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kSpotShadow)) // PotemasCatacombs02
			{
				if (!MCM::settings::GetShowShadowSpotMarkers()) return;
				modelName = "marker_spotlight.nif";
				shapeName = "marker_spotlight:0";
			}
			else if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kOmniShadow))
			{
				if (!MCM::settings::GetShowShadowOmniMarkers()) return;
				modelName = "marker_lightshadow.nif";
				shapeName = "marker_lightshadow:0";
			}
			else
			{
				if (!MCM::settings::GetShowOmniMarkers()) return;
				modelName = "marker_light.nif";
				shapeName = "marker_light:0";
			}
		}

		if (MCM::settings::showInfoOnHover && MCM::settings::showMarkerInfo)
		{
			auto infoPosition = a_ref->GetPosition();
			
			auto metaData = CreateMetaData();
			metaData->cell = a_ref->parentCell;
			metaData->ref = a_ref;
			metaData->infoType = InfoType::kLightMarker;
			GetDrawHandler()->DrawPoint(infoPosition, 20.0f, MCM::settings::lightBulbInfoColor, MCM::settings::markerInfoAlpha, metaData);
		}

		if (Utils::HasChildrenOfName(node, lightMarkerName)) return;

		RE::NiAVObject* markerModel = nullptr;
		if (!Utils::TryAttachChildByName(node, modelName, lightMarkerName, markerModel)) return;
		a_ref->InitNonNPCAnimation(*markerModel->AsNode());

		//some lights will not be visible without bounds
		node->UpdateWorldBound();

		if (!visibleMarkersIDs.contains(a_ref->formID))
		{
			AddVisibleMarker(a_ref, lightMarkerName, false);
		}

		if (auto shape = markerModel->AsNode()->GetObjectByName(shapeName))
		{
			if (auto shaderProperty = netimmerse_cast<RE::BSEffectShaderProperty*>(shape->AsGeometry()->GetGeometryRuntimeData().properties[RE::BSGeometry::States::kEffect].get()))
			{
				if (lightBulbMaterial)
				{
					shaderProperty->lastRenderPassState = std::numeric_limits<std::int32_t>::max();
					shaderProperty->SetMaterial(lightBulbMaterial, true);
					shaderProperty->SetupGeometry(shape->AsGeometry());
					shaderProperty->FinishSetupGeometry(shape->AsGeometry());
					shaderProperty->SetMaterialAlpha(MCM::settings::lightBulbAplha / 100.0f * lightBulbAlphaMax); // alpha of 1 is barely visible. above 2.25 it gets wacky	

				}
			}
			if (MCM::settings::lightBulbAplha > 99.5f && lightBulbMaterial)
			{
				if (auto alphaProperty = netimmerse_cast<RE::NiAlphaProperty*>(shape->AsGeometry()->GetGeometryRuntimeData().properties[RE::BSGeometry::States::kProperty].get()))
				{
					alphaProperty->SetAlphaBlending(false);
					alphaProperty->SetAlphaTesting(true);
					alphaProperty->alphaThreshold = 40;
				}
			}
		}
	}

	void MarkerHandler::ShowSoundMarker(RE::TESObjectREFR* a_ref)
	{
		if (MCM::settings::showInfoOnHover && MCM::settings::showMarkerInfo)
		{
			auto pos = a_ref->GetPosition();

			auto metaData = CreateMetaData();
			metaData->cell = a_ref->parentCell;
			metaData->ref = a_ref;
			metaData->infoType = InfoType::kSoundMarker;
			GetDrawHandler()->DrawPoint(pos, 20.0f, MCM::settings::soundMarkerInfoColor, MCM::settings::markerInfoAlpha, metaData);
		}

		auto node = GetNodeFromRef(a_ref);

		if (!Utils::IsNodeTreeVisible(node) && !visibleMarkersIDs.contains(a_ref->formID))
		{
			node->CullNode(false);
			AddVisibleMarker(a_ref);
		}
	}

	bool MarkerHandler::ShouldMarkerBeDrawnWhenFar(RE::TESObjectREFR* a_ref)
	{
		if (auto baseObject = a_ref->GetBaseObject())
		{
			bool drawWhenFar = false;
			ShouldMarkerBeDrawn(a_ref, drawWhenFar);
			return drawWhenFar;
		}
		return false;
	}

	bool MarkerHandler::ShouldMarkerBeDrawn(RE::TESObjectREFR* a_ref)
	{
		bool dummy = false;
		return ShouldMarkerBeDrawn(a_ref, dummy);
	}

	bool MarkerHandler::ShouldMarkerBeDrawn(RE::TESObjectREFR* a_ref, bool& a_showWhenFar)
	{
		auto baseObject = a_ref->GetBaseObject();
		switch (baseObject->formID)
		{
			case 0x02: // TravelMarker
			case 0x03: // NorthMarker
			case 0x05: // DivineMarker
			case 0x06: // TempleMarker
			case 0x10:	// MapMarker
			case 0x12: // HorseMarker
			case 0x15: // MultiBoundMárker
			case 0x1F: // RoomMarker
			case 0x20: // PortalMarker
			case 0x21: // CollisionMarker
			case 0x33: // Radiationmarker
			case 0x61: // CellWaterCurrentMarker
			case 0x62: // WaterCurrentMarker
			case 0xA0: // AnimInteractionMarker
			case 0xC4: // WaterCurrentZoneMarker
			case 0x1C035: // ComplexSceneMARKER
			case 0xF077B: // RoadMarker
				return MCM::settings::GetShowOtherStaticMarkers();
			case 0x32: // COCHeadingMarker
			case 0x34: // HeadingMarker
				return MCM::settings::GetShowHeadingMarkers();
			case 0x3B: // XMarker
				return MCM::settings::GetShowXMarkers();
			case 0x1E595:  // DragonPerchTower
			case 0x7A37D:	// MQ104FortMeetSoldier4Marker
			case 0xAA934:  // DragonPerchRockL02
			case 0xBFB04:  // SoldierWallIdle
			case 0x103442: // CartFurnitureDriver
			case 0x105D4D: // CartFurnitureHorse
				return MCM::settings::GetShowSitMarkers(); // || MCM::settings::GetShowDragonMarkers() // complicated, sinces the dragons are furniture
			case 0x52FF5: // WallLeanMarker
				return MCM::settings::GetShowLeanMarkers();
			case 0x138C0: // DragonMarker
				return MCM::settings::GetShowDragonMarkers();
			case 0x3DF55: // DragonMarkerCrashStrip
				a_showWhenFar = true;
				return MCM::settings::GetShowDragonMarkers();
			case 0x68E43: // DragonMoundBaseAlt
				return MCM::settings::GetShowOtherActivatorMarkers() || MCM::settings::GetShowDragonMarkers();
			case 0x22201: // CritterLandingMarker_Small
			case 0x6B2FF: // critterSpawnPond_Shallow
			case 0x6B44D: // critterSpawnPond_Deep
			case 0x6B45B: // critterSpawnPond_Small
			case 0x6B45C: // critterSpawnDragonflies24x7
			case 0xC2D47: // DoNotPlaceSmallCritterLandingMarkerHelper
			case 0xC51FF: // critterSpawnInsects_Few
			case 0xC5208: // critterSpawnInsects_Single
			case 0xC5209: // critterSpawnInsects_Many
				return MCM::settings::GetShowCritterMarkers();
			case 0x7776E: // FXMistLow01LongHalfVis
			case 0x7776F: // FXMistLow01Long
			case 0x77770: // FXMistLow02Rnd
			case 0x77771: // FXMistLow02RndHalfVis
			case 0x77772: // FXMistLow01
			case 0x77773: // FXMistLow01HalfVis
				return MCM::settings::GetShowMistMarkers();
			case 0x75DCB: // FXAmbBeamXbDustBig02
			case 0xAA8F5: // FXAmbBeamSlowFogBrt02
			case 0xAA8F6: // FXAmbBeamSlowFogBrt01
				return MCM::settings::GetShowLightBeamMarkers();
			case 0x16D4B: // FXSteamCrack
			case 0x1ACAC: // FXWaterFallSkirtSlope
			case 0x2BCA5: // FXSplashSmallParticles
			case 0x2BCA7: // FXSplashSmallParticlesLong
			case 0x2BCAB: // FXWaterfallThin512x128
			case 0x2BCC8: // FXWaterfallThin2048x128
			case 0x2EB0D: // FXGlowFillRoundDim
			case 0x2EB0E: // FXGlowFillRoundMid
			case 0x2EB0F: // FXGlowFillRoundXBrt
			case 0x2EB10: // FXGlowFillRoundXDim
			case 0x2EB11: // FXGlowFillRoundBrt
			case 0x30B39: // FXSteamBillow
			case 0x3103C: // FXSteamJetFast
			case 0x3103F: // FXSteamJet
			case 0x77775: // FXSmokeWispsLg1x1
			case 0x77776: // FXSmokeWispsLg2x1
			case 0x77777: // FXSmokeWispsLgVol01
			case 0xB09E4: // FXSmokeChimney01
			case 0xB09ED: // FXSmokeChimney02
			case 0xE4E22: // FXDweSteam01
			case 0x108E37: // FXAmbWaterSalmon018
				return MCM::settings::GetShowOtherMSTTMarkers();
			case 0x57A8C: // FireLgPlacedHazard
				return MCM::settings::GetShowHazardMarkers();
			case 0x80C32: // ImpactMarker01
				return MCM::settings::GetShowImpactMarkers();
				//case 0x1B37D : // FXRapids
			case 0x21513: // NorLever01
			case 0x27EF9: // GuardMarker
			case 0x812C6: // FireWeaponMarker
			case 0x10B035: // FCAmbWaterfallSalmon01
				return MCM::settings::GetShowOtherActivatorMarkers();
			case 0x2630D: // SkyrimCloudDistant01
			case 0x274B7: // SkyrimCloudDistant01_25
			case 0x274B8: // SkyrimCloudDistant01_50
			case 0x2754F: // SkyrimCloudDistant01_O
			case 0x274A6: // SkyrimCloudDistant01_O_25
			case 0x274B5: // SkyrimCloudDistant01_O_50
			case 0x2630E: // SkyrimCloudDistant02
			case 0x27499: // SkyrimCloudDistant02_25
			case 0x27483: // SkyrimCloudDistant02_50
			case 0x27481: // SkyrimCloudDistant02_O
			case 0x2747E: // SkyrimCloudDistant02_O_25
			case 0x2747D: // SkyrimCloudDistant02_O_50
			case 0x17521: // SkyrimCloudDistant03
			case 0x2747C: // SkyrimCloudDistant03_25
			case 0x2747A: // SkyrimCloudDistant03_50
			case 0x27472: // SkyrimCloudDistant03_O
			case 0x2746E: // SkyrimCloudDistant03_O_25
			case 0x2745A: // SkyrimCloudDistant03_O_50
			case 0x17520: // SkyrimCloudDistant04
			case 0x2744C: // SkyrimCloudDistant04_25
			case 0x27447: // SkyrimCloudDistant04_50
			case 0x2743B: // SkyrimCloudDistant04_O
			case 0x2743A: // SkyrimCloudDistant04_O_25
			case 0x27437: // SkyrimCloudDistant04_O_50
			case 0x1751F: // SkyrimCloudDistant05
			case 0x27432: // SkyrimCloudDistant05_25
			case 0x27430: // SkyrimCloudDistant05_50
			case 0x27426: // SkyrimCloudDistant05_O
			case 0x27425: // SkyrimCloudDistant05_O_25
			case 0x2740F: // SkyrimCloudDistant05_O_50
			case 0x1751E: // SkyrimCloudDistant06
			case 0x2740C: // SkyrimCloudDistant06_25
			case 0x27408: // SkyrimCloudDistant06_50
			case 0x273FC: // SkyrimCloudDistant06_O
			case 0x273FA: // SkyrimCloudDistant06_O_25
			case 0x273F5: // SkyrimCloudDistant06_O_50
			case 0x273F3: // SkyrimCloudShape01
			case 0x273E9: // SkyrimCloudShape01_25
			case 0x273E8: // SkyrimCloudShape01_50
			case 0x273E6: // SkyrimCloudShape01_O
			case 0x273E0: // SkyrimCloudShape01_O_25
			case 0x273DC: // SkyrimCloudShape01_O_50
			case 0x273D9: // SkyrimCloudShape02
			case 0x273D4: // SkyrimCloudShape02_25
			case 0x273C1: // SkyrimCloudShape02_50
			case 0x273BF: // SkyrimCloudShape02_O
			case 0x273B8: // SkyrimCloudShape02_O_25
			case 0x273B7: // SkyrimCloudShape02_O_50
			case 0x273B0: // SkyrimCloudShape03
			case 0x273AC: // SkyrimCloudShape03_25
			case 0x273AB: // SkyrimCloudShape03_50
			case 0x2738B: // SkyrimCloudShape03_O
			case 0x27349: // SkyrimCloudShape03_O_25
			case 0x27338: // SkyrimCloudShape03_O_50
			case 0x27323: // SkyrimCloudShape04
			case 0x2730F: // SkyrimCloudShape04_25
			case 0x272E5: // SkyrimCloudShape04_50
			case 0x272DB: // SkyrimCloudShape04_O
			case 0x272D3: // SkyrimCloudShape04_O_25
			case 0x272CC: // SkyrimCloudShape04_O_50
			case 0x272C4: // SkyrimCloudShape05
			case 0x272B3: // SkyrimCloudShape05_25
			case 0x272B2: // SkyrimCloudShape05_50
			case 0x272B1: // SkyrimCloudShape05_O
			case 0x272B0: // SkyrimCloudShape05_O_25
			case 0x272AF: // SkyrimCloudShape05_O_50
			case 0x272AE: // SkyrimCloudShape06
			case 0x272AD: // SkyrimCloudShape06_25
			case 0x272A8: // SkyrimCloudShape06_50
			case 0x272A6: // SkyrimCloudShape06_O
			case 0x27299: // SkyrimCloudShape06_O_25
			case 0x27289: // SkyrimCloudShape06_O_50
			case 0x10C4D1: // INV_SkyrimCloudDistant01
			case 0x10C4D0: // INV_SkyrimCloudDistant01_25
			case 0x10C4CF: // INV_SkyrimCloudDistant01_50
			case 0x10C4CE: // INV_SkyrimCloudDistant01_O
			case 0x10C4CD: // INV_SkyrimCloudDistant01_O_25
			case 0x10C4CC: // INV_SkyrimCloudDistant01_O_50
			case 0x10C4CB: // INV_SkyrimCloudDistant02
			case 0x10C4CA: // INV_SkyrimCloudDistant02_25
			case 0x10C4C9: // INV_SkyrimCloudDistant02_50
			case 0x10C4C8: // INV_SkyrimCloudDistant02_O
			case 0x10C4C7: // INV_SkyrimCloudDistant02_O_25
			case 0x10C4C6: // INV_SkyrimCloudDistant02_O_50
			case 0x10C4C5: // INV_SkyrimCloudDistant03
			case 0x10C4C4: // INV_SkyrimCloudDistant03_25
			case 0x10C4C3: // INV_SkyrimCloudDistant03_50
			case 0x10C4C2: // INV_SkyrimCloudDistant03_O
			case 0x10C4C1: // INV_SkyrimCloudDistant03_O_25
			case 0x10C4C0: // INV_SkyrimCloudDistant03_O_50
			case 0x10C4BF: // INV_SkyrimCloudDistant04
			case 0x10C4BE: // INV_SkyrimCloudDistant04_25
			case 0x10C4BD: // INV_SkyrimCloudDistant04_50
			case 0x10C4BC: // INV_SkyrimCloudDistant04_O
			case 0x10C4BB: // INV_SkyrimCloudDistant04_O_25
			case 0x10C4BA: // INV_SkyrimCloudDistant04_O_50
			case 0x10C4B9: // INV_SkyrimCloudDistant05
			case 0x10C4B8: // INV_SkyrimCloudDistant05_25
			case 0x10C4B7: // INV_SkyrimCloudDistant05_50
			case 0x10C4B6: // INV_SkyrimCloudDistant05_O
			case 0x10C4B5: // INV_SkyrimCloudDistant05_O_25
			case 0x10C4B4: // INV_SkyrimCloudDistant05_O_50
			case 0x10C4B3: // INV_SkyrimCloudDistant06
			case 0x10C4B2: // INV_SkyrimCloudDistant06_25
			case 0x10C4B1: // INV_SkyrimCloudDistant06_50
			case 0x10C4B0: // INV_SkyrimCloudDistant06_O
			case 0x10C4AF: // INV_SkyrimCloudDistant06_O_25
			case 0x10C4AE: // INV_SkyrimCloudDistant06_O_50
			case 0x10C4AD: // INV_SkyrimCloudShape01
			case 0x10C4AC: // INV_SkyrimCloudShape01_25
			case 0x10C4AB: // INV_SkyrimCloudShape01_50
			case 0x10C4AA: // INV_SkyrimCloudShape01_O
			case 0x10C4A9: // INV_SkyrimCloudShape01_O_25
			case 0x10C4A8: // INV_SkyrimCloudShape01_O_50
			case 0x10C4A7: // INV_SkyrimCloudShape02
			case 0x10C4A6: // INV_SkyrimCloudShape02_25
			case 0x10C4A5: // INV_SkyrimCloudShape02_50
			case 0x10C4A4: // INV_SkyrimCloudShape02_O
			case 0x10C4A3: // INV_SkyrimCloudShape02_O_25
			case 0x10C4A2: // INV_SkyrimCloudShape02_O_50
			case 0x10C4A1: // INV_SkyrimCloudShape03
			case 0x10C4A0: // INV_SkyrimCloudShape03_25
			case 0x10C49F: // INV_SkyrimCloudShape03_50
			case 0x10C49E: // INV_SkyrimCloudShape03_O
			case 0x10C49D: // INV_SkyrimCloudShape03_O_25
			case 0x10C49C: // INV_SkyrimCloudShape03_O_50
			case 0x10C49B: // INV_SkyrimCloudShape04
			case 0x10C49A: // INV_SkyrimCloudShape04_25
			case 0x10C499: // INV_SkyrimCloudShape04_50
			case 0x10C498: // INV_SkyrimCloudShape04_O
			case 0x10C497: // INV_SkyrimCloudShape04_O_25
			case 0x10C496: // INV_SkyrimCloudShape04_O_50
			case 0x10C495: // INV_SkyrimCloudShape05
			case 0x10C494: // INV_SkyrimCloudShape05_25
			case 0x10C493: // INV_SkyrimCloudShape05_50
			case 0x10C492: // INV_SkyrimCloudShape05_O
			case 0x10C491: // INV_SkyrimCloudShape05_O_25
			case 0x10C490: // INV_SkyrimCloudShape05_O_50
			case 0x10C48F: // INV_SkyrimCloudShape06
			case 0x10C48E: // INV_SkyrimCloudShape06_25
			case 0x10C48D: // INV_SkyrimCloudShape06_50
			case 0x10C48C: // INV_SkyrimCloudShape06_O
			case 0x10C48B: // INV_SkyrimCloudShape06_O_25
			case 0x10C4D2: // INV_SkyrimCloudShape06_O_50
				a_showWhenFar = true;
				return MCM::settings::GetShowCloudMarkers();
			case 0x3040C: // CW1MeleeCloseAttacker
			case 0x3040E: // CW1MeleeWideAttacker
			case 0x3041E: // CW1MissileCloseAttacker
			case 0x3041F: // CW1MissileWideAttacker
			case 0x30404: // CW1SpawnAttacker
			case 0x3040F: // CW2MeleeCloseAttacker
			case 0x30413: // CW2MeleeWideAttacker
			case 0x30420: // CW2MissileCloseAttacker
			case 0x30421: // CW2MissileWideAttacker
			case 0x30405: // CW2SpawnAttacker
			case 0x30414: // CW3MeleeCloseAttacker
			case 0x30415: // CW3MeleeWideAttacker
			case 0x30422: // CW3MissileCloseAttacker
			case 0x30423: // CW3MissileWideAttacker
			case 0x30406: // CW3SpawnAttacker
			case 0x30416: // CW4MeleeCloseAttacker
			case 0x30417: // CW4MeleeWideAttacker
			case 0x30424: // CW4MissileCloseAttacker
			case 0x30425: // CW4MissileWideAttacker
			case 0x30407: // CW4SpawnAttacker
			case 0x30418: // CW5MeleeCloseAttacker
			case 0x30419: // CW5MeleeWideAttacker
			case 0x30426: // CW5MissileCloseAttacker
			case 0x30427: // CW5MissileWideAttacker
			case 0x30408: // CW5SpawnAttacker
				return MCM::settings::GetShowCWAttackerMarkers();
			case 0x30428: // CW1MeleeCloseDefender
			case 0x30429: // CW1MeleeWideDefender
			case 0x30432: // CW1MissileCloseDefender
			case 0x30433: // CW1MissileWideDefender
			case 0x30409: // CW1SpawnDefender
			case 0x3042A: // CW2MeleeCloseDefender
			case 0x3042B: // CW2MeleeWideDefender
			case 0x30434: // CW2MissileCloseDefender
			case 0x30435: // CW2MissileWideDefender
			case 0x3041A: // CW2SpawnDefender
			case 0x30431: // CW3MeleeCloseDefender
			case 0x3042C: // CW3MeleeWideDefender
			case 0x3043C: // CW3MissileCloseDefender
			case 0x30441: // CW3MissileWideDefender
			case 0x3041B: // CW3SpawnDefender
			case 0x3042D: // CW4MeleeCloseDefender
			case 0x3042E: // CW4MeleeWideDefender
			case 0x3043D: // CW4MissileCloseDefender
			case 0x3043E: // CW4MissileWideDefender
			case 0x3041C: // CW4SpawnDefender		<------------------
			case 0x30430: // CW5MeleeCloseDefender
			case 0x3042F: // CW5MeleeWideDefender
			case 0x3043F: // CW5MissileCloseDefender
			case 0x30440: // CW5MissileWideDefender
			case 0x3041D: // CW5SpawnDefender
				return MCM::settings::GetShowCWDefenderMarkers();
			case 0x41B2D: // CWSiegeSonsSoldier
			case 0x41B2E: // CWSiegeImperialSoldier
			case 0x4B77C: // CWTower01
			case 0xDEEC6: // CWTowerWall01
			case 0xE108E: // CWSoldierImperialUseCatapult (NPC, so doesnt work yet)
				return MCM::settings::GetShowOtherCWMarkers();
			case 0x69811: // ShadowMarkFenceWood01
			case 0x69813: // ShadowMarkLootWood01 // debug this
				return MCM::settings::GetShowTextureSetMarkers();
		}

		switch (baseObject->GetFormType())
		{
			case RE::FormType::IdleMarker:
				return MCM::settings::GetShowIdleMarkers();

		}

		return false;
	}
}