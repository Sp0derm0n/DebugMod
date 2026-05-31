#include "IElement.h"

constexpr const ScaleformUI::IElement::Direction ScaleformUI::IElement::Direction::left{ 0 };
constexpr const ScaleformUI::IElement::Direction ScaleformUI::IElement::Direction::right{ 1 };
constexpr const ScaleformUI::IElement::Direction ScaleformUI::IElement::Direction::top{ 2 };
constexpr const ScaleformUI::IElement::Direction ScaleformUI::IElement::Direction::bottom{ 3 };

ScaleformUI::IElement::Direction::operator ScaleformUI::Alignment() const
{
	switch (value)
	{
		case left:
			return Alignment::kLeft;
		case right:
			return Alignment::kRight;
		case top:
			return Alignment::kTop;
		case bottom:
			return Alignment::kBottom;
		default:
			return Alignment::kNone;
	}
}