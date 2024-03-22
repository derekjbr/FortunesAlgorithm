#include "Conversion.h"
#include "../types/Point.h"

static PlaneBounds* Instance = 0;


///////////////////////////////////////////////////////////
PlaneBounds::PlaneBounds()
{
	Left = Right = Upper = Lower = 0.0f;
}
///////////////////////////////////////////////////////////
PlaneBounds* PlaneBounds::GetInstance()
{
	if (nullptr == Instance)
		Instance = new PlaneBounds();
		
	return Instance;
}

///////////////////////////////////////////////////////////
void PlaneBounds::SetBounds(float left, float right, float lower, float upper)
{
	Left = left;
	Right = right;
	Lower = lower;
	Upper = upper;
}

///////////////////////////////////////////////////////////
void PlaneBounds::ConvertPointToScreenRange(const Point& point, float& x, float& y)
{
	x = (point.x - Left) * (2.0f) / (Right - Left) - 1.f;
	y = (point.y - Lower) * (2.0f) / (Upper - Lower) - 1.f;
}

///////////////////////////////////////////////////////////
void SetPlaneBounds(const float leftBound, const float rightBound, const float lowerBound, const float upperBound)
{
	PlaneBounds* bounds = PlaneBounds::GetInstance();
	bounds->SetBounds(leftBound, rightBound, lowerBound, upperBound);
}

///////////////////////////////////////////////////////////
void PointToScreen(const Point& point, float& x, float& y)
{
	PlaneBounds* bounds = PlaneBounds::GetInstance();
	bounds->ConvertPointToScreenRange(point, x, y);
}