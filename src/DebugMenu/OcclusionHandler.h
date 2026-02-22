#pragma once

#include "DebugItem.h"

namespace DebugMenu
{
	class OcclusionHandler : public DebugItem
	{
		public:
			OcclusionHandler();

			void Draw() override;

		private:
			
			float	GetRange() override;
			void	DrawOcclusion();
	};
}