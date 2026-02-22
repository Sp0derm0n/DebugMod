#include "CollisionHandler.h"
#include "math.h"

//#define COLLISIONS_PROFILING
//#define LOG_COLLISION


namespace DebugMenu
{
	CollisionHandler::CollisionHandler()
	{
		logger::debug("Initialized CollisionHandler");
	}

	std::pair<CollisionHandler::CollisionTriangle, CollisionHandler::CollisionTriangle> CollisionHandler::RefCollisionData::SquareToTriangles(
		vec3u& a_point1, vec3u& a_point2, vec3u& a_point3, vec3u& a_point4, vec4u& a_color)
	{
		// returns 2 triangles in same direction points are given (clockwise or counter clockwise)
		CollisionTriangle triangle1{ a_point1, a_point2, a_point4, a_color };
		CollisionTriangle triangle2{ a_point2, a_point3, a_point4, a_color };
		return std::pair<CollisionTriangle, CollisionTriangle>(triangle1, triangle2);
	}

	void CollisionHandler::RefCollisionData::AddCollisionLine(vec3u& a_start, vec3u& a_end)
	{
		collisionLines.push_back(CollisionLine(a_start, a_end, MCM::settings::collisionColor));
	}

	void CollisionHandler::RefCollisionData::AddCollisionLine(vec3u& a_start, vec3u& a_end, glm::vec4& a_color)
	{
		collisionLines.push_back(CollisionLine(a_start, a_end, a_color));
	}

	void CollisionHandler::RefCollisionData::AddCollisionMesh(std::vector<CollisionTriangle>& a_triangles)
	{
		if (a_triangles.size() > 0) collisionMeshes.push_back(CollisionMesh(a_triangles));
	}

	#ifdef COLLISIONS_PROFILING
		struct CollisionProfileData
		{
			struct SubDelta
			{
				std::string name;
				std::vector<long long> deltas;
			};
			std::string	name;
			std::vector<long long> deltas;
			std::vector<SubDelta> subDeltas;
			uint32_t numCollisions;

			float Avg(std::vector<long long> a_list)
			{
				if (a_list.empty()) return -1.0;

				long long sum = 0;
				for (auto item : a_list)
				{
					sum += item;
				}
				return static_cast<float>(sum) / a_list.size();
			}

			// 1% top decentile
			long long NearMax(std::vector<long long> a_list)
			{
				if (a_list.empty()) return 0;
				auto copy = a_list;
				std::sort(copy.begin(), copy.end());
				uint32_t length = copy.size() - 1;
				uint32_t index = static_cast<uint32_t>(length*0.99);
				if (index > length) index = length;
				return copy[index];
			}

			// 1% low decentile
			long long NearMin(std::vector<long long> a_list)
			{
				if (a_list.empty()) return 0;
				auto copy = a_list;
				std::sort(copy.begin(), copy.end());
				uint32_t length = copy.size() - 1;
				uint32_t index = static_cast<uint32_t>(length * 0.01);
				if (index > length) index = length;
				return copy[index];
			}

			float AvgDelta()
			{
				return Avg(deltas);
			}

			bool operator==(CollisionProfileData& other)
			{
				return name == other.name;
			}
		};

		static CollisionProfileData allCollisionsTotalData{			"All colisions" };
		static CollisionProfileData boxCollisionsData{				"Boxes" };
		static CollisionProfileData capsuleCollisionsData{			"Capsules" };
		static CollisionProfileData compressedMeshCollisionsData{	"Compress meshes" };
		static CollisionProfileData convexVerticesCollisionsData{	"Convex vertices" };
		static CollisionProfileData collisionMeshData{				"collision mesh" };

		static std::vector<CollisionProfileData*> allProfilingData{ &allCollisionsTotalData,
																	&boxCollisionsData, 
																	&capsuleCollisionsData,
																	&compressedMeshCollisionsData,
																	&convexVerticesCollisionsData,
																	&collisionMeshData };
		

		static void AddCount(CollisionProfileData& a_data)
		{
			a_data.numCollisions++;
			if (a_data != collisionMeshData)
				allCollisionsTotalData.numCollisions++;
		}

		static void AddDelta(CollisionProfileData& a_data, long long a_delta)
		{
			a_data.deltas.push_back(a_delta);

			if (a_data != allCollisionsTotalData) AddCount(a_data);	
		}

		static void AddSubDelta(CollisionProfileData& a_data, long long a_delta, std::string a_name)
		{
			for (int i = 0; i < a_data.subDeltas.size(); i++)
			{
				if (a_data.subDeltas[i].name == a_name)
				{
					a_data.subDeltas[i].deltas.push_back(a_delta);
					return;
				}
			}
			a_data.subDeltas.push_back(CollisionProfileData::SubDelta(a_name, std::vector<long long>{ a_delta }));
		}

		static void ResetCounts()
		{
			for (const auto& data : allProfilingData)
			{
				data->numCollisions = 0;
			}
		}

		static void PrintProfiling()
		{
			logger::debug("{:<30}     {:>12} [{:>5}:{:>5}] {:>10}", "Collision Profiling :", "Avg (µs)", "1% lo", "1% hi", "Last frame");
			for (const auto& data : allProfilingData)
			{
				logger::debug("{:<30}     {:>12.2f} [{:>5}:{:>5}] {:>10} | {} shapes", data->name, data->AvgDelta(), data->NearMin(data->deltas), data->NearMax(data->deltas), data->deltas.back(), data->numCollisions);
				for (const auto& subDelta : data->subDeltas)
				{
					//                  30-3
					logger::debug("    {:<27}     {:>12.2f} [{:>5}:{:>5}]", subDelta.name, data->Avg(subDelta.deltas), data->NearMin(subDelta.deltas), data->NearMax(subDelta.deltas));
					
				}
			}

			ResetCounts();
		}
	#endif

