#include "InfoHandler.h"
#include "Utils.h"
#include "DebugMenu.h"

std::map<RE::FormID, std::string> DebugMenu::InfoHandler::soundEditorIDs;

std::string	DebugMenu::InfoHandler::GetInfo()
{
	switch (shapeMetaData.infoType)
	{
		case InfoType::kQuad:
		{
			return GetQuadInfo();
		}
		case InfoType::kNavmesh:
		{
			return GetNavmeshInfo();
		}
		case InfoType::kNavmeshCover:
		{
			return GetNavmeshCoverInfo();
		}
		case InfoType::kOcclusion:
		{
			return GetOcclusionInfo();
		}
		case InfoType::kCollisionMarker:
		{
			return GetCollisionMarkerInfo();
		}
		case InfoType::kRef:
		{
			return GetRefInfo();
		}
		case InfoType::kLightMarker:
		{
			return GetLightMarkerInfo();
		}
		case InfoType::kSoundMarker:
		{
			return GetSoundMarkerInfo();
		}
	}
	return ""s;
}

RE::FormID DebugMenu::InfoHandler::GetFormID()
{
	return shapeMetaData.formID != 0x0 ? shapeMetaData.formID : shapeMetaData.ref ? shapeMetaData.ref->formID : 0;
}

bool DebugMenu::InfoHandler::DoesRefExist(std::string& a_infoStr)
{
	if (!shapeMetaData.ref)
	{
		a_infoStr += "\nERROR: REF NULL!";
		return false;
	}
	return true;
}

std::string	DebugMenu::InfoHandler::GetCellInfo()
{
	std::string infoStr{ "CELL INFO" };

	auto cell = shapeMetaData.cell;
	auto ref = shapeMetaData.ref;

	if (!cell || cell->GetFormID() >> 24 == 0xFF)
	{
		if (ref)
		{
			if (const auto* TES = RE::TES::GetSingleton(); TES)
			{
				cell = TES->GetCell(ref->GetPosition());
			}
		}
		if (cell && cell->GetFormID() >> 24 == 0xFF)
		{
			infoStr += "\nNot available";
		}

	}
	if (cell)
	{
		infoStr += "\nEditor ID: ";
		const char* editorID = cell->GetFormEditorID();
		if (editorID) infoStr += editorID;
		else infoStr += "Not available";

		infoStr += fmt::format("\nForm ID: {:08X}", cell->GetFormID());
		if (cell->IsExteriorCell())
		{
			infoStr += "\nCoordinates: ";
			infoStr += std::to_string(cell->GetRuntimeData().cellData.exterior->cellX);
			infoStr += ", ";
			infoStr += std::to_string(cell->GetRuntimeData().cellData.exterior->cellY);
		}
	}
	return infoStr;
}

std::string DebugMenu::InfoHandler::GetSourceFilesInfo()
{
	auto infoStr = "\nReferenced by:"s;
	for (const auto& fileName : GetSouceFiles(shapeMetaData.ref))
	{
		infoStr += "\nMod: "s;
		infoStr += std::string(fileName);
	}
	return infoStr;
}

std::string DebugMenu::InfoHandler::GetQuadInfo()
{
	std::string infoStr = GetCellInfo();

	auto cell = shapeMetaData.cell;
	auto quad = shapeMetaData.quad;

	auto& cellLand = cell->GetRuntimeData().cellLand;

	infoStr += "\n\nLANDSCAPE INFO:"s;
	infoStr += fmt::format("\nForm ID {:08X}", cellLand->formID);
	infoStr += GetSourceFilesInfo();

	std::string quadLabel = "";
	switch (quad)
	{
		case 0:
			quadLabel = "Bottom Left";
			break;
		case 1:
			quadLabel = "Bottom Right";
			break;
		case 2:
			quadLabel = "Top Left";
			break;
		case 3:
			quadLabel = "Top Right";
			break;
	}

	infoStr += "\n\nQUAD INFO:"s;
	infoStr += fmt::format("\nQuad nummber: {} | {}", quad + 1, quadLabel);

	const auto defaultTexture = cellLand->loadedData->defQuadTextures[quad];

	uint8_t numberOfTextureSets = defaultTexture ? 1 : 0;
	for (const auto texture : cellLand->loadedData->quadTextures[quad])
	{
		if (!texture) break;
		numberOfTextureSets++;
	}

	infoStr += fmt::format("\nTexture sets: {}", numberOfTextureSets);

	int i = 0;

	if (defaultTexture)
	{
		i++;
		infoStr += GetLandTextureInfo(defaultTexture, i);
	}

	for (auto landTexture : cellLand->loadedData->quadTextures[quad])
	{
		i++;
		infoStr += GetLandTextureInfo(landTexture, i);
	}

	return infoStr;
}

