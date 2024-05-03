#pragma once

#include "../types/VoronoiDiagram.h"

namespace BL
{
	class Arc;
	class Edge;
}
class EventPoint;
class PriorityQueue;

class FortunesAlgorithm
{
public:
	FortunesAlgorithm(VoronoiDiagram& diagram);
	~FortunesAlgorithm();

	void Continues(double height);
	void Run();
	void Next();

// Utility Functions
	void PrintTree();
	bool IsComplete();
	double GetHeight();
	const std::vector<BL::Arc*>& InOrder();
	const std::vector<BL::Edge*>& GetCompletedEdges() { return CompletedEdges; }
	const std::vector<BL::Edge*>& GetInfiniteEdges() { return IniniteEdges; }


private:
// Fortunes Functions
	void HandleSiteEvent(EventPoint* event);
	void HandleCircleEvent(EventPoint* site);
	void CheckForCircleEvent(BL::Arc* arc, bool potentinalVertexSplit = false);
	void CleanRemainingTree();
	void CleanZeroLengthEdges();
	void FillOuterEdgesIncidentFaces();
	void UpdateBounds(const Point& point);

// Tree Functions
	BL::Arc* FindArcAtX(double x);

	BL::Arc* GetLeftParent(BL::Arc* root);
	BL::Arc* GetRightParent(BL::Arc* root);

	BL::Arc* GetClosestLeftChild(BL::Arc* root);
	BL::Arc* GetClosestRightChild(BL::Arc* root);

	void RightRotation(BL::Arc* arc);
	void LeftRotation(BL::Arc* arc);
	void ReplaceParentsChild(BL::Arc* parent, BL::Arc* oldChild, BL::Arc* newChild);

	void FixRedBlackPropertiesAfterInsert(BL::Arc* arc);
	void FixRedBlackPropertiesAfterDelete(BL::Arc* arc);
	BL::Arc* GetUncle(BL::Arc* parent);

	void PrintTreeInOrder(BL::Arc* arc, int depth = 0);
	void TreeInOrder(BL::Arc* arc);

	VoronoiDiagram& Diagram;
	PriorityQueue* Queue;
	BL::Arc* Root;
	VoronoiSite* FirstSite;
	double SweepHeight;
	bool Complete;
	int NumVoronoiSites;
	int NumBoundingVertices;
	int NumTriangles;
	DCEL::Vertex* LastVistedVertex;

// Utility Variables
	std::vector<BL::Arc*> InOrderArcs;
	std::vector<BL::Edge*> CompletedEdges;
	std::vector<BL::Edge*> IniniteEdges;

public:
// Voronoi Needed Variables
	double MinX, MinY, MaxX, MaxY;
};

struct Point;

// We combine the ideas of a perpindicular bisector and a halfedge to create an edge
// These can be used to render the Voronoi Diagram as well
namespace BL
{
	class Edge
	{
	public:
		Edge(Point* start, VoronoiSite* left, VoronoiSite* right);
		~Edge();

		Point* Start;
		Point* End;
		VoronoiSite* Left;
		VoronoiSite* Right;
		DCEL::HalfEdge* HalfEdge;
		DCEL::HalfEdge* TriHalfEdge;

		Edge* Neighbour;
		Point Line;
		Point Direction;
		bool IsVertical;

		Point* Intersect(Edge* edge);
		Point RenderEdge(double y);
	};

	class Arc
	{
	public:
		enum TreeColor
		{
			Red,
			Black
		};

		Arc(VoronoiSite* site);
		Arc(Edge* site);
		VoronoiSite* Site;
		Edge* Edge;
		EventPoint* CircleEvent;
		Arc* Parent;
		Arc* Left;
		Arc* Right;


		bool Leaf;
		TreeColor Color;

		void SetLeft(Arc* a);
		void SetRight(Arc* a);
	};
}

// Parameters
//		left   : the left parabola focus point
//		right  : the right parabola focus point
//		lh     : the line height of the sweep line
// Uses equation presented on page 153 of Computational Geometry Algorithms and Applications 3rd Edition
// too calculate the intersection between two parabolas and return the point between them.
double IntersectionX(const Point& left, const Point& right, const double lh);

// Parameters
//		x     : the x coordinate on the parabola
//		lh    : the line height of the sweep line
//		point : the focus point of the parabola
// The equation presented on page 153 of Computational Geometry Algorithms and Applications 3rd Edition
double CalculateParabolaY(const double x, const double lh, const Point& point);