	CollisionHandler::CollisionMesh::CollisionMesh(std::vector<CollisionTriangle>& a_triangles)
	{
		#ifdef COLLISIONS_PROFILING
			auto start = std::chrono::system_clock::now();
		#endif

		std::vector<Renderer::Model::Vertex> vertices;

		for (const auto& triangle : a_triangles)
		{
			vec2u uv{ 0.0f, 0.0f };
			vec3u normal = { 0.0f, 0.0f, 0.0f }; //glm::cross(p2 - p1, p3 - p1); // Not needed for simple wireframes
			vec4u color = MCM::settings::collisionColor;

			Renderer::Model::Vertex v1{ triangle.point1, uv, normal, color };
			Renderer::Model::Vertex v2{ triangle.point2, uv, normal, color };
			Renderer::Model::Vertex v3{ triangle.point3, uv, normal, color };

			vertices.push_back(v1);
			vertices.push_back(v2);
			vertices.push_back(v3);
		}

		#ifdef COLLISIONS_PROFILING
			auto convertingTriangles = std::chrono::system_clock::now();
			auto delta_convertingTriangles = std::chrono::duration_cast<std::chrono::microseconds>(convertingTriangles - start).count();
			AddSubDelta(collisionMeshData, delta_convertingTriangles, "Converting triangles");
		#endif
		
		Renderer::Model::MeshHeader meshHeader{ "ello", vertices.size() };
		Renderer::Model::Mesh mesh{ meshHeader, vertices };
		Renderer::Model::ModelHeader modelHeader{ 0, 0, 1 };

		std::vector<Renderer::Model::Mesh> meshes{ mesh };

		Renderer::Model::Model mdl{ modelHeader, meshes };

		auto& ctx = Renderer::GetContext();
		auto perObjectBuffer = Renderer::GetPerObjectCBuffer();

		#ifdef COLLISIONS_PROFILING
			auto setupModel = std::chrono::system_clock::now();
			auto delta_setupModel = std::chrono::duration_cast<std::chrono::microseconds>(setupModel - convertingTriangles).count();
			AddSubDelta(collisionMeshData, delta_setupModel, "Setup model");
		#endif

		Renderer::MeshCreateInfo meshInfo;
		meshInfo.mesh = &mdl.meshes[0];
		meshInfo.vs = Renderer::GetMeshVS();
		meshInfo.ps = Renderer::GetMeshPS();

		#ifdef COLLISIONS_PROFILING
			auto creatingMeshInfo = std::chrono::system_clock::now();
			auto delta_creatingMeshInfo = std::chrono::duration_cast<std::chrono::microseconds>(creatingMeshInfo - setupModel).count();
			AddSubDelta(collisionMeshData, delta_creatingMeshInfo, "Creating mesh info");
		#endif

		meshDrawer = std::make_shared<Renderer::MeshDrawer>(meshInfo, perObjectBuffer, ctx);

		#ifdef COLLISIONS_PROFILING
			auto end = std::chrono::system_clock::now();
			auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
			AddDelta(collisionMeshData, delta);

			auto delta_makingMeshDrawer = std::chrono::duration_cast<std::chrono::microseconds>(end - creatingMeshInfo).count();
			AddSubDelta(collisionMeshData, delta_makingMeshDrawer, "Creating mesh drawer");
		#endif
	}

	float CollisionHandler::GetRange()
	{
		return MCM::settings::collisionRange;
	}

	void CollisionHandler::Reset()
	{
		HideAllCollisions();
		selectedRefs.clear();
		previousConsoleSelectedRef = nullptr;
	}

	void CollisionHandler::Draw()
	{
		if (MCM::settings::showCollision && MCM::settings::useD3D) DrawCollisions();
		#ifdef COLLISIONS_PROFILING
			PrintProfiling();
		#endif
	}

	void CollisionHandler::DrawCollisions()
	{
		#ifdef COLLISIONS_PROFILING
			auto start = std::chrono::system_clock::now();
		#endif

		auto consoleRef = RE::Console::GetSelectedRef().get();

		UpdateSelectedRefs(consoleRef);

		UpdateVisibleCollisions(consoleRef);

		switch (MCM::settings::collisionDisplayIndex)
		{
			case 0: // Range
			{
				Utils::ForEachCellInRange(RE::PlayerCharacter::GetSingleton()->GetPosition(), GetRange(), [&](const RE::TESObjectCELL* a_cell)
				{
					a_cell->ForEachReference([&](RE::TESObjectREFR* a_ref)
					{
						if (!a_ref) return RE::BSContainer::ForEachResult::kContinue;

						float dx = GetCenter().x - a_ref->GetPositionX();
						float dy = GetCenter().y - a_ref->GetPositionY();
						if (dx * dx + dy * dy < GetRange() * GetRange())
						{
							DrawCollision(a_ref);
						}
						return RE::BSContainer::ForEachResult::kContinue;
					});
				});
				break;
			}
			case 1: // currently selected
			{
				if (consoleRef)
				{
					DrawCollision(consoleRef);
				}
				break;
			}
			case 2: // multiple selected
			{
				for (auto& ref : selectedRefs)
				{
					DrawCollision(ref.get());
				}
				break;
			}			

		}

		#ifdef COLLISIONS_PROFILING
			auto end = std::chrono::system_clock::now();
			auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
			AddDelta(allCollisionsTotalData, delta);
		#endif

	}

	void CollisionHandler::UpdateSelectedRefs(RE::TESObjectREFR* a_ref)
	{
		// remove unloaded refs
		for (size_t i = selectedRefs.size(); i--;)
		{
			auto& refPtr = selectedRefs[i];
			if (!refPtr || !Utils::IsRefInLoadedCell(refPtr.get()))
			{
				selectedRefs.erase(selectedRefs.begin()+i);
			}
		}

		// Remove any additional refs if maxCollisions has been changed
		if (selectedRefs.size() > MCM::settings::maxCollisions)
		{
			for (int i = 0; i < selectedRefs.size() - MCM::settings::maxCollisions; i++)
			{
				selectedRefs.erase(selectedRefs.begin());
			}
		}

		// this triggers when going into first person if the playerRef check isn't made, which
		// will remove the player from the selected refs when going into first person
		if (!a_ref || !a_ref->Get3D() || !a_ref->Get3D()->GetCollisionObject() && !a_ref->IsPlayerRef())
		{
			previousConsoleSelectedRef = nullptr;
			return;
		}

		// skip if the selected ref hasn't changed
		if (previousConsoleSelectedRef && previousConsoleSelectedRef.get() == a_ref)
		{
			return;
		}
		previousConsoleSelectedRef = a_ref->GetHandle().get();

		// ONLY WHEN NOT IN CONSOLE SELECTED MODE, SINCE THE CURRENTLY SELECTED ELEMENT SHOULD BE
		// VISIBLE WHEN SWITCHING TO MULTIPLE SELECTED MODE
		// remove ref from list if has been selected again
		
			for (int i = 0; i < selectedRefs.size(); i++)
			{
				if (selectedRefs[i].get() == a_ref)
				{
					if (MCM::settings::collisionDisplayIndex != 1)
					{
						selectedRefs.erase(selectedRefs.begin() + i);
					}
					return;
				}
			}

		// add ref to list (and remove first element if list is full)
		if (selectedRefs.size() == MCM::settings::maxCollisions)
		{
			selectedRefs.erase(selectedRefs.begin());
		}
		selectedRefs.push_back(a_ref->GetHandle().get());

	}

	void CollisionHandler::ResetSelectedRefs()
	{
		selectedRefs.clear();
		previousConsoleSelectedRef = nullptr;
	}

	void CollisionHandler::HideAllCollisions()
	{
		visibleCollisions.clear();
	}

