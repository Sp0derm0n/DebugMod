#pragma once

#include "DrawHandler.h"
#include "MCM.h"
#include "Utils.h"

using InfoType = DrawHandler::ShapeMetaData::InfoType;

namespace DebugMenu
{
	class DebugItem
	{
		public:
			struct InfoRequestData
			{
				InfoRequestData() {};
				RE::FormID formID = 0x0;
				RE::NiPoint3 pointLocation{ 0.0f, 0.0f, 0.0f };
			};

			

			virtual void Draw() = 0;
			static InfoRequestData infoRequestData;

			DebugItem() {}

		protected:
			inline DrawHandler::MetaDataPtr CreateMetaData(){ return std::make_shared<DrawHandler::ShapeMetaData>(); }

			virtual RE::NiPoint3 GetCenter() { return RE::PlayerCharacter::GetSingleton()->GetPosition(); }
			virtual float		 GetRange() { return 0; }
	};		
	
}