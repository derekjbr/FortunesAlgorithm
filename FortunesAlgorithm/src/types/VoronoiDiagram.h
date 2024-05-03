#pragma once

#include "DCELTypes.h"
#include "Point.h"

#include <vector>
#include <iostream>

struct VoronoiSite
{
	Point point;
	DCEL::Face* face;
	DCEL::Vertex* triVertex;
	int index;
};

std::ostream& operator<<(std::ostream& os, const VoronoiSite& site);

class VoronoiDiagram
{
public:
	VoronoiDiagram(std::vector<Point>& points);
	VoronoiDiagram(std::string fileLocation);

	std::vector<Point> Points;

	std::vector<VoronoiSite*> Sites;
	std::vector<DCEL::Face*> Faces;
	std::vector<DCEL::Vertex*> Vertices;
	std::vector<DCEL::HalfEdge*> HalfEdges;

	std::vector<DCEL::Face*> TriangulationFaces;
	std::vector<DCEL::Vertex*> TriangulationVertices;
	std::vector<DCEL::HalfEdge*> TriangulationHalfEdges;

	void PrintToFile(std::string fileLocation);
	void PrintVoronoiDCEL(std::ostream& os);
	void PrintDelaunayTriangulation(std::ostream& os);

	double MinX, MinY, MaxX, MaxY;

private:
	void UpdateBounds(const Point& point);


	friend class FortunesAlgorithm;
};