	// Unlike markers, the collisions are purely visuals. The refs are only needed to extract the 
	// world position of the collisions. Therefore, removing a drawn collision is as simple as
	// not drawing it anymore.
	void CollisionHandler::UpdateVisibleCollisions(RE::TESObjectREFR* a_consoleRef)
	{
		for (size_t i = visibleCollisions.size(); i--;)
		{
			auto& visibleCollision = visibleCollisions[i];
			bool hideCollision = true;

			auto worldSpace = RE::PlayerCharacter::GetSingleton()->GetWorldspace();
			RE::FormID worldSpaceID = worldSpace ? worldSpace->formID : 0x0;
			if (visibleCollision->worldSpaceID == worldSpaceID &&
				visibleCollision->ref && 
				visibleCollision->ref->Get3D() &&
				Utils::IsRefInLoadedCell(visibleCollision->ref.get()))
			{
				auto node = visibleCollision->ref->Get3D()->AsNode();
				if (node)
				{
					// if in selected ref mode, if ref != selected ref->remove (Happens automatically, since we don't continue
					// if index == 1, so the single collision that may be visible will be removed at the bottom of the loop
					// and drawn anew afterwards

					if (MCM::settings::collisionDisplayIndex == 0) // range mode
					{
						if (visibleCollision->GetSquareDistance(GetCenter()) < GetRange() * GetRange())
						{
							visibleCollision->UpdateCollision();
							hideCollision = false;
						}
					}
					else if (MCM::settings::collisionDisplayIndex == 1) // Console Selected
					{
						if (a_consoleRef && a_consoleRef->formID == visibleCollision->refFormID)
						{
							visibleCollision->UpdateCollision();
							hideCollision = false;
						}
					}
					else if (MCM::settings::collisionDisplayIndex == 2) // Multiple selected
					{
						for (auto& ref : selectedRefs)
						{
							if (ref->formID == visibleCollision->refFormID)
							{
								visibleCollision->UpdateCollision();
								hideCollision = false;
								break;
							}
						}
					}
				}
			}
			if (hideCollision) visibleCollisions.erase(visibleCollisions.begin() + i);
		}
	}

	void CollisionHandler::DrawCollision(RE::TESObjectREFR* a_ref)
	{
		if (a_ref->IsDisabled() || !Utils::IsRefInLoadedCell(a_ref)) return;

		bool isCreature = a_ref->formType == RE::FormType::ActorCharacter;

		for (const auto& visibleCollision : visibleCollisions)
		{
			if(visibleCollision->refFormID == a_ref->formID) 
			{
				return;
			}
		}

		auto node = a_ref->Get3D() ? a_ref->Get3D()->AsNode() : nullptr;
		if (!node || node->GetAppCulled()) return;

		#ifdef LOG_COLLISION
			logger::debug("REF: {:X}; scale: {}", a_ref->formID, a_ref->GetScale());
		#endif

		auto data = std::make_unique<RefCollisionData>(a_ref);
		data->isCreature = isCreature;
		data->GetCollisionCoordinates();
		data->DrawObject();


		const auto bsxFlags = node->GetExtraData<RE::BSXFlags>("BSX");

		// dynamic flag (signs on hinges etc.)
		if (bsxFlags && ((bsxFlags->value & 1 << 6) != 0)) data->isStatic = false;

		// animated flag (salmon fx etc.)
		if (bsxFlags && ((bsxFlags->value & 1) != 0)) data ->isStatic = false;

		visibleCollisions.push_back(std::move(data));

	}

	CollisionHandler::RefCollisionData::RefCollisionData(RE::TESObjectREFR* a_ref)
	{
		ref = a_ref->GetHandle().get();
		auto worldSpace = RE::PlayerCharacter::GetSingleton()->GetWorldspace();
		if (worldSpace) worldSpaceID = worldSpace->formID;
		
		refFormID = a_ref->formID;
		auto base = a_ref->GetBaseObject();
		//masterNode = a_ref->Get3D()->AsNode();
		previousPosition = a_ref->GetPosition();//masterNode->world.translate;
	}

	void CollisionHandler::RefCollisionData::UpdateCollision()
	{
		if (isCreature || !isStatic || previousPosition != ref->GetPosition())
		{
			collisionLines.clear();
			collisionMeshes.clear();
			GetCollisionCoordinates();
			previousPosition = ref->GetPosition();
		}
		DrawObject();
	}

	float CollisionHandler::RefCollisionData::GetSquareDistance(const RE::NiPoint3& a_point)
	{
		auto masterNode = ref->Get3D() ? ref->Get3D()->AsNode() : nullptr;
		if (!masterNode) return 123456789.10f;
		float dx = a_point.x - masterNode->world.translate.x;
		float dy = a_point.y - masterNode->world.translate.y;
		return dx*dx + dy*dy;
	}

	void CollisionHandler::RefCollisionData::GetCollisionCoordinates()
	{
		const auto cell = ref->GetSaveParentCell();
		
		auto bhkWorld = cell ? cell->GetbhkWorld() : nullptr;

		if (isCreature)
		{
			if (const auto& actor = ref->As<RE::Actor>())
			{
				if (const auto charController = actor->GetCharController())
				{
					bhkWorld = charController->GetWorldImpl();
				}
			}
		}
		
		if (bhkWorld)
		{
			RE::BSReadLockGuard locker(bhkWorld->worldLock);
			
			RE::BSVisit::TraverseScenegraphCollision(ref->Get3D(), [&](RE::bhkNiCollisionObject* a_niCollision)
			{
				auto result = RE::BSVisit::BSVisitControl::kContinue;

				if (!a_niCollision) return result;

				auto collisionObject = reinterpret_cast<RE::bhkCollisionObject*>(a_niCollision);
				if (!collisionObject) return result;
				
				CollisionObject object{ a_niCollision->sceneObject };
				object.collisionScale = bhkWorld->GetWorldScaleInverse();

				if (isCreature && MCM::settings::showCharController) 
				{
					HandleActors(object);
					if (object.hkpShape) 
						result = RE::BSVisit::BSVisitControl::kStop;
				}

				if (!object.hkpShape)
				{
					auto rigidBody = collisionObject->GetRigidBody();
					if (!rigidBody) return result;

					if (const auto* hkpShape = rigidBody->GetRigidBody()->GetShape())
					{
						object.hkpShape = hkpShape;

						if (auto rigidBodyT = skyrim_cast<RE::bhkRigidBodyT*>(rigidBody))
						{
							object.useRBTransform = true;
							object.rbRotation = rigidBodyT->rotation;
							object.rbOffset = Utils::hkvec4toNiVec3(rigidBodyT->translation);

							#ifdef LOG_COLLISION
								auto pos = object.rbOffset * object.collisionScale;
								logger::debug("RigidBodyT:");
								logger::debug("  -Offset: {} {} {}", object.rbOffset.x, object.rbOffset.y, object.rbOffset.z);;
								logger::debug("  -Scaled Offset: {} {} {}", pos.x, pos.y, pos.z);
								Utils::printVec4("  -Rotation:", object.rbRotation.vec);
							#endif

						}
					}
				}

				if (object.hkpShape)
				{
					#ifdef LOG_COLLISION
						auto& pos = object.parent->world.translate;
						logger::debug("Node:");
						logger::debug("  -World translation: {} {} {}", pos.x, pos.y, pos.z);
						Linalg::PrintMatrix("  -World rotation:", object.parent->world.rotate, 4);
					#endif

					GetObjectCollisionCoordinates(object);
				}
			
				return result;
			});
		}
	}

