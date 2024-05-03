#include "DCELTypes.h"

std::ostream& operator<<(std::ostream& os, DCEL::HalfEdge& rhs)
{
	char eOrD = (rhs.incidentFace != nullptr && rhs.incidentFace->index != 0) ? 'd' : 'e';
	if (rhs.origin)
	{
		if (rhs.origin->box)
			os << "b";
		else
			os << eOrD;
		os << rhs.origin->index;
	}
	else
	{
		os << eOrD;
		os << "UK";
	}
	os << ",";
	if (rhs.dest)
	{
		if (rhs.dest->box)
			os << "b";
		else if (rhs.origin && rhs.origin->box)
			os << eOrD;
		os << rhs.dest->index;
	}
	else
	{
		os << eOrD;
		os << "UK";
	}
	return os;
}