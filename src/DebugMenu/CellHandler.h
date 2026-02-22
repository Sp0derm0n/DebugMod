#pragma once

#include "DebugItem.h"

namespace DebugMenu
{
	class CellHandler : public DebugItem
	{
		public:
			CellHandler();
			void Draw() override;
	};
}