	void CollisionHandler::RefCollisionData::HandleActors(CollisionObject& a_object)
	{
		const auto& actor = ref->As<RE::Actor>();
		if (!actor) return;

		auto charController = actor->GetCharController();
		if (!charController) return;

		if (auto proxyController = skyrim_cast<RE::bhkCharProxyController*>(charController))
		{
			auto charProxy = static_cast<RE::hkpCharacterProxy*>(proxyController->proxy.referencedObject.get());
			if (!charProxy) return;

			a_object.hkpShape = charProxy->shapePhantom->collidable.shape;

		}
		else if (auto rigidBodyController = skyrim_cast<RE::bhkCharRigidBodyController*>(charController))
		{
			auto charRigidBody = static_cast<RE::hkpCharacterRigidBody*>(rigidBodyController->charRigidBody.referencedObject.get());
			if (!charRigidBody) return;

			a_object.hkpShape = charRigidBody->character->collidable.shape;

		}

		if (a_object.hkpShape)
		{
			RE::hkVector4 pos{ 0.0f, 0.0f, 0.0f, 0.0f };
			charController->GetPosition(pos, false);

			a_object.isCharController = true;
			a_object.charControllerOffset = Utils::hkvec4toNiVec3(pos);
			a_object.charControllerRotation = Utils::GetRotationMatrixZ(-actor->data.angle.z);
			hasCharControllerCollision = true;
		}
	}

	void CollisionHandler::RefCollisionData::GetObjectCollisionCoordinates(CollisionObject& a_object)
	{
		#ifdef LOG_COLLISION
			logger::debug("Collision Type: {}", Utils::GethkpShapeTypeName(a_object.hkpShape));
		#endif
		switch (a_object.hkpShape->type)
		{
			case RE::hkpShapeType::kBox:
			{
				#ifdef COLLISIONS_PROFILING
					auto start = std::chrono::system_clock::now();
				#endif

				GetBoxCollisionCoordinates(a_object);

				#ifdef COLLISIONS_PROFILING
					auto end = std::chrono::system_clock::now();
					auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
					AddDelta(boxCollisionsData, delta);
				#endif

				break;
			}
			case RE::hkpShapeType::kCapsule:
			{
				#ifdef COLLISIONS_PROFILING
					auto start = std::chrono::system_clock::now();
				#endif

				GetCapsuleCollisionCoordnates(a_object);

				#ifdef COLLISIONS_PROFILING
					auto end = std::chrono::system_clock::now();
					auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
					AddDelta(capsuleCollisionsData, delta);
				#endif

				break;
			}
			case RE::hkpShapeType::kCompressedMesh:
			{
				#ifdef COLLISIONS_PROFILING
					auto start = std::chrono::system_clock::now();
				#endif

				GetCompresshedMeshCollisionCoordinates(a_object);

				#ifdef COLLISIONS_PROFILING
					auto end = std::chrono::system_clock::now();
					auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
					AddDelta(compressedMeshCollisionsData, delta);
				#endif

				break;
			}
			case RE::hkpShapeType::kConvexTransform:
			{
				GetConvexTransformCollisionCoordinates(a_object);
				break;
			}
			case RE::hkpShapeType::kConvexVertices:
			{
				#ifdef COLLISIONS_PROFILING
					auto start = std::chrono::system_clock::now();
				#endif

				GetConvexVerticesCollisionCoordinates(a_object);

				#ifdef COLLISIONS_PROFILING
					auto end = std::chrono::system_clock::now();
					auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
					AddDelta(convexVerticesCollisionsData, delta);
				#endif

				break;
			}
			case RE::hkpShapeType::kList:
			{
				GetListCollisionCoordinates(a_object);
				break;
			}
			case RE::hkpShapeType::kMOPP:
			{
				GetMOPPCollisionCoordinates(a_object);
				break;
			}
		}
	}

	void CollisionHandler::RefCollisionData::GetBoxCollisionCoordinates(CollisionObject& a_object)
	{
		const auto* boxShape = static_cast<const RE::hkpBoxShape*>(a_object.hkpShape);
		if (!boxShape) return;
		auto sides = Utils::hkvec4toNiVec3(boxShape->halfExtents);

		auto cornerToWorldPos = [&](RE::NiPoint3 a_corner)
		{
			return Utils::NiToGLMVec3(a_object.GetWorldPos(a_corner));
		};

		//
		//			ULB ------- URB
		//		   / |		   / |
		//		  /	 |		  /	 |	
		//		 /	 |		 /	 |
		//		/	 |		/	 |		 z
		//     /    LLB ---/--- LRB		 |  y
		//    /		/	  /     /        | /
		//   /	   /	 /     /         |/
		// ULF ------- URF	  /			 O-------x
		//	|	 /		|    /			
		//	|	/		|   /		   
		//	|  /		|  /		  
		//	| /			| /
		// LLF ------- LRF
		//

		vec3u upperRightBack  = cornerToWorldPos(RE::NiPoint3(  sides.x,  sides.y,  sides.z ));
		vec3u lowerRightBack  = cornerToWorldPos(RE::NiPoint3(  sides.x,  sides.y, -sides.z ));
		vec3u lowerLeftBack   = cornerToWorldPos(RE::NiPoint3( -sides.x,  sides.y, -sides.z ));
		vec3u upperLeftBack   = cornerToWorldPos(RE::NiPoint3( -sides.x,  sides.y,  sides.z ));
		vec3u upperRightFront = cornerToWorldPos(RE::NiPoint3(  sides.x, -sides.y,  sides.z ));
		vec3u lowerRightFront = cornerToWorldPos(RE::NiPoint3(  sides.x, -sides.y, -sides.z ));
		vec3u lowerLeftFront  = cornerToWorldPos(RE::NiPoint3( -sides.x, -sides.y, -sides.z ));
		vec3u upperLeftFront  = cornerToWorldPos(RE::NiPoint3( -sides.x, -sides.y,  sides.z ));

		
		if (MCM::settings::cleanCollisions)
		{
			AddCollisionLine(upperRightBack, lowerRightBack);
			AddCollisionLine(lowerRightBack, lowerLeftBack);
			AddCollisionLine(lowerLeftBack, upperLeftBack);
			AddCollisionLine(upperLeftBack, upperRightBack);

			// Front square
			AddCollisionLine(upperRightFront, lowerRightFront);
			AddCollisionLine(lowerRightFront, lowerLeftFront);
			AddCollisionLine(lowerLeftFront, upperLeftFront);
			AddCollisionLine(upperLeftFront, upperRightFront);

			//Middle part
			AddCollisionLine(upperRightBack, upperRightFront);
			AddCollisionLine(lowerRightBack, lowerRightFront);
			AddCollisionLine(lowerLeftBack, lowerLeftFront);
			AddCollisionLine(upperLeftBack, upperLeftFront);
		}
		else
		{
			auto back = SquareToTriangles(upperLeftBack, lowerLeftBack, lowerRightBack, upperRightBack);
			auto right = SquareToTriangles(upperRightFront, upperRightBack, lowerRightBack, lowerLeftBack);
			auto bottom = SquareToTriangles(lowerLeftBack, lowerLeftFront, lowerRightFront, lowerRightBack);
			auto left = SquareToTriangles(upperLeftFront, lowerLeftFront, lowerLeftBack, upperLeftBack);
			auto top = SquareToTriangles(upperLeftFront, upperLeftBack, upperRightBack, upperRightFront);
			auto front = SquareToTriangles(upperLeftFront, upperRightFront, lowerRightFront, lowerLeftFront);

			std::vector<CollisionTriangle> triangles;
			triangles.push_back(back.first);
			triangles.push_back(back.second);
			triangles.push_back(right.first);
			triangles.push_back(right.second);
			triangles.push_back(bottom.first);
			triangles.push_back(bottom.second);
			triangles.push_back(left.first);
			triangles.push_back(left.second);
			triangles.push_back(top.first);
			triangles.push_back(top.second);
			triangles.push_back(front.first);
			triangles.push_back(front.second);

			AddCollisionMesh(triangles);
		}
	}

