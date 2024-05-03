#include "Point.h";

bool operator==(const Point& lhs, const Point& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!= (const Point& lhs, const Point& rhs)
{
	return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const Point& rhs)
{
	return lhs << "(" << rhs.x << ", " << rhs.y << ")";
}
