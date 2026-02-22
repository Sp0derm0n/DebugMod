# pragma once

#include "DebugItem.h"

namespace DebugMenu
{
	class RefInspectorHandler : public DebugItem
	{
		public:
			RefInspectorHandler();

			void Draw() override;

		private:
			float	GetRange() override;
	};
}