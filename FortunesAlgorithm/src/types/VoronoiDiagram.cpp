#pragma once

#include "VoronoiDiagram.h"

#include "DCELTypes.h"
#include "Point.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

std::ostream& operator<<(std::ostream& os, const VoronoiSite& site)
{
	os << "P" << site.index << ": (" << site.point.x << ", " << site.point.y << ")";
	return os;
}


VoronoiDiagram::VoronoiDiagram(std::vector<Point>& points)
{
	MinX = MinY = _DMAX;
	MaxX = MaxY = -_DMAX;

	int i = 1;
	for (const Point& p : points)
	{
		Points.push_back(p);
		UpdateBounds(p);
		VoronoiSite* s = new VoronoiSite({ p, nullptr, nullptr, i });
		Sites.push_back(s);
		i++;
	}
}

VoronoiDiagram::VoronoiDiagram(std::string fileLocation)
{
	MinX = MinY = _DMAX;
	MaxX = MaxY = -_DMAX;
	std::cout << "Reading " << fileLocation << std::endl;
	std::fstream inputFile;
	inputFile.open(fileLocation, std::ios::in);

	if (inputFile.is_open())
	{
		int i = 1;
		double x, y;
		std::string sa;
		char discard;

		while (std::getline(inputFile, sa, ')')) {
			std::istringstream input(sa);
			if (sa.size() <= 1)
				continue;

			input >> discard >> x >> discard >> y;
			Point p = { x, y };
			Points.push_back(p);
			UpdateBounds(p);
			VoronoiSite* s = new VoronoiSite({ p, nullptr, nullptr, i });
			Sites.push_back(s);
			i++;
		}
	}
	else {
		std::cout << "Could not read " << std::endl;
	}
}

void VoronoiDiagram::PrintToFile(std::string fileLocation)
{
	std::fstream file(fileLocation, std::ios::out);
	if (file.is_open())
	{
		PrintVoronoiDCEL(file);
		file << std::endl;
		PrintDelaunayTriangulation(file);
	}
}

void VoronoiDiagram::PrintVoronoiDCEL(std::ostream& os)
{
	os << "****** Voronoi diagram ******" << std::endl;
	for (DCEL::Vertex* v : Vertices)
	{
		if (v->box) os << "b";
		else os << "v";
		os << v->index << " " << v->point << " ";
		if (nullptr == v->incidentEdge)
			os << "nil";
		else
			os << *v->incidentEdge;
		os << std::endl;
	}

	os << std::endl;

	for (DCEL::Face* f : Faces)
	{
		if (f->Unbounded)
			os << "uf" << " ";
		else
			os << "c" << f->site->index << " ";
		if (f->outerComponent) os << *f->outerComponent;
		else os << "nil";
		os << " ";
		if (f->innerComponent) os << *f->innerComponent;
		else os << "nil";

		os << std::endl;
	}

	os << std::endl;

	for (DCEL::HalfEdge* h : HalfEdges)
	{
		os << *h;
		os << " ";
		//os << "v" << h->origin->index;
		os << " ";
		if (h->twin) os << *h->twin;
		else os << "nil";
		if (h->incidentFace == nullptr)
			os << " nil ";
		else
		{
			if (h->incidentFace->Unbounded)
				os << " uf" << " ";
			else
				os << " c" << h->incidentFace->site->index << " ";
		}

		if (h->next) os << *h->next;
		else os << "nil";
		os << " ";
		if (h->prev) os << *h->prev;
		else os << "nil";

		os << std::endl;
	}
}

void VoronoiDiagram::PrintDelaunayTriangulation(std::ostream& os)
{
	os << "****** Delaunay triangulation ******" << std::endl;
	for (DCEL::Vertex* v : TriangulationVertices)
	{
		os << "v" << v->index << " " << v->point << " ";
		if (nullptr == v->incidentEdge)
			os << "nil";
		else
			os << *v->incidentEdge;
		os << std::endl;
	}

	os << std::endl;

	for (DCEL::Face* f : TriangulationFaces)
	{
		if (f->Unbounded)
			os << "uf" << " ";
		else
			os << "t" << f->index << " ";
		if (f->outerComponent) os << *f->outerComponent;
		else os << "nil";
		os << " ";
		if (f->innerComponent) os << *f->innerComponent;
		else os << "nil";

		os << std::endl;
	}

	os << std::endl;

	for (DCEL::HalfEdge* h : TriangulationHalfEdges)
	{
		os << *h;
		os << " ";
		os << "v" << h->origin->index << " ";
		if (h->twin) os << *h->twin;
		else os << "nil";
		if (h->incidentFace == nullptr)
			os << " nil ";
		else
		{
			if (h->incidentFace->Unbounded)
				os << " uf" << " ";
			else
				os << " c" << h->incidentFace->index << " ";
		}

		if (h->next) os << *h->next;
		else os << "nil";
		os << " ";
		if (h->prev) os << *h->prev;
		else os << "nil";

		os << std::endl;
	}
}

void VoronoiDiagram::UpdateBounds(const Point& point)
{
	if (point.y < MinY)
		MinY = point.y;
	if (point.y > MaxY)
		MaxY = point.y;
	if (point.x < MinX)
		MinX = point.x;
	if (point.x > MaxX)
		MaxX = point.x;
}


