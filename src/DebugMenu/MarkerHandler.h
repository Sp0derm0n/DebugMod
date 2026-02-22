#pragma once

#include "DebugItem.h"

namespace DebugMenu
{
	class MarkerHandler : public DebugItem
	{
		public:
			MarkerHandler();

			std::map<RE::FormID, std::string> soundDescriptorEditorIDs;

			void InitPostDataLoaded();
			void Draw() override;
			void Reset();
			void HideAllMarkers();

		private:
			const RE::FormID debugFormID = 0x10FF45; // reference ID
			uint32_t debugMarkerIndex = -1; // sat by the code when the marker is added

			enum Visibility : bool
			{
				kHide = false,
				kShow = true
			};

			struct Marker
			{
				

				RE::TESObjectREFRPtr ref;
				RE::BSFixedString markerName = ""s;
				RE::FormID formID = 0;
				bool drawWhenFar = false;
				bool cullWhenHiding = true;

				Marker(RE::TESObjectREFR* a_ref, RE::BSFixedString a_markerName = "") :
					ref(a_ref->GetHandle().get()),
					markerName(a_markerName),
					formID(a_ref->formID)
				{
				}
				void SetDefaultState();
			};

			struct MarkerInfo
			{
				std::string path = ""s;
				RE::BSFixedString name = "";
			};

			const float	lightBulbAlphaMax = 2.25;
			const float	drawWhenFarRange = 30000.0f;

			RE::BSEffectShaderMaterial* lightBulbMaterial = nullptr;

			const RE::BSFixedString furnitureMarkerName = "FurnitureMarkerVis";
			const RE::BSFixedString doorTeleportMarkerName = "TeleportMarkerVis";
			const RE::BSFixedString lightMarkerName = "LightMarkerVis";
			const RE::BSFixedString locRefTypeName = "LocRefTypeMarkerVis";
			const RE::BSFixedString skyMarkerBeamName = "SkyMarkerVis";

			std::vector<std::unique_ptr<Marker>> visibleMarkers;
			std::unordered_set<RE::FormID> visibleMarkersIDs;

			float GetRange() override;

			void DrawMarkers();
			void AddVisibleMarker(RE::TESObjectREFR* a_ref, RE::BSFixedString a_markerName = "", bool a_cullWhenHiding = true);
			void RemoveVisibleMarker(uint32_t a_markerIndex);
			void CopyEffectShaderMaterial(const std::string& a_filepath, const std::string& a_shapeName, RE::BSEffectShaderMaterial*& a_materialOut);
			void ShowMarker(RE::TESObjectREFR* a_ref);
			void ShowDoorTeleportMarker(RE::TESObjectREFR* a_ref);
			void ShowFurnitureMarker(RE::TESObjectREFR* a_ref);
			void ShowStaticMarker(RE::TESObjectREFR* a_ref);
			void ShowLightMarker(RE::TESObjectREFR* a_ref);
			void ShowSoundMarker(RE::TESObjectREFR* a_ref);
			//void ShowLocRefType(RE::TESObjectREFR* a_ref);
			//void ShowSkyMarkerBeam(RE::TESObjectREFR* a_ref);
			void HideMarker(uint32_t a_markerIndex);

			[[nodiscard]] bool ShowNodeIfNeeded(RE::TESObjectREFR* a_ref);

			bool IsMarkerLoaded(const std::unique_ptr<Marker>& a_marker);
			bool ShouldMarkerBeDrawnWhenFar(RE::TESObjectREFR* a_ref);
			bool ShouldMarkerBeDrawn(RE::TESObjectREFR* a_ref);
			bool ShouldMarkerBeDrawn(RE::TESObjectREFR* a_ref, bool& a_showWhenFar);

			RE::NiNode* GetNodeFromRef(RE::TESObjectREFR* a_ref);
			RE::NiNode* TrySet3DByName(RE::TESObjectREFR* a_ref, const char* a_modelName, const RE::BSFixedString a_markerName);
			MarkerInfo	GetMarkerInfo(RE::TESObjectREFR* a_ref);
	};
}