#pragma once
#include <iostream>
struct Point {
public:
	Point(double _x, double _y) : x(_x), y(_y) {}
	double x;
	double y;
};

bool operator==(const Point& lhs, const Point& rhs);

bool operator!= (const Point& lhs, const Point& rhs);

std::ostream& operator<<(std::ostream& lhs, const Point& rhs);