	std::vector<vec3u> CollisionHandler::RefCollisionData::GetCircle
	(uint32_t a_segments, float a_radius, vec3u& a_unitXVector, vec3u& a_unitYVector, vec3u& a_center)
	{
		float PI = 3.14159265358f;
		float thetaStep = 2 * PI / a_segments;

		auto radiusX = a_unitXVector * a_radius;
		auto radiusY = a_unitYVector * a_radius;


		// Doing mirroing is ~5-10% faster than continously rotating the points the full 360 degrees
		std::vector<vec3u> circle{ a_center + radiusX};
		// points in first quad exluding the radius x and y vectors
		for (int i = 1; i < a_segments / 4; i++)
		{
			float cos = (cosf(thetaStep * i) * a_radius);
			float sin = (sinf(thetaStep * i) * a_radius);
			circle.push_back((a_unitXVector * cos + a_unitYVector * sin) + a_center);
		}

		// y vector
		circle.push_back(a_center + radiusY);

		// starting with the first point before the y vector, mirror all points around y axis (exluding the x vector)
		for (int i = a_segments / 4 - 1; i > 0; i--)
		{
			circle.push_back(circle[i] - a_unitXVector * (2 * a_radius * cosf(thetaStep * i)));
		}
		circle.push_back(a_center - radiusX);

		// make bottom half to by mirroring top half x in reverse order (excluding the points on the x axis)
		for (int i = a_segments / 2 - 1 ; i > 0; i--)
		{
			if (i == a_segments - a_segments / 4)
			{
				circle.push_back(a_center - radiusY);
				continue;
			}
			circle.push_back(circle[i] - a_unitYVector * (2 * a_radius * sinf(thetaStep * i)));
		}

		return circle;
	}

	void CollisionHandler::RefCollisionData::GetCapsuleCollisionCoordnates(CollisionObject& a_object)
	{
		const auto* capsuleShape = static_cast<const RE::hkpCapsuleShape*>(a_object.hkpShape);
		if (!capsuleShape) return;

		uint32_t segments = MCM::settings::capsuleCylinderSegments;
		float PI = 3.14159265358f;
		float thetaStep = 2 * PI / segments;

		float r = capsuleShape->radius * a_object.collisionScale;

		vec3u topPt = Utils::NiToGLMVec3(a_object.GetWorldPos(Utils::hkvec4toNiVec3(capsuleShape->vertexA)));
		vec3u bottomPt = Utils::NiToGLMVec3(a_object.GetWorldPos(Utils::hkvec4toNiVec3(capsuleShape->vertexB)));

		auto vertical = topPt - bottomPt;
		auto unitVertical = vertical / glm::length(vertical);

		uint8_t biggestComponentIndex = 0;
		float biggestComponent = vertical.x;
		if (fabsf(vertical.y) > fabsf(biggestComponent)) 
		{
			biggestComponent = vertical.y;
			biggestComponentIndex = 1;
		}
		if (fabsf(vertical.z) > fabsf(biggestComponent)) 
		{
			biggestComponent = vertical.z;
			biggestComponentIndex = 2;
		}
		
		float negativeSum = -vertical.x - vertical.y - vertical.z + biggestComponent;
		vec3u radiusVector{ 1.0f, 1.0f, 1.0f};
		radiusVector[biggestComponentIndex] = negativeSum / biggestComponent;
		radiusVector /= glm::length(radiusVector);

		auto radiusPerpendicular = glm::cross(unitVertical, radiusVector);

		std::vector<vec3u> bottomCircle = GetCircle(segments, r, radiusVector, radiusPerpendicular, bottomPt);
		std::vector<vec3u> topCircle = GetCircle(segments, r, radiusVector, radiusPerpendicular, topPt);

		// Draw the cylinder
		for (int i = 0; i < segments; i++)
		{
			int j = i != segments - 1 ? i + 1 : 0;
			auto bottom1 = bottomCircle[i];
			auto bottom2 = bottomCircle[j];
			auto top1 = topCircle[i];
			auto top2 = topCircle[j];

			AddCollisionLine(bottom1,bottom2);
			AddCollisionLine(top1, top2);
			AddCollisionLine(bottom1,top1);
		}

		// Draw the hemispheres at the ends of cylinder
		uint32_t sphereSegments = MCM::settings::capsuleSphereSegments;
		thetaStep = PI/2 / sphereSegments;
		std::vector<std::vector<vec3u>> smallerCircles;
		std::vector<std::vector<vec3u>> topSphere{ topCircle };
		std::vector<std::vector<vec3u>> bottomSphere{ bottomCircle };

		for (int i = 1; i < sphereSegments; i++)
		{
			float reducedRadius = cosf(thetaStep * i) * r;
			vec3u verticalOffset = unitVertical * (sinf(thetaStep * i) * r);
			
			std::vector<vec3u> topReducedCircle;
			std::vector<vec3u> bottomReducedCircle;

			float k = reducedRadius/r;
			float km = 1 - k;

			vec3u reducedTopPt = (topPt * km) + verticalOffset;
			vec3u reducedBottomPt = (bottomPt * km) - verticalOffset;

			for (const auto& point : topCircle)
			{
				topReducedCircle.push_back((point * k) + reducedTopPt);
			}

			for (const auto& point : bottomCircle)
			{
				bottomReducedCircle.push_back((point * k) + reducedBottomPt);
			}

			topSphere.push_back(topReducedCircle);
			bottomSphere.push_back(bottomReducedCircle);
		}

		auto topSphereApex = topPt + unitVertical*r;
		auto bottomSphereApex = bottomPt - unitVertical*r;

		for (int i = 0; i < segments; i++)
		{
			for (int j = 0; j < sphereSegments; j++)
			{
				auto top1 = topSphere[j][i];
				auto top2 = j < sphereSegments-1 ? topSphere[j+1][i] : topSphereApex;

				auto bottom1 = bottomSphere[j][i];
				auto bottom2 = j < sphereSegments-1 ? bottomSphere[j+1][i] : bottomSphereApex;
			
				AddCollisionLine(top1, top2);
				AddCollisionLine(bottom1, bottom2);
			}
		}
	}

