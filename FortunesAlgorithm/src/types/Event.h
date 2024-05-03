#pragma once
#include "VoronoiDiagram.h"

namespace BL
{
	class Arc;
}

enum EventPointType
{
	Site = 0,
	Circle = 1
};

class EventPoint {
public:
	EventPoint(VoronoiSite* site) : Type(EventPointType::Site), Site(site), Arc(nullptr), Radius(0.0), Deleted(false)
	{

	}

	EventPoint(VoronoiSite* site, EventPointType type) : Type(type), Site(site), Arc(nullptr), Radius(0.0), Deleted(false)
	{

	}

	EventPointType Type;
	VoronoiSite* Site;
	BL::Arc* Arc;
	long double Radius;

	//CircleSite
	bool Deleted;

	// We are handling events left to right if equal prioritizing circle events
	bool isGreater(const EventPoint& point)
	{
		return Site->point.y > point.Site->point.y 
			|| (Site->point.y == point.Site->point.y && Type == EventPointType::Circle && point.Type == EventPointType::Site)
			|| (Site->point.y == point.Site->point.y && Site->point.x <= point.Site->point.x);
	}
};