std::string DebugMenu::InfoHandler::GetLandTextureInfo(const RE::TESLandTexture* a_landTexture, uint8_t a_textureIndex, bool a_defaultTexture)
{
	if (!a_landTexture) return ""s;

	std::string infoStr = ""s;

	std::string landTextureName = Utils::GetFormEditorID(a_landTexture);
	if (a_landTexture->formID == 0) landTextureName = "LAND_DEFAULT"s;

	if (a_textureIndex > 1) infoStr += "\n"s;
	infoStr += fmt::format("\n ({}) {}{}", a_textureIndex, landTextureName.c_str(), a_defaultTexture ? " (default)"s : ""s);
	auto textureSet = a_landTexture->textureSet;
	if (textureSet)
	{
		for (const auto& texture : textureSet->textures)
		{
			if (!texture.textureName.empty())
			{
				infoStr += "\n  ";
				infoStr += texture.textureName;
			}
		}
	}
	return infoStr;
}

std::string	DebugMenu::InfoHandler::GetNavmeshInfo()
{
	std::string infoStr = GetCellInfo();

	auto cell = shapeMetaData.cell;
	auto formID = GetFormID();

	infoStr += "\n\nNAVMESH INFO";
	infoStr += fmt::format("\nForm ID: {:08X}", formID);

	infoStr += std::string("\nReferenced by (list may be incomplete): ");

	for (const auto& fileName : GetNavmeshHandler()->GetNavmeshSourceFiles(formID))
	{
		infoStr += "\nMod: ";
		infoStr += std::string(fileName);
	}
	return infoStr;
}

std::string DebugMenu::InfoHandler::GetNavmeshCoverInfo()
{
	uint16_t flags = shapeMetaData.navmeshTraversalFlags;
	uint8_t coverEdge = shapeMetaData.coverEdge;
	int32_t height = Utils::GetNavmeshCoverHeight(flags, coverEdge);
	bool left = Utils::GetNavmeshCoverLeft(flags, coverEdge);
	bool right = Utils::GetNavmeshCoverRight(flags, coverEdge);

	std::string infoStr = fmt::format("Cover height: {} units", height);
	if (MCM::settings::showNavmeshCoverLines) infoStr += fmt::format("\nSubsections height: {} units", MCM::settings::linesHeight);
	infoStr += fmt::format("\nLedge: {}", height < 0);
	infoStr += fmt::format("\nRight:  {}", right);
	infoStr += fmt::format("\nLeft:    {}", left);

	return infoStr;
}

std::string DebugMenu::InfoHandler::GetOcclusionInfo()
{
	std::string infoStr = GetCellInfo();

	if (!DoesRefExist(infoStr)) return infoStr;

	RE::FormID formID = GetFormID();
	auto ref = shapeMetaData.ref;
	bool isDisabled = shapeMetaData.ref->IsDisabled();
	auto bounds = shapeMetaData.bounds;

	infoStr += "\n\nPLANEMARKER INFO";
	if (isDisabled) infoStr += "\nPlanemarker currently disabled";
	infoStr += fmt::format("\nForm ID: {:08X}", formID);
	infoStr += fmt::format("\nPosition: {:.0f}, {:.0f}, {:.0f}", ref->GetPositionX(), ref->GetPositionY(), ref->GetPositionZ());
	infoStr += fmt::format("\nBounds: {:.0f}, {:.0f}, {:.0f}", bounds.x, bounds.y, bounds.z);

	infoStr += GetSourceFilesInfo();
	return infoStr;
}

std::string DebugMenu::InfoHandler::GetCollisionMarkerInfo()
{
	std::string infoStr = GetCellInfo();

	if (!DoesRefExist(infoStr)) return infoStr;

	RE::FormID formID = GetFormID();
	auto ref = shapeMetaData.ref;
	bool isDisabled = shapeMetaData.ref->IsDisabled();
	auto bounds = shapeMetaData.bounds;
	auto collisionLayer = shapeMetaData.colliisonLayer;

	infoStr += "\n\nCOLLISIONMARKER INFO";
	if (isDisabled) infoStr += "\nCollisionmarker currently disabled";
	infoStr += fmt::format("\nForm ID: {:08X}", formID);
	infoStr += fmt::format("\nCol layer: {}", GetCollisionLayerName(collisionLayer));
	infoStr += fmt::format("\nPosition: {:.0f}, {:.0f}, {:.0f}", ref->GetPositionX(), ref->GetPositionY(), ref->GetPositionZ());
	infoStr += fmt::format("\nBounds: {:.0f}, {:.0f}, {:.0f}", bounds.x, bounds.y, bounds.z);

	infoStr += GetSourceFilesInfo();
	return infoStr;
}

std::string DebugMenu::InfoHandler::GetRefInfo()
{
	std::string infoStr = GetCellInfo();

	if (!DoesRefExist(infoStr)) return infoStr;

	auto ref = shapeMetaData.ref;

	infoStr += "\n\nREFERENCE INFO:"s;

	if (auto editorID = Utils::GetFormEditorID(ref); !editorID.empty())
	{
		infoStr += fmt::format("\nEditor ID: {}", editorID);
	}
	infoStr += fmt::format("\nFormID: {:X}", ref->formID);

	if (auto base = ref->GetBaseObject())
	{
		infoStr += fmt::format("\nBaseID: {:X}", ref->GetBaseObject()->formID);
		if (auto baseEditorID = Utils::GetFormEditorID(base); !baseEditorID.empty())
		{
			infoStr += fmt::format("\nBase: {}", baseEditorID);
		}
	}

	infoStr += fmt::format("\nEnabled? {}", !ref->IsDisabled());
	if (auto obj = ref->Get3D())
	{
		infoStr += fmt::format("\nCulled? {}", obj->GetAppCulled());
	}

	infoStr += GetSourceFilesInfo();

	return infoStr;
}

std::string DebugMenu::InfoHandler::GetLightMarkerInfo()
{
	std::string infoStr = GetCellInfo();

	if (!DoesRefExist(infoStr)) return infoStr;

	auto cell = shapeMetaData.cell;
	auto ref = shapeMetaData.ref;

	infoStr += "\n\nLIGHT INFO:";

	const RE::TESObjectLIGH* light = ref->GetBaseObject()->As<RE::TESObjectLIGH>();

	auto type = "Omnidirectional"s;

	if (light)
	{
		if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kHemiShadow)) // WhiterunDragonreachBasement
		{
			type = "Shadow Hemisphere"s;
		}
		else if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kSpotShadow)) // PotemasCatacombs02
		{
			type = "Shadow Spotlight"s;
		}
		else if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kOmniShadow))
		{
			type = "Shadow Omnidirectional"s;
		}

		auto& FOV = light->data.fov;
		auto& fade = light->fade;

		auto& falloffExponent = light->data.fallofExponent;
		auto& radius = light->data.radius;
		auto& nearDistance = light->data.nearDistance;
		auto& color = light->data.color;
		auto flickerEffect = "None"s;
		if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kFlicker)) flickerEffect = "Flicker"s;
		else if (light->data.flags.any(RE::TES_LIGHT_FLAGS::kPulse)) flickerEffect = "Pulse"s;

		auto& flickerPeriod = light->data.flickerPeriodRecip;
		auto& flickerIntensityAmplitude = light->data.flickerIntensityAmplitude;
		auto& flicekerMovementAmplitude = light->data.flickerMovementAmplitude;
		auto portalStrict = light->data.flags.any(RE::TES_LIGHT_FLAGS::kPortalStrict);



		infoStr += fmt::format("\nForm ID: {:08X}", GetFormID());
		infoStr += fmt::format("\nPosition: {:.0f}, {:.0f}, {:.0f}", ref->GetPositionX(), ref->GetPositionY(), ref->GetPositionZ());
		infoStr += fmt::format("\nType: {}", type);


		infoStr += fmt::format("\nFOV: {:.2f}", FOV);
		infoStr += fmt::format("\nFade: {:.2f}", fade);
		infoStr += fmt::format("\nFalloff Exponent: {:.2f}", falloffExponent);
		infoStr += fmt::format("\nRadius: {}", radius);
		infoStr += fmt::format("\nNear Clip: {:.2f}", nearDistance);
		infoStr += fmt::format("\nColor: {}, {}, {}", color.red, color.green, color.blue);
		infoStr += fmt::format("\nFlicker Effect: {}", flickerEffect);
		if (flickerEffect != "None"s)
		{
			infoStr += fmt::format("\n Period: {:.2f}", 1 / flickerPeriod);
			infoStr += fmt::format("\n Intensity Amplitude: {:.2f}", flickerIntensityAmplitude);
			infoStr += fmt::format("\n Movement Amplitude: {:.2f}", flicekerMovementAmplitude);
		}
		infoStr += fmt::format("\nPortal Strict: {}", portalStrict);
	}
	else
	{
		infoStr += "\nNo light info available"s;
	}

	infoStr += GetSourceFilesInfo();
	
	return infoStr;
}

std::string DebugMenu::InfoHandler::GetSoundMarkerInfo()
{
	std::string infoStr = GetCellInfo();

	if (!DoesRefExist(infoStr)) return infoStr;

	auto ref = shapeMetaData.ref;

	infoStr += "\n\nSOUND INFO:"s;
	auto sound = ref->GetBaseObject()->As<RE::TESSound>();

	if (sound && sound->descriptor && sound->descriptor->soundDescriptor)
	{
		auto soundDescriptor = sound->descriptor;
		// for more sound info:
		//auto soundDefinition = reinterpret_cast<RE::BGSStandardSoundDef*>(sound->descriptor->soundDescriptor);

		if (!soundEditorIDs.empty())
			infoStr += fmt::format("\nEditor ID: {}", soundEditorIDs[sound->GetFormID()]);
		else
			infoStr += "\nRestart game with 'Mod Active = True' for editorID";




		infoStr += fmt::format("\nBase ID: {:08X}", sound->GetFormID());
		infoStr += fmt::format("\nForm ID: {:08X}", ref->GetFormID());

		infoStr += "\n\nSOUND DESCRIPTOR INFO:"s;

		std::string soundDescriptorEditorID = Utils::GetFormEditorID(soundDescriptor);
		if (soundDescriptorEditorID.empty())
			infoStr += "\nMake sure Powerofthree's Tweaks version >= 1.14.1"s;
		else
			infoStr += fmt::format("\nEditor ID: {}", soundDescriptorEditorID);

		/*if (!soundEditorIDs.empty())
			infoStr += fmt::format("\nEditor ID: {}", soundDescriptorEditorIDs[soundDescriptor->GetFormID()]);
		else
			infoStr += "Restart game with 'Mod Active = True' for editorID";*/

		infoStr += fmt::format("\nForm ID: {:08X}", soundDescriptor->GetFormID());

	}
	else
	{
		infoStr += "\nNo sound info available"s;
	}

	infoStr += GetSourceFilesInfo();

	return infoStr;
}

std::string DebugMenu::InfoHandler::GetCollisionLayerName(RE::COL_LAYER a_layer)
{
	logger::info("layer: {}", static_cast<int>(a_layer));
	switch (a_layer)
	{
		case RE::COL_LAYER::kStatic:
		{
			return "Static"s;
		}
		case RE::COL_LAYER::kAnimStatic:
		{
			return "AnimStatic"s;
		}
		case RE::COL_LAYER::kTransparent:
		{
			return "Transparent"s;
		}
		case RE::COL_LAYER::kClutter:
		{
			return "Clutter"s;
		}
		case RE::COL_LAYER::kWeapon:
		{
			return "Weapon"s;
		}
		case RE::COL_LAYER::kProjectile:
		{
			return "Projectile"s;
		}
		case RE::COL_LAYER::kSpell:
		{
			return "Spell"s;
		}
		case RE::COL_LAYER::kBiped:
		{
			return "Biped"s;
		}
		case RE::COL_LAYER::kTrees:
		{
			return "Trees"s;
		}
		case RE::COL_LAYER::kProps:
		{
			return "Props"s;
		}
		case RE::COL_LAYER::kWater:
		{
			return "Water"s;
		}
		case RE::COL_LAYER::kTrigger:
		{
			return "Trigger"s;
		}
		case RE::COL_LAYER::kTerrain:
		{
			return "Terrain"s;
		}
		case RE::COL_LAYER::kTrap:
		{
			return "Trap"s;
		}
		case RE::COL_LAYER::kNonCollidable:
		{
			return "NonCollidable"s;
		}
		case RE::COL_LAYER::kCloudTrap:
		{
			return "CloudTrap"s;
		}
		case RE::COL_LAYER::kGround:
		{
			return "Ground"s;
		}
		case RE::COL_LAYER::kPortal:
		{
			return "Portal"s;
		}
		case RE::COL_LAYER::kDebrisSmall:
		{
			return "DebrisSmall"s;
		}
		case RE::COL_LAYER::kDebrisLarge:
		{
			return "DebrisLarge"s;
		}
		case RE::COL_LAYER::kAcousticSpace:
		{
			return "AcousticSpace"s;
		}
		case RE::COL_LAYER::kActorZone:
		{
			return "ActorZone"s;
		}
		case RE::COL_LAYER::kProjectileZone:
		{
			return "ProjectileZone"s;
		}
		case RE::COL_LAYER::kGasTrap:
		{
			return "GasTrap"s;
		}
		case RE::COL_LAYER::kShellCasting:
		{
			return "ShellCasting"s;
		}
		case RE::COL_LAYER::kTransparentSmall:
		{
			return "TransparentSmall"s;
		}
		case RE::COL_LAYER::kInvisibleWall:
		{
			return "InvisibleWall"s;
		}
		case RE::COL_LAYER::kTransparentSmallAnim:
		{
			return "TransparentSmallAnim"s;
		}
		case RE::COL_LAYER::kWard:
		{
			return "Ward"s;
		}
		case RE::COL_LAYER::kCharController:
		{
			return "CharController"s;
		}
		case RE::COL_LAYER::kStairHelper:
		{
			return "StairHelper"s;
		}
		case RE::COL_LAYER::kDeadBip:
		{
			return "DeadBip"s;
		}
		case RE::COL_LAYER::kBipedNoCC:
		{
			return "BipedNoCC"s;
		}
		case RE::COL_LAYER::kAvoidBox:
		{
			return "AvoidBox"s;
		}
		case RE::COL_LAYER::kCollisionBox:
		{
			return "CollisionBox"s;
		}
		case RE::COL_LAYER::kCameraSphere:
		{
			return "CameraSphere"s;
		}
		case RE::COL_LAYER::kDoorDetection:
		{
			return "DoorDetection"s;
		}
		case RE::COL_LAYER::kConeProjectile:
		{
			return "ConeProjectile"s;
		}
		case RE::COL_LAYER::kCameraPick:
		{
			return "CameraPick"s;
		}
		case RE::COL_LAYER::kItemPick:
		{
			return "ItemPick"s;
		}
		case RE::COL_LAYER::kLOS:
		{
			return "LOS"s;
		}
		case RE::COL_LAYER::kPathPick:
		{
			return "PathPick"s;
		}
		case RE::COL_LAYER::kCustomPick1:
		{
			return "CustomPick1"s;
		}
		case RE::COL_LAYER::kCustomPick2:
		{
			return "CustomPick2"s;
		}
		case RE::COL_LAYER::kSpellExplosion:
		{
			return "SpellExplosion"s;
		}
		case RE::COL_LAYER::kDroppingPick:
		{
			return "DroppingPick"s;
		}
		//case RE::COL_LAYER::kUnused1:
		//{
		//	return "Unused1"s;
		//}
		//case RE::COL_LAYER::kUnused2:
		//{
		//	return "Unused2"s;
		//}
		case RE::COL_LAYER::kUnused3:
		{
			return "NavCut"s;
		}
		//case RE::COL_LAYER::kUnused4:
		//{
		//	return "Unused4"s;
		//}
		//case RE::COL_LAYER::kUnused5:
		//{
		//	return "Unused5"s;
		//}
		//case RE::COL_LAYER::kUnused6:
		//{
		//	return "Unused6"s;
		//}
		//case RE::COL_LAYER::kUnused7:
		//{
		//	return "Unused7"s;
		//}
		case RE::COL_LAYER::kInvalid:
		{
			return "Invalid"s;
		}
		default:
		{
			return "Unidentified";
		}
	}
}


std::vector<std::string_view> DebugMenu::InfoHandler::GetSouceFiles(const RE::TESForm* a_form)
{
	std::vector<std::string_view> fileNames;

	if (!a_form || !a_form->sourceFiles.array)
	{
		fileNames.push_back("Unable to get files");
		return fileNames;
	}

	RE::TESFile** files = a_form->sourceFiles.array->data();
	int numberOfFiles = a_form->sourceFiles.array->size();

	// yoinked from more informative console sauce https://github.com/Liolel/More-Informative-Console/blob/1613cda4ec067e86f97fb6aae4a7c85533afe031/src/Scaleform/MICScaleform_GetReferenceInfo.cpp#L57
	if ((a_form->GetFormID() >> 24) == 0x00)  //Refs from Skyrim.ESM will have 00 for the first two hexidecimal digits
	{								 //And refs from all other mods will have a non zero value, so a bitwise && of those two digits with FF will be nonzero for all non Skyrim.ESM mods
		if (numberOfFiles == 0 || std::string(files[0]->fileName) != "Skyrim.esm")
		{
			fileNames.push_back("Skyrim.esm");
		}
	}
	for (int i = 0; i < numberOfFiles; i++)
	{
		fileNames.push_back(files[i]->GetFilename());
	}
	return fileNames;
}
