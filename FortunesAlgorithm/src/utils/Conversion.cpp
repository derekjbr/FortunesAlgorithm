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
void PlaneBounds::SetBounds(double left, double right, double lower, double upper)
{
	Left = left;
	Right = right;
	Lower = lower;
	Upper = upper;
}

///////////////////////////////////////////////////////////
void PlaneBounds::ConvertPointToScreenRange(const Point& point, double& x, double& y)
{
	x = (point.x - Left) * (2.0f) / (Right - Left) - 1.f;
	y = (point.y - Lower) * (2.0f) / (Upper - Lower) - 1.f;
}

///////////////////////////////////////////////////////////
void SetPlaneBounds(const double leftBound, const double rightBound, const double lowerBound, const double upperBound)
{
	PlaneBounds* bounds = PlaneBounds::GetInstance();
	bounds->SetBounds(leftBound, rightBound, lowerBound, upperBound);
}

///////////////////////////////////////////////////////////
void PointToScreen(const Point& point, double& x, double& y)
{
	PlaneBounds* bounds = PlaneBounds::GetInstance();
	bounds->ConvertPointToScreenRange(point, x, y);
}