	void CollisionHandler::RefCollisionData::GetCompresshedMeshCollisionCoordinates(CollisionObject& a_object)
	{
		const auto* hkpCompressedMeshShape = static_cast<const RE::hkpCompressedMeshShape*>(a_object.hkpShape);
		if (!hkpCompressedMeshShape) return;
	
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		// The scale of collision meshes mostly baked into the vertices, but not for compressed mesh shapes //
		// However, the precalculated bounds of the mesh are scaled correctly, so we can scale the vertices //
		//   to all fit withing these bounds																//
		// To optimize getting the world points of the mesh, we must first run through all local vertices   //
		//   to determine the local scale																	//
		//////////////////////////////////////////////////////////////////////////////////////////////////////

		bool hasUnscaledBoundsChanged = false;

		RE::NiPoint3 unscaledMinBounds{ 0.0f, 0.0f, 0.0f };
		RE::NiPoint3 unscaledMaxBounds{ 0.0f, 0.0f, 0.0f };

		float minBoundsMargin = hkpCompressedMeshShape->bounds.min.quad.m128_f32[3];
		float maxBoundsMargin = hkpCompressedMeshShape->bounds.max.quad.m128_f32[3];
		RE::NiPoint3 scaledMinBounds = Utils::hkvec4toNiVec3(hkpCompressedMeshShape->bounds.min);
		RE::NiPoint3 scaledMaxBounds = Utils::hkvec4toNiVec3(hkpCompressedMeshShape->bounds.max);

		scaledMinBounds.x -= minBoundsMargin;
		scaledMinBounds.y -= minBoundsMargin;
		scaledMinBounds.z -= minBoundsMargin;

		scaledMaxBounds.x -= maxBoundsMargin;
		scaledMaxBounds.y -= maxBoundsMargin;
		scaledMaxBounds.z -= maxBoundsMargin;
		

		auto UpdateBounds = [&](RE::NiPoint3& a_point)
		{
			if (!hasUnscaledBoundsChanged)
			{
				unscaledMinBounds = a_point;
				unscaledMaxBounds = a_point;
				hasUnscaledBoundsChanged = true;
				return;
			}

			if (a_point.x < unscaledMinBounds.x) unscaledMinBounds.x = a_point.x;
			else if (a_point.x > unscaledMaxBounds.x) unscaledMaxBounds.x = a_point.x;

			if (a_point.y < unscaledMinBounds.y) unscaledMinBounds.y = a_point.y;
			else if (a_point.y > unscaledMaxBounds.y) unscaledMaxBounds.y = a_point.y;

			if (a_point.z < unscaledMinBounds.z) unscaledMinBounds.z = a_point.z;
			else if (a_point.z > unscaledMaxBounds.z) unscaledMaxBounds.z = a_point.z;
		};

		float error = hkpCompressedMeshShape->error;
		for (const auto& chunk : hkpCompressedMeshShape->chunks)
		{
			RE::NiPoint3 chunkOffset = Utils::hkvec4toNiVec3(chunk.offset);


			for (int i = 0; i < chunk.vertices.size() / 3; i++)
			{
				RE::NiPoint3 localPos{ chunk.vertices[3 * i] * error,
									   chunk.vertices[3 * i + 1] * error,
									   chunk.vertices[3 * i + 2] * error };

				localPos += chunkOffset;
				UpdateBounds(localPos);
			}
		}

		for (auto& triangle : hkpCompressedMeshShape->bigTriangles)
		{
			auto localPt1 = Utils::hkvec4toNiVec3(hkpCompressedMeshShape->bigVertices[triangle.a]);
			auto localPt2 = Utils::hkvec4toNiVec3(hkpCompressedMeshShape->bigVertices[triangle.b]);
			auto localPt3 = Utils::hkvec4toNiVec3(hkpCompressedMeshShape->bigVertices[triangle.c]);

			UpdateBounds(localPt1);
			UpdateBounds(localPt2);
			UpdateBounds(localPt3);
		}

		auto unscaledBounds = unscaledMaxBounds - unscaledMinBounds;
		auto scaledBounds = scaledMaxBounds - scaledMinBounds;
		auto xRatio = unscaledBounds.x != 0 ? scaledBounds.x / unscaledBounds.x : 0.0f;
		auto yRatio = unscaledBounds.y != 0 ? scaledBounds.y / unscaledBounds.y : 0.0f;
		auto zRatio = unscaledBounds.z != 0 ? scaledBounds.z / unscaledBounds.z : 0.0f;

		uint8_t nonZeroRatios = 0;
		if (xRatio != 0) nonZeroRatios += 1;
		if (yRatio != 0) nonZeroRatios += 1;
		if (zRatio != 0) nonZeroRatios += 1;

		float localScale = nonZeroRatios != 0 ? (xRatio + yRatio + zRatio) / nonZeroRatios : 1.0f;

		#ifdef LOG_COLLISION
			logger::debug("Compressed mesh:");
			logger::debug("  -scale: {:.3f}; ratios: ({:.3f} / {:.3f} / {:.3f})", localScale, xRatio, yRatio, zRatio);
			logger::debug("  -Chunks: {}", hkpCompressedMeshShape->chunks.size());
			logger::debug("  -BigTris: {}", hkpCompressedMeshShape->bigTriangles.size());
		#endif

		std::vector<CollisionTriangle> triangles;
		for (const auto& chunk : hkpCompressedMeshShape->chunks)
		{

			
			// calculate collision mesh vertices https://github.com/niftools/nifskope/blob/3a85ac55e65cc60abc3434cc4aaca2a5cc712eef/src/gl/gltools.cpp#L968
			RE::NiPoint3 chunkOffset = Utils::hkvec4toNiVec3(chunk.offset);
			
			// Transforms seem to all be identity
			// auto& transform = hkpCompressedMeshShape->transforms[chunk.transformIndex];	 
			 
			 // could use Chunk::decompressVertex instead of finding all vertices manually
			 #ifdef LOG_COLLISION
				auto pos = chunkOffset * a_object.collisionScale;
				logger::debug("  -CHUNK:");
				logger::debug("     >Offset: {} {} {}", chunkOffset.x, chunkOffset.y, chunkOffset.z);
				logger::debug("     >Scaled Offset: {} {} {}", pos.x, pos.y, pos.z);
			 #endif
			
				

			std::vector<vec3u> vertices;
			for (int i = 0; i < chunk.vertices.size() / 3; i++)
			{
				RE::NiPoint3 localPos{ chunk.vertices[3 * i] * error,
									   chunk.vertices[3 * i + 1] * error,
									   chunk.vertices[3 * i + 2] * error };
				
				localPos += chunkOffset;
				localPos *= localScale;
				// localPos *= Utils::hkvec4toNiVec3(transform.scale);
				// localPos = Utils::RotateNiPoint3(localPos, transform.rotation);
				// localPos += Utils::hkvec4toNiVec3(transform.translation);
				vertices.push_back(Utils::NiToGLMVec3(a_object.GetWorldPos(localPos)));
			}

			int indexOffset = 0;
			for (int s = 0; s < (int)chunk.stripLengths.size(); s++)
			{
				for (int i = 0; i < chunk.stripLengths[s] - 2; i++)
				{
					auto pt1 = vertices[chunk.indices[indexOffset + i]];
					auto pt2 = vertices[chunk.indices[indexOffset + i + 1]];
					auto pt3 = vertices[chunk.indices[indexOffset + i + 2]];

					triangles.push_back(CollisionTriangle{ pt1, pt2, pt3 });
				}
				indexOffset += chunk.stripLengths[s];
			}				
		
			for (int i = indexOffset; i < chunk.indices.size() - 2; i += 3)
			{

				auto pt1 = vertices[chunk.indices[i]];
				auto pt2 = vertices[chunk.indices[i + 1]];
				auto pt3 = vertices[chunk.indices[i + 2]];

				triangles.push_back(CollisionTriangle{ pt1, pt2, pt3 });
			}

			//std::string indicesStr = "Indices = [ "s;
			//for (auto i : chunk.indices)
			//{
			//	indicesStr += fmt::format("{}, ", i);
			//}
			//indicesStr += "999][:-1]";
			//logger::debug("{}", indicesStr);
			//
			//std::string verticesStr = "Vertices = [ "s;
			//
			//for (auto v : chunk.vertices)
			//{
			//	verticesStr += fmt::format("{}, ", v);
			//}
			//verticesStr += "999][:-1]";
			//logger::debug("{}", verticesStr);
			//
			//std::string stripsStr = "Strips = [";
			//for (auto s : chunk.stripLengths)
			//{
			//	stripsStr += fmt::format("{}, ", s);
			//}
			//stripsStr += "999][:-1]";
			//logger::debug("{}", stripsStr);

		}

		for (auto& triangle : hkpCompressedMeshShape->bigTriangles)
		{
			auto localPt1 = Utils::hkvec4toNiVec3(hkpCompressedMeshShape->bigVertices[triangle.a]);
			auto localPt2 = Utils::hkvec4toNiVec3(hkpCompressedMeshShape->bigVertices[triangle.b]);
			auto localPt3 = Utils::hkvec4toNiVec3(hkpCompressedMeshShape->bigVertices[triangle.c]);

			localPt1 *= localScale;
			localPt2 *= localScale;
			localPt3 *= localScale;

			auto pt1 = Utils::NiToGLMVec3(a_object.GetWorldPos(localPt1));
			auto pt2 = Utils::NiToGLMVec3(a_object.GetWorldPos(localPt2));
			auto pt3 = Utils::NiToGLMVec3(a_object.GetWorldPos(localPt3));

			triangles.push_back(CollisionTriangle{ pt1, pt2, pt3 });
		}

		AddCollisionMesh(triangles);
	}

	void CollisionHandler::RefCollisionData::GetConvexTransformCollisionCoordinates(CollisionObject& a_object)
	{
		const auto* convexTransform = static_cast<const RE::hkpConvexTransformShape*>(a_object.hkpShape);
		if (!convexTransform) return;

		a_object.useLocalTransform = true;
		a_object.localOffset += Utils::hkvec4toNiVec3(convexTransform->transform.translation);
		a_object.localRotation = Utils::hkMat3toNiMat3(convexTransform->transform.rotation) * a_object.localRotation;
		
		LoopOverSingleShapeContainer(a_object, convexTransform->childShape);
	}

	std::vector<RE::FormID> CollisionHandler::RefCollisionData::refsWithBadConvexHulls;

	void CollisionHandler::RefCollisionData::GetConvexVerticesCollisionCoordinates(CollisionObject& a_object)
	{
		if (badConvexHull) return;

		const auto* convexVerticesShape = static_cast<const RE::hkpConvexVerticesShape*>(a_object.hkpShape);
		if (!convexVerticesShape) return;

		if (!convexVerticesShape->connectivity)
		{
			RE::hkpConvexVerticesConnectivityUtil::ensureConnectivity(convexVerticesShape);
		}

		if (convexVerticesShape->connectivity)
		{
			auto vertices = Utils::GetEmptyHkArray<RE::hkVector4>();
			RE::GetOriginalVertices(convexVerticesShape, vertices);
			
			RE::hkArray<uint16_t>& vertexIndices = convexVerticesShape->connectivity->vertexIndices;
			RE::hkArray<uint8_t>& verticesPerFaceArray = convexVerticesShape->connectivity->numVerticesPerFace;

			int expectedVertexIndices = 0;
			for (const auto verticesPerFace : verticesPerFaceArray)
			{
				expectedVertexIndices += verticesPerFace;
			}

			// occasionally, the convex hull cannot be computed by ensureConnectivity 
			// E.g. when different planes are extremely close to each other (like 1e-7 close)
			// In this case, we use an ordinary convex hull algorithm to build the hull
			// If this also fails, we issue a notice
			if (vertexIndices.size() == 0 || !vertexIndices._data || expectedVertexIndices != vertexIndices.size())
			{
				Utils::ConnectivityData newConvexHull = Utils::FindConvexHull(vertices, convexVerticesShape->planeEquations);
				
				if (newConvexHull.vertexIndices.size() < 3)
				{
					badConvexHull = true;

					bool hasErrorBeenPrinted = false;
					for (auto id : refsWithBadConvexHulls)
					{
						if (id == refFormID)
						{
							hasErrorBeenPrinted = true;
							break;
						}
					}
					if (!hasErrorBeenPrinted)
					{
						logger::debug("Failed to get convex hull connectivity data; Failed building new convex hull.");
						logger::debug("References with formID: {:X} has convex hull collision that cannot be displayed.", refFormID);
						refsWithBadConvexHulls.push_back(refFormID);
					}
					return;
				}

				convexVerticesShape->connectivity->vertexIndices = newConvexHull.vertexIndices;
				convexVerticesShape->connectivity->numVerticesPerFace = newConvexHull.verticesPerFace;
			}

			int faceStartIndex = 0;

			if (MCM::settings::cleanCollisions)
			{
				for (int i = 0; i < verticesPerFaceArray.size(); i++)
				{
					auto verticesPerFace = verticesPerFaceArray[i];
					if (verticesPerFace >= 3)
					{
						for (int k = faceStartIndex; k < faceStartIndex + verticesPerFace; k++)
						{
							auto index1 = vertexIndices[k];
							auto index2 = k < faceStartIndex + verticesPerFace - 1 ? vertexIndices[k + 1] : vertexIndices[faceStartIndex];

							auto vertex1 = Utils::NiToGLMVec3(a_object.GetWorldPos(Utils::hkvec4toNiVec3(vertices[index1])));
							auto vertex2 = Utils::NiToGLMVec3(a_object.GetWorldPos(Utils::hkvec4toNiVec3(vertices[index2])));

							AddCollisionLine(vertex1, vertex2);
						}
					}
					faceStartIndex += verticesPerFace;
				}
			}
			else
			{
				std::vector<CollisionTriangle> triangles;

				std::vector<std::vector<uint16_t>> faces;
				for (int i = 0; i < verticesPerFaceArray.size(); i++)
				{
					auto verticesPerFace = verticesPerFaceArray[i];
					if (verticesPerFace >= 3)
					{
						std::vector<uint16_t> faceIndices;

						for (int k = faceStartIndex; k < faceStartIndex + verticesPerFace; k++)
						{
							faceIndices.push_back(vertexIndices[k]);
						}
						faceStartIndex += verticesPerFace;
						faces.push_back(faceIndices);
					}
				}

				auto triangleIndices = Utils::ConvexHullPlanesIndicesToTriangleIndices(faces);

				for (const auto& triangle : triangleIndices)
				{
					auto pt1 = Utils::NiToGLMVec3(a_object.GetWorldPos(Utils::hkvec4toNiVec3(vertices[triangle.index1])));
					auto pt2 = Utils::NiToGLMVec3(a_object.GetWorldPos(Utils::hkvec4toNiVec3(vertices[triangle.index2])));
					auto pt3 = Utils::NiToGLMVec3(a_object.GetWorldPos(Utils::hkvec4toNiVec3(vertices[triangle.index3])));

					triangles.push_back(CollisionTriangle{ pt1, pt2, pt3 });
				}
				AddCollisionMesh(triangles);
			}
		}
	}

	void CollisionHandler::RefCollisionData::GetListCollisionCoordinates(CollisionObject& a_object)
	{
		const auto* listShape = static_cast<const RE::hkpListShape*>(a_object.hkpShape);
		if (!listShape) return;
		
		if (listShape->GetNumChildShapes() == 0) return;

		RE::hkpShapeBuffer buffer;
		int32_t childIndex = 0;
		for (auto key = listShape->GetFirstKey(); childIndex < listShape->GetNumChildShapes(); key = listShape->GetNextKey(key))
		{
			childIndex++;
			auto childShape = listShape->GetChildShape(key, buffer);
			CollisionObject childObject = a_object;
			childObject.hkpShape = childShape;

			GetObjectCollisionCoordinates(childObject);

		}
	}


	void CollisionHandler::RefCollisionData::GetMOPPCollisionCoordinates(CollisionObject& a_object)
	{
		const auto* treeShape = static_cast<const RE::hkpMoppBvTreeShape*>(a_object.hkpShape);
		if (!treeShape) return;

		a_object.testOffset.z = 400/70.0f;
		#ifdef LOG_COLLISION
			auto pos = Utils::hkvec4toNiVec3(treeShape->code->info.offset);
			auto scaledPos = a_object.collisionScale * pos;
			logger::debug("MOPP:");
			logger::debug("  -Address: {:X}", reinterpret_cast<uintptr_t>(treeShape));
			logger::debug("  -Userdata Address: {:X}", reinterpret_cast<uintptr_t>(treeShape->userData));
			logger::debug("  -Offset: {} {} {}", pos.x, pos.y, pos.z);
			logger::debug("  -scaled offset: {} {} {}", scaledPos.x, scaledPos.y, scaledPos.z);
		#endif

		LoopOverSingleShapeContainer(a_object, treeShape->child);

	}

	void CollisionHandler::RefCollisionData::LoopOverSingleShapeContainer(CollisionObject& a_object, const RE::hkpSingleShapeContainer& a_singleShapeContainer)
	{
		if (a_singleShapeContainer.GetNumChildShapes() == 0) return;

		RE::hkpShapeBuffer buffer;
		int childIndex = 0;
		for (auto key = a_singleShapeContainer.GetFirstKey(); childIndex < a_singleShapeContainer.GetNumChildShapes(); key = a_singleShapeContainer.GetNextKey(key))
		{
			childIndex++;
			auto childShape = a_singleShapeContainer.GetChildShape(key, buffer);
			CollisionObject childObject = a_object;
			childObject.hkpShape = childShape;

			GetObjectCollisionCoordinates(childObject);
		}
	}


	void CollisionHandler::RefCollisionData::DrawObject()
	{
		for (auto& line : collisionLines)
		{
			Renderer::DrawLine(line.start, line.end, line.color);
		}

		for (auto& mesh : collisionMeshes)
		{
			Renderer::DrawMesh(mesh.meshDrawer);
		}
	}

	//  Collision Profiling : Avg(µs)[1 % lo:1 % hi] Last frame
	//	All colisions                            897.62[770:1202]        938 | 96 shapes
	//	Boxes                                      6.10[5:12]          6 | 20 shapes
	//	Capsules                                   4.24[4:8]          4 | 20 shapes
	//	Compress meshes                           37.87[14:110]         15 | 3 shapes
	//	Convex vertices                           29.97[16:58]         28 | 15 shapes
	//	collision mesh                             7.32[5:27]          5 | 38 shapes
	//	Converting triangles                    0.84[0:9]
	//	Setup model                             0.31[0:6]
	//	Creating mesh info                      0.00[0:0]
	//	Creating mesh drawer                    5.16[4:21]

	RE::NiPoint3 CollisionHandler::CollisionObject::GetWorldPos(RE::NiPoint3 a_position)
	{
		auto& worldPos = parent->world.translate;
		auto& worldRot = parent->world.rotate;

		if (useLocalTransform) a_position = localOffset + localRotation * a_position;
		if (useRBTransform) a_position = rbOffset + Utils::RotateNiPoint3(a_position, rbRotation);
		
		a_position *= collisionScale;
		
		if (isCharController) return (charControllerOffset * collisionScale + charControllerRotation*a_position);

		return worldPos + worldRot * a_position;

	}


}