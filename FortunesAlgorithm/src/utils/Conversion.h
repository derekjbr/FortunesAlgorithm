#pragma once

struct Point;

///////////////////////////////////////////////////////////
static class PlaneBounds
{
public:
	///////////////////////////////////////////////////////////
	static PlaneBounds* GetInstance();
	void SetBounds(float left, float right, float lower, float upper);
	void ConvertPointToScreenRange(const Point& point, float& x, float& y);

private:
	PlaneBounds();
	float Left, Right, Upper, Lower;
};

void SetPlaneBounds(const float leftBound, const float rightBound, const float lowerBound, const float upperBound);
void PointToScreen(const Point& point, float& x, float& y);
