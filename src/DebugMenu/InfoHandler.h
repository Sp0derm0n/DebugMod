#pragma once

#include "DrawHandler.h"

using InfoType = DrawHandler::ShapeMetaData::InfoType;

namespace DebugMenu
{
	//std::string Get

	class InfoHandler
	{
		public:
			static std::map<RE::FormID, std::string> soundEditorIDs;

			DrawHandler::MetaDataPtr shapeMetaData;

			InfoHandler(DrawHandler::MetaDataPtr& a_shapeMetaData) : shapeMetaData(a_shapeMetaData) {}

			std::string GetInfo();

		private:
			RE::FormID  GetFormID();
			bool		DoesRefExist(std::string& a_infoStr);

			std::string GetQuadInfo();
			std::string GetNavmeshInfo();
			std::string GetNavmeshCoverInfo();
			std::string GetOcclusionInfo();
			std::string GetRefInfo();
			std::string GetLightMarkerInfo();
			std::string GetSoundMarkerInfo();

			// Helper functions
			std::string	GetCellInfo();
			std::string GetLandTextureInfo(const RE::TESLandTexture* a_landTexture, uint8_t a_textureIndex, bool a_defaultTexture = false);
			
			std::vector<std::string_view> GetSouceFiles(const RE::TESForm* a_form);

	};

	


}