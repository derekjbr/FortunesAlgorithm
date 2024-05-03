#pragma once

struct Point;

///////////////////////////////////////////////////////////
static class PlaneBounds
{
public:
	///////////////////////////////////////////////////////////
	static PlaneBounds* GetInstance();
	void SetBounds(double left, double right, double lower, double upper);
	void ConvertPointToScreenRange(const Point& point, double& x, double& y);

	double Left, Right, Upper, Lower;
private:
	PlaneBounds();
};

void SetPlaneBounds(const double leftBound, const double rightBound, const double lowerBound, const double upperBound);
void PointToScreen(const Point& point, double& x, double& y);
