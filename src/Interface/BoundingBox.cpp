#include "BoundingBox.h";

bool ScaleformUI::SquareBBox::IsPointInBBox(float a_x, float a_y)
{ 
	// only works when rotation is 0
	
	return (a_x > xmin) && (a_x < xmax) && (a_y > ymin) && (a_y < ymax); 
}

void ScaleformUI::SquareBBox::UpdateBounds(float a_xmin, float a_ymin, float a_xmax, float a_ymax, float a_rotation)
{
	// xmin etc. are in global coords
	// needs to rotate to set AABB

	xmin = a_xmin;
	ymin = a_ymin;
	xmax = a_xmax;
	ymax = a_ymax;
	rotation = a_rotation;

	aabb.xmin = xmin;
	aabb.ymin = ymin;
	aabb.xmax = xmax;
	aabb.ymax = ymax;
}