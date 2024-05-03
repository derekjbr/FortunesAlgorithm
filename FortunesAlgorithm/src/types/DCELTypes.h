#pragma once
#include "Point.h"
#include <iostream>
struct VoronoiSite;

namespace DCEL {
	struct Vertex;
	struct Face;
	struct HalfEdge;

	struct Vertex
	{
		int index;
		Point point;
		HalfEdge* incidentEdge;
		bool box = false;
	};

	struct Face
	{
		VoronoiSite* site;
		HalfEdge* outerComponent;
		HalfEdge* innerComponent;
		bool Unbounded = false;
		int index;
	};

	struct HalfEdge {
		Vertex* origin;
		Vertex* dest;
		HalfEdge* twin;
		Face* incidentFace;
		HalfEdge* next;
		HalfEdge* prev;
	};

}

std::ostream& operator<<(std::ostream& os, DCEL::HalfEdge& rhs);