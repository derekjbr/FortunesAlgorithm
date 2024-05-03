#include "FortunesAlgorithm.h"

#include "../types/Point.h"
#include "../types/VoronoiDiagram.h"
#include "../utils/PriorityQueue.h"

#include <cmath>
#include <limits>

using namespace BL;

////////////////////////////////////////////////////////////////////
FortunesAlgorithm::FortunesAlgorithm(VoronoiDiagram& diagram)
	: Diagram(diagram)
	, Queue(new PriorityQueue())
	, Root(nullptr)
	, FirstSite(nullptr)
	, SweepHeight(DBL_MAX)
	, Complete(false)
	, NumVoronoiSites(0)
	, NumBoundingVertices(0)
	, LastVistedVertex(nullptr)
	, InOrderArcs()
	, CompletedEdges()
	, MinX(DBL_MAX)
	, MinY(DBL_MAX)
	, MaxX(-DBL_MAX)
	, MaxY(-DBL_MAX)
{
	std::cout << "Number of sites: " << Diagram.Sites.size() << std::endl;
	for (VoronoiSite* site : Diagram.Sites)
	{
		Queue->Push(new EventPoint(site));
	}

	SweepHeight = Queue->Peek()->Site->point.y;
}

////////////////////////////////////////////////////////////////////
FortunesAlgorithm::~FortunesAlgorithm()
{
	delete Queue;
}

void FortunesAlgorithm::Run()
{
	while (!Queue->IsEmpty())
	{
		Next();
	}

	CleanZeroLengthEdges();
	CleanRemainingTree();
	FillOuterEdgesIncidentFaces();

	Complete = true;
	std::cout << std::endl;
}


////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::Continues(double height)
{
	if (!Queue->IsEmpty() && height - Queue->Peek()->Site->point.y < -0.005)
	{
		Next();
	}
}


////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::Next()
{
	if (Queue->IsEmpty())
	{
		return;
	}

	EventPoint* top = Queue->Pop();
	SweepHeight = top->Site->point.y;
	// Handle Site Event
	if (EventPointType::Site == top->Type)
	{
		HandleSiteEvent(top);
	}
	// Handle Circle Event That needs to be deleted
	else if (!top->Deleted)
	{
		HandleCircleEvent(top);
	}
	delete top;
}


////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::HandleSiteEvent(EventPoint* site)
{
	// Maintain Records
	UpdateBounds(site->Site->point);
	site->Site->face = new DCEL::Face({ site->Site, nullptr, nullptr });
	Diagram.Faces.push_back(site->Site->face);
	Diagram.TriangulationVertices.push_back(new DCEL::Vertex({ site->Site->index, site->Site->point, nullptr }));
	DCEL::Vertex* triV = Diagram.TriangulationVertices.back();
	site->Site->triVertex = triV;

	if (nullptr == Root) { Root = new Arc(site->Site); FirstSite = Root->Site; return; };

	Point& p = site->Site->point;
	Arc* a = FindArcAtX(p.x);

	if (FirstSite->point.y - site->Site->point.y < .1)
	{
		double middle = (site->Site->point.x + a->Site->point.x) / 2.0;
		Point* start = new Point({ middle, SweepHeight });

		if (a->Site->point.x < site->Site->point.x)
		{
			a->Edge = new Edge(start, a->Site, site->Site);
			a->Edge->Neighbour = new Edge(start, site->Site, a->Site);
			a->Edge->Neighbour->Neighbour = a->Edge;
			a->SetLeft(new Arc(a->Site));
			a->SetRight(new Arc(site->Site));
		}
		else
		{
			a->Edge = new Edge(start, site->Site, a->Site);
			a->Edge->Neighbour = new Edge(start, site->Site, a->Site);
			a->Edge->Neighbour->Neighbour = a->Edge;
			a->SetRight(new Arc(site->Site));
			a->SetRight(new Arc(a->Site));
		}

		a->Leaf = false;
		if (a != Root) a->Color = Arc::TreeColor::Red;

		FixRedBlackPropertiesAfterInsert(a);
		PrintTree();
		return;
	}

	if (nullptr != a->CircleEvent)
	{
		a->CircleEvent->Deleted = true;
		a->CircleEvent = nullptr;
	}

	Point* edgeStart = new Point(p.x, CalculateParabolaY(p.x, SweepHeight, a->Site->point));
	Edge* el = new Edge(edgeStart, a->Site, site->Site);
	Edge* er = new Edge(edgeStart, site->Site, a->Site);

	el->Neighbour = er;
	er->Neighbour = el;

	a->Edge = er;
	a->Leaf = false;
	a->Color = Arc::TreeColor::Red;

	Arc* pl = new Arc(a->Site);
	Arc* pm = new Arc(site->Site);
	Arc* pr = new Arc(a->Site);

	Arc* elArc = new Arc(el);

	a->SetRight(pr);
	a->SetLeft(elArc);

	elArc->SetLeft(pl);
	elArc->SetRight(pm);

	CheckForCircleEvent(pl, true);
	CheckForCircleEvent(pr, true);
	
	// Balance (elArc)
	FixRedBlackPropertiesAfterInsert(a);
	FixRedBlackPropertiesAfterInsert(elArc);

	PrintTree();
	std::cout << std::endl;
}

////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::HandleCircleEvent(EventPoint* site)
{
	Arc* arc = site->Arc;

	Arc* leftEdge = GetLeftParent(arc);
	Arc* rightEdge = GetRightParent(arc);

	if (nullptr == leftEdge || nullptr == rightEdge) return;

	Arc* leftArc = GetClosestLeftChild(leftEdge);
	Arc* rightArc = GetClosestRightChild(rightEdge);

	if (nullptr != leftArc->CircleEvent && leftArc->CircleEvent->Site->point != site->Site->point)
	{ 
		leftArc->CircleEvent->Deleted = true; 
		leftArc->CircleEvent = nullptr;
	}

	if (nullptr != rightArc->CircleEvent && rightArc->CircleEvent->Site->point != site->Site->point)
	{
		rightArc->CircleEvent->Deleted = true;
		rightArc->CircleEvent = nullptr;
	}

	Point* vertex = new Point(site->Site->point.x, site->Site->point.y + site->Radius);
	leftEdge->Edge->End = vertex;
	rightEdge->Edge->End = vertex;

	CompletedEdges.push_back(leftEdge->Edge);
	CompletedEdges.push_back(rightEdge->Edge);

	//Maintain records
	UpdateBounds(*vertex);
	if(LastVistedVertex == nullptr || LastVistedVertex->point.x != vertex->x || LastVistedVertex->point.y != vertex->y)
	{ 
		Diagram.Vertices.emplace_back(new DCEL::Vertex({ ++NumVoronoiSites, *vertex, nullptr }));
		LastVistedVertex = Diagram.Vertices.back();
	}


	DCEL::HalfEdge* vNv1 = nullptr;
	DCEL::HalfEdge* v1vN = nullptr;
	DCEL::HalfEdge* vNv2 = nullptr;
	DCEL::HalfEdge* v2vN = nullptr;

	Edge* newEdge = new Edge(vertex, leftArc->Site, rightArc->Site);

	if (nullptr == leftEdge->Edge->HalfEdge)
	{
		vNv1 = new DCEL::HalfEdge({ Diagram.Vertices.back(), nullptr, nullptr, nullptr, nullptr, nullptr });
		v1vN = new DCEL::HalfEdge({ nullptr, Diagram.Vertices.back(),    vNv1, nullptr, nullptr, nullptr });
		vNv1->twin = v1vN;
		vNv1->incidentFace = leftEdge->Edge->Left->face;
		if (nullptr == vNv1->incidentFace->outerComponent) vNv1->incidentFace->outerComponent = vNv1;
		v1vN->incidentFace = leftEdge->Edge->Right->face;
		if (nullptr == v1vN->incidentFace->outerComponent) v1vN->incidentFace->outerComponent = v1vN;

		leftEdge->Edge->HalfEdge = vNv1;
		leftEdge->Edge->Neighbour->HalfEdge = v1vN;
		Diagram.HalfEdges.emplace_back(vNv1);
		Diagram.HalfEdges.emplace_back(v1vN);
	}
	else {
		vNv1 = leftEdge->Edge->HalfEdge;
		v1vN = vNv1->twin;
		vNv1->origin = Diagram.Vertices.back();
		v1vN->dest = Diagram.Vertices.back();
	}

	if (nullptr == rightEdge->Edge->HalfEdge)
	{
		vNv2 = new DCEL::HalfEdge({ Diagram.Vertices.back(), nullptr, nullptr, nullptr, nullptr, nullptr });
		v2vN = new DCEL::HalfEdge({ nullptr, Diagram.Vertices.back(),    vNv2, nullptr, nullptr, nullptr });
		vNv2->twin = v2vN;
		vNv2->incidentFace = rightEdge->Edge->Left->face;
		if (nullptr == vNv2->incidentFace->outerComponent) vNv2->incidentFace->outerComponent = vNv2;
		v2vN->incidentFace = rightEdge->Edge->Right->face;
		if (nullptr == v2vN->incidentFace->outerComponent) v2vN->incidentFace->outerComponent = v2vN;

		rightEdge->Edge->HalfEdge = vNv2;
		rightEdge->Edge->Neighbour->HalfEdge = v2vN;
		Diagram.HalfEdges.emplace_back(vNv2);
		Diagram.HalfEdges.emplace_back(v2vN);
	}
	else {
		vNv2 = rightEdge->Edge->HalfEdge;
		v2vN = vNv2->twin;
		vNv2->origin = Diagram.Vertices.back();
		v2vN->dest = Diagram.Vertices.back();
	}

	DCEL::HalfEdge* vNv3 = new DCEL::HalfEdge({ Diagram.Vertices.back(), nullptr, nullptr, nullptr, nullptr, nullptr });
	DCEL::HalfEdge* v3vN = new DCEL::HalfEdge({ nullptr, Diagram.Vertices.back(),    vNv3, nullptr, nullptr, nullptr });
	vNv3->twin = v3vN;
	newEdge->HalfEdge = v3vN;
	vNv3->incidentFace = rightEdge->Edge->Right->face;
	if (nullptr == vNv3->incidentFace->outerComponent) vNv3->incidentFace->outerComponent = vNv3;
	v3vN->incidentFace = leftEdge->Edge->Left->face;
	if (nullptr == v3vN->incidentFace->outerComponent) v3vN->incidentFace->outerComponent = v3vN;

	v1vN->next = vNv2;
	vNv2->prev = v1vN;

	v2vN->next = vNv3;
	vNv3->prev = v2vN;

	v3vN->next = vNv1;
	vNv1->prev = v3vN;

	Diagram.HalfEdges.emplace_back(v3vN);
	Diagram.HalfEdges.emplace_back(vNv3);

	if (v1vN->origin != nullptr && v1vN->origin->incidentEdge == nullptr)
		v1vN->origin->incidentEdge = v1vN;

	if (v2vN->origin != nullptr && v2vN->origin->incidentEdge == nullptr)
		v2vN->origin->incidentEdge = v2vN;

	if(vNv3->origin != nullptr && vNv3->origin->incidentEdge == nullptr)
		vNv3->origin->incidentEdge = vNv3;



	// Delany
	// Test for test turn
	bool leftTurn = ((rightArc->Site->triVertex->point.x - arc->Site->triVertex->point.x) * (leftArc->Site->triVertex->point.y - rightArc->Site->triVertex->point.y)
		- (rightArc->Site->triVertex->point.y - arc->Site->triVertex->point.y) * (leftArc->Site->triVertex->point.x - rightArc->Site->triVertex->point.x) > 0);

	DCEL::Vertex* v1 = (leftTurn) ? rightArc->Site->triVertex : leftArc->Site->triVertex;
	DCEL::Vertex* v2 = (leftTurn) ? leftArc->Site->triVertex : rightArc->Site->triVertex;

	DCEL::Face* tri = new DCEL::Face({nullptr, nullptr, nullptr, false, ++NumTriangles});

	DCEL::HalfEdge* e1 = new DCEL::HalfEdge({ arc->Site->triVertex, v1, nullptr, tri, nullptr, nullptr });
	DCEL::HalfEdge* e2 = new DCEL::HalfEdge({ v1, v2, nullptr, tri, nullptr, e1 });
	DCEL::HalfEdge* e3 = new DCEL::HalfEdge({ v2, arc->Site->triVertex, nullptr, tri, e1, e2 });
	e1->prev = e3;
	e1->next = e2;
	e2->next = e3;

	if (e1->origin->incidentEdge == nullptr)
		e1->origin->incidentEdge = e1;
	if (e2->origin->incidentEdge == nullptr)
		e2->origin->incidentEdge = e2;
	if (e3->origin->incidentEdge == nullptr)
		e3->origin->incidentEdge = e3;

	tri->outerComponent = e1;

	if (leftTurn)
	{
		e1->twin = rightEdge->Edge->TriHalfEdge;
		if (e1->twin != nullptr) e1->twin->twin = e1;
		if (rightEdge->Edge->Neighbour != nullptr)  rightEdge->Edge->Neighbour->TriHalfEdge = e1;

		e3->twin = leftEdge->Edge->TriHalfEdge;
		if (e3->twin != nullptr) e3->twin->twin = e3;
		if (leftEdge->Edge->Neighbour != nullptr)  leftEdge->Edge->Neighbour->TriHalfEdge = e3;
	} else
	{
		e1->twin = leftEdge->Edge->TriHalfEdge;
		if (e1->twin != nullptr) e1->twin->twin = e1;
		if (leftEdge->Edge->Neighbour != nullptr)  leftEdge->Edge->Neighbour->TriHalfEdge = e1;
		
		e3->twin = rightEdge->Edge->TriHalfEdge;
		if (e3->twin != nullptr) e3->twin->twin = e3;
		if (rightEdge->Edge->Neighbour != nullptr)  rightEdge->Edge->Neighbour->TriHalfEdge = e3;
	}

	newEdge->TriHalfEdge = e2;

	Diagram.TriangulationHalfEdges.push_back(e1);
	Diagram.TriangulationHalfEdges.push_back(e2);
	Diagram.TriangulationHalfEdges.push_back(e3);
	Diagram.TriangulationFaces.push_back(tri);

	// Clean Tree
	Arc* higherNode = nullptr;
	Arc* tmp = arc;

	while (tmp->Parent != nullptr)
	{
		tmp = tmp->Parent;
		if (tmp == leftEdge)
			higherNode = leftEdge;

		if (tmp == rightEdge)
			higherNode = rightEdge;
	}

	higherNode->Edge = newEdge;

	Arc* grandParent = arc->Parent->Parent;
	Arc* movedUp = nullptr;
	if (arc == arc->Parent->Left)
	{
		movedUp = arc->Parent->Right;
		if (grandParent->Left == arc->Parent)
		{
			grandParent->SetLeft(arc->Parent->Right);

		}
		else {
			grandParent->SetRight(arc->Parent->Right);
		}
	}
	else
	{
		movedUp = arc->Parent->Left;
		if (grandParent->Left == arc->Parent)
		{
			grandParent->SetLeft(arc->Parent->Left);
		}
		else {
			grandParent->SetRight(arc->Parent->Left);

		}
	}

	if (arc->Parent->Color == Arc::TreeColor::Black)
	{
		FixRedBlackPropertiesAfterDelete(movedUp);
	}
	delete arc->Parent;
	delete arc;

	CheckForCircleEvent(leftArc);
	CheckForCircleEvent(rightArc);

	}

////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::CleanRemainingTree()
{
	DCEL::Face* unbounded = new DCEL::Face({ nullptr, nullptr, nullptr, true });
	DCEL::Face* triUnbounded = new DCEL::Face({ nullptr, nullptr, nullptr, true, ++NumTriangles });
	Diagram.Faces.push_back(unbounded);
	Diagram.TriangulationFaces.push_back(triUnbounded);

	std::vector<DCEL::HalfEdge*> boundingEdges;
	DCEL::Vertex* b1 = new DCEL::Vertex({ ++NumBoundingVertices, Point({ MinX - 5.0, MinY - 5.0 }), nullptr, true });
	DCEL::Vertex* b2 = new DCEL::Vertex({ ++NumBoundingVertices, Point({ MaxX + 5.0, MinY - 5.0 }), nullptr, true });
	DCEL::Vertex* b3 = new DCEL::Vertex({ ++NumBoundingVertices, Point({ MaxX + 5.0, MaxY + 5.0 }), nullptr, true });
	DCEL::Vertex* b4 = new DCEL::Vertex({ ++NumBoundingVertices, Point({ MinX - 5.0, MaxY + 5.0 }), nullptr, true });

	Diagram.Vertices.push_back(b1);
	Diagram.Vertices.push_back(b2);
	Diagram.Vertices.push_back(b3);
	Diagram.Vertices.push_back(b4);

	boundingEdges.push_back(new DCEL::HalfEdge({ b1, b2, nullptr, nullptr, nullptr, nullptr }));
	b1->incidentEdge = boundingEdges.back();
	boundingEdges.push_back(new DCEL::HalfEdge({ b2, b1, boundingEdges.back(), nullptr, nullptr, nullptr}));
	boundingEdges.back()->incidentFace = unbounded;
	unbounded->innerComponent = boundingEdges.back();
	boundingEdges.back()->twin->twin = boundingEdges.back();
	boundingEdges.push_back(new DCEL::HalfEdge({ b2, b3, nullptr, nullptr, nullptr, nullptr }));
	b2->incidentEdge = boundingEdges.back();
	boundingEdges.push_back(new DCEL::HalfEdge({ b3, b2, boundingEdges.back(), nullptr, nullptr, nullptr }));
	boundingEdges.back()->incidentFace = unbounded;
	boundingEdges.back()->twin->twin = boundingEdges.back();
	boundingEdges.push_back(new DCEL::HalfEdge({ b3, b4, nullptr, nullptr, nullptr, nullptr }));
	b3->incidentEdge = boundingEdges.back();
	boundingEdges.push_back(new DCEL::HalfEdge({ b4, b3, boundingEdges.back(), nullptr, nullptr, nullptr }));
	boundingEdges.back()->incidentFace = unbounded;
	boundingEdges.back()->twin->twin = boundingEdges.back();
	boundingEdges.push_back(new DCEL::HalfEdge({ b4, b1, nullptr, nullptr, nullptr, nullptr }));
	b4->incidentEdge = boundingEdges.back();
	boundingEdges.push_back(new DCEL::HalfEdge({ b1, b4, boundingEdges.back(), nullptr, nullptr, nullptr }));
	boundingEdges.back()->incidentFace = unbounded;
	boundingEdges.back()->twin->twin = boundingEdges.back();

	// First loop
	boundingEdges[0]->next = boundingEdges[2];
	boundingEdges[0]->prev = boundingEdges[6];
	boundingEdges[2]->next = boundingEdges[4];
	boundingEdges[2]->prev = boundingEdges[0];
	boundingEdges[4]->next = boundingEdges[6];
	boundingEdges[4]->prev = boundingEdges[2];
	boundingEdges[6]->next = boundingEdges[0];
	boundingEdges[6]->prev = boundingEdges[4];

	// Other loop
	boundingEdges[1]->next = boundingEdges[7];
	boundingEdges[1]->prev = boundingEdges[3];
	boundingEdges[3]->next = boundingEdges[1];
	boundingEdges[3]->prev = boundingEdges[5];
	boundingEdges[5]->next = boundingEdges[3];
	boundingEdges[5]->prev = boundingEdges[7];
	boundingEdges[7]->next = boundingEdges[5];
	boundingEdges[7]->prev = boundingEdges[1];

	for (Arc* arc : InOrder())
	{
		if (arc->Edge != nullptr)
		{
			// Voronoi Diagram
			double x, y;
			for (int i = 0; i < boundingEdges.size(); i+=2)
			{
				bool intersectingCorner = false;
				bool intersecting = false;
				DCEL::HalfEdge* edge = boundingEdges[i];

				if (arc->Edge->IsVertical)
				{
					x = arc->Edge->Start->x;
					y = edge->origin->point.y;

					intersecting = (y - arc->Edge->Start->y) / (arc->Edge->Direction.y) > 0;
				} else if (edge->origin->point.x - edge->dest->point.x == 0)
				{
					x = edge->origin->point.x;
					y = arc->Edge->Line.x * x + arc->Edge->Line.y;
					intersecting = (y < std::max(edge->origin->point.y, edge->dest->point.y)
						&& y > std::min(edge->origin->point.y, edge->dest->point.y)
						&& (x - arc->Edge->Start->x) / (arc->Edge->Direction.x) > 0);

					intersectingCorner = (y == edge->origin->point.y);

				}
				else if (edge->origin->point.y - edge->dest->point.y == 0)
				{
					y = edge->origin->point.y;
					x = (y - arc->Edge->Line.y) / arc->Edge->Line.x;

					intersecting = (x < std::max(edge->origin->point.x, edge->dest->point.x)
						&& x > std::min(edge->origin->point.x, edge->dest->point.x)
						&& (y - arc->Edge->Start->y) / (arc->Edge->Direction.y) > 0);

					intersectingCorner = (x == edge->origin->point.x);
				}

				if ((intersecting || intersectingCorner) && arc->Edge->HalfEdge == nullptr)
				{
					arc->Edge->HalfEdge = new DCEL::HalfEdge({ nullptr, nullptr, nullptr, arc->Edge->Left->face, nullptr, nullptr });
					arc->Edge->HalfEdge->twin = new DCEL::HalfEdge({ nullptr, nullptr, arc->Edge->HalfEdge, arc->Edge->Right->face, nullptr, nullptr });

					if (arc->Edge->Left->face->outerComponent == nullptr) arc->Edge->Left->face->outerComponent = arc->Edge->HalfEdge;
					if (arc->Edge->Right->face->outerComponent == nullptr) arc->Edge->Right->face->outerComponent = arc->Edge->HalfEdge->twin;

					Diagram.HalfEdges.push_back(arc->Edge->HalfEdge);
					Diagram.HalfEdges.push_back(arc->Edge->HalfEdge->twin);

					if(arc->Edge->Neighbour != nullptr)
						arc->Edge->Neighbour->HalfEdge = arc->Edge->HalfEdge->twin;
				}

				if (intersecting)
				{
					DCEL::Vertex* b = new DCEL::Vertex({ ++NumBoundingVertices, Point({ x, y }), nullptr, true });
					Diagram.Vertices.push_back(b);

					// Create new half edge
					DCEL::HalfEdge* e1eB = new DCEL::HalfEdge({ edge->origin, b, nullptr, nullptr, nullptr, nullptr });
					e1eB->prev = edge->prev;
					e1eB->next = arc->Edge->HalfEdge;
					e1eB->incidentFace = arc->Edge->HalfEdge->incidentFace;
					edge->prev->next = e1eB;

					// Create new Half edge
					DCEL::HalfEdge* eBe1 = new DCEL::HalfEdge({ b, edge->origin, e1eB, nullptr, nullptr, nullptr });
					eBe1->incidentFace = unbounded;
					eBe1->next = edge->twin->next;
					eBe1->prev = edge->twin;
					e1eB->twin = eBe1;

					// Update old records
					edge->origin = b;
					edge->twin->dest = b;

					arc->Edge->HalfEdge->origin = b;
					arc->Edge->HalfEdge->twin->dest = b;
					arc->Edge->HalfEdge->twin->next = edge;
					arc->Edge->HalfEdge->prev = e1eB;

					edge->prev = arc->Edge->HalfEdge->twin;
					edge->incidentFace = arc->Edge->HalfEdge->twin->incidentFace;

					// Provide incident edges
					b->incidentEdge = edge;
					e1eB->origin->incidentEdge = e1eB;

					boundingEdges.push_back(e1eB);
					boundingEdges.push_back(eBe1);

					break;
				}
				else if (intersectingCorner)
				{
					arc->Edge->HalfEdge->origin = edge->origin;
					arc->Edge->HalfEdge->twin->dest= edge->origin;

					arc->Edge->HalfEdge->twin->next = edge;
					arc->Edge->HalfEdge->prev = edge->prev;
					edge->prev->next = arc->Edge->HalfEdge;
					edge->prev->incidentFace = arc->Edge->HalfEdge->incidentFace;

					edge->prev = arc->Edge->HalfEdge->twin;
					edge->incidentFace = arc->Edge->HalfEdge->twin->incidentFace;
					break;
				}
			}

			// Triangulation
			if (arc->Edge->TriHalfEdge != nullptr)
			{
				arc->Edge->TriHalfEdge->twin = new DCEL::HalfEdge({ arc->Edge->TriHalfEdge->dest, arc->Edge->TriHalfEdge->origin ,arc->Edge->TriHalfEdge, triUnbounded, nullptr, nullptr });
				Diagram.TriangulationHalfEdges.push_back(arc->Edge->TriHalfEdge->twin);

				if (triUnbounded->innerComponent == nullptr)
				{
					triUnbounded->innerComponent = arc->Edge->TriHalfEdge->twin;
				}
			}

			IniniteEdges.push_back(arc->Edge);
			arc->Edge->End = new Point({ arc->Edge->Start->x + 10.0 * arc->Edge->Direction.x,
				arc->Edge->Start->y + 10.0 * arc->Edge->Direction.y });
		}
		delete arc;
	}

	for (DCEL::HalfEdge* edge : boundingEdges)
	{
		Diagram.HalfEdges.push_back(edge);
	}

	// Finish Delauny 
	DCEL::HalfEdge* start = triUnbounded->innerComponent;
	DCEL::HalfEdge* cur = nullptr;
	Diagram.PrintDelaunayTriangulation(std::cout);
	while (start != cur)
	{
		if (cur == nullptr) cur = start;
		DCEL::HalfEdge* prev = cur->twin;
		while (prev->incidentFace != triUnbounded)
		{
			prev = prev->next->twin;
		}
		cur->prev = prev;
		prev->next = cur;

		cur = prev;
	}
	

	Root == nullptr;
}

////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::CleanZeroLengthEdges()
{
	std::vector<DCEL::HalfEdge* >::iterator it = Diagram.HalfEdges.begin();
	while(it != Diagram.HalfEdges.end())
	{
		DCEL::HalfEdge* edge = *it;
		if (edge->origin == edge->dest)
		{
			edge->next->prev = edge->prev;
			edge->prev->next = edge->next;
			edge->twin = nullptr;
			it = Diagram.HalfEdges.erase(it);
		}
		else {
			it++;
		}
	}
}

////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::FillOuterEdgesIncidentFaces()
{
	DCEL::HalfEdge* start = Diagram.Faces.back()->innerComponent->twin;
	DCEL::HalfEdge* cur = nullptr;
	while (start != cur)
	{
		if (cur == nullptr)
			cur = start;

		if (cur->incidentFace == nullptr)
			cur->incidentFace = cur->prev->incidentFace;

		cur = cur->twin->next->twin;
	}
}


////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::UpdateBounds(const Point& point)
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

////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::CheckForCircleEvent(Arc* arc, bool potentinalVertexSplit)
{
	Arc* leftEdge = GetLeftParent(arc);
	Arc* rightEdge = GetRightParent(arc);


	if (nullptr == rightEdge || nullptr == leftEdge)
		return;

	Arc* leftArc = GetClosestLeftChild(leftEdge);
	Arc* rightArc = GetClosestRightChild(rightEdge);

	if (nullptr == leftArc || nullptr == rightArc || leftArc->Site == rightArc->Site) return;

	Point* intersection = leftEdge->Edge->Intersect(rightEdge->Edge);
	if (nullptr == intersection) return;

	double changeX = arc->Site->point.x - intersection->x;
	double changeY = arc->Site->point.y - intersection->y;

	double distance = std::sqrt((changeX * changeX) + (changeY * changeY));

	if (intersection->y - distance> SweepHeight || (!potentinalVertexSplit && intersection->y - distance >= SweepHeight)) return;

	EventPoint* circleEvent = new EventPoint(
		new VoronoiSite({ Point(intersection->x, intersection->y - distance), nullptr, nullptr, -1 }), EventPointType::Circle);

	arc->CircleEvent = circleEvent;
	circleEvent->Arc = arc;
	circleEvent->Radius = distance;

	Queue->Push(circleEvent);
}


////////////////////////////////////////////////////////////////////
Arc* FortunesAlgorithm::FindArcAtX(double x)
{
	BL::Arc* root = Root;
	while (!root->Leaf)
	{
		// Error - A leaf has become an internal node (only edge nodes should be internal)
		if (nullptr == root->Edge) return nullptr;
		if (root->Edge->IsVertical && x < root->Edge->Start->x ||
			x < IntersectionX(root->Edge->Left->point, root->Edge->Right->point, SweepHeight))
		{
			root = root->Left;
		}
		else
			root = root->Right;
	}
	return root;
}

////////////////////////////////////////////////////////////////////
Arc* FortunesAlgorithm::GetLeftParent(Arc* root)
{
	Arc* parent = root->Parent;
	while (nullptr != parent && parent->Left == root)
	{
		root = parent;
		parent = parent->Parent;
	}
	return parent;
}

////////////////////////////////////////////////////////////////////
Arc* FortunesAlgorithm::GetRightParent(Arc* root)
{
	Arc* parent = root->Parent;
	while (nullptr != parent && parent->Right == root)
	{
		root = parent;
		parent = parent->Parent;
	}
	return parent;
}

////////////////////////////////////////////////////////////////////
Arc* FortunesAlgorithm::GetClosestLeftChild(Arc* root)
{
	Arc* child = root->Left;
	while (nullptr != child && !child->Leaf)
	{
		child = child->Right;
	}
	return child;
}

////////////////////////////////////////////////////////////////////
Arc* FortunesAlgorithm::GetClosestRightChild(Arc* root)
{
	Arc* child = root->Right;
	while (nullptr != child && !child->Leaf)
	{
		child = child->Left;
	}
	return child;
}

////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::RightRotation(Arc* arc)
{
	Arc* parent = arc->Parent;
	Arc* leftChild = arc->Left;

	arc->Left = leftChild->Right;
	if (leftChild->Right != nullptr) {
		leftChild->Right->Parent = arc;
	}

	leftChild->Right = arc;
	arc->Parent = leftChild;

	ReplaceParentsChild(parent, arc, leftChild);
}

////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::LeftRotation(Arc* arc) {
	Arc* parent = arc->Parent;
	Arc* rightChild = arc->Right;

	arc->Right = rightChild->Left;
	if (rightChild->Left != nullptr) {
		rightChild->Left->Parent = arc;
	}

	rightChild->Left = arc;
	arc->Parent = rightChild;

	ReplaceParentsChild(parent, arc, rightChild);
}

////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::ReplaceParentsChild(Arc* parent, Arc* oldChild, Arc* newChild)
{
	if (parent == nullptr) {
		Root = newChild;
	}
	else if (parent->Left == oldChild) {
		parent->Left = newChild;
	}
	else if (parent->Right == oldChild) {
		parent->Right = newChild;
	}

	if (newChild != nullptr) {
		newChild->Parent = parent;
	}
}

////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::FixRedBlackPropertiesAfterInsert(Arc* arc)
{
	Arc* parent = arc->Parent;

	if (parent == nullptr) {
		arc->Color = Arc::TreeColor::Black;
		return;
	}

	if (parent->Color == Arc::TreeColor::Black) {
		return;
	}

	Arc* grandparent = parent->Parent;

	if (grandparent == nullptr) {
		parent->Color = Arc::TreeColor::Black;
		return;
	}

	Arc* uncle = GetUncle(parent);

	if (uncle != nullptr && uncle->Color == Arc::TreeColor::Red) {
		parent->Color = Arc::TreeColor::Black;
		grandparent->Color = Arc::TreeColor::Red;
		uncle->Color = Arc::TreeColor::Black;

		FixRedBlackPropertiesAfterInsert(grandparent);
	}

	else if (parent == grandparent->Left) {
		if (arc == parent->Right) {
			LeftRotation(parent);

			parent = arc;
		}

		RightRotation(grandparent);

		parent->Color = Arc::TreeColor::Black;
		grandparent->Color = Arc::TreeColor::Red;
	}

	else {
		if (arc == parent->Left) {
			RightRotation(parent);
			parent = arc;
		}

		LeftRotation(grandparent);

		parent->Color = Arc::TreeColor::Black;
		grandparent->Color = Arc::TreeColor::Red;
	}
}

////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::FixRedBlackPropertiesAfterDelete(Arc* arc)
{
	if (arc == Root) {
		arc->Color = Arc::TreeColor::Black;
		return;
	}

	Arc* sibling = (arc == arc->Parent->Left) ? arc->Parent->Right : arc->Parent->Left;

	if (sibling->Color == Arc::TreeColor::Red) {
		sibling->Color = Arc::TreeColor::Black;
		arc->Parent->Color = Arc::TreeColor::Red;
		if (arc == arc->Parent->Left) {
			LeftRotation(arc->Parent);
		}
		else {
			RightRotation(arc->Parent);
		}
		sibling = (arc == arc->Parent->Left) ? arc->Parent->Right : arc->Parent->Left;
	}

	if (sibling->Left->Color == Arc::TreeColor::Black && sibling->Right->Color == Arc::TreeColor::Black) {
		sibling->Color = Arc::TreeColor::Red;

		if (arc->Parent->Color == Arc::TreeColor::Red) {
			arc->Parent->Color = Arc::TreeColor::Black;
		}

		else 
		{
			FixRedBlackPropertiesAfterDelete(arc->Parent);
		}
	} else 
	{
		bool isLeftChild = arc == arc->Parent->Left;

		if (isLeftChild && sibling->Right->Color == Arc::TreeColor::Black) {
			sibling->Left->Color = Arc::TreeColor::Black;
			sibling->Color = Arc::TreeColor::Red;
			RightRotation(sibling);
			sibling = arc->Parent->Right;
		}
		else if (!isLeftChild && sibling->Left->Color == Arc::TreeColor::Black) {
			sibling->Right->Color = Arc::TreeColor::Black;
			sibling->Color = Arc::TreeColor::Red;
			LeftRotation(sibling);
			sibling = arc->Parent->Left;
		}

		sibling->Color = arc->Parent->Color;
		arc->Parent->Color = Arc::TreeColor::Black;
		if (isLeftChild) {
			sibling->Right->Color = Arc::TreeColor::Black;
			LeftRotation(arc->Parent);
		}
		else {
			sibling->Left->Color = Arc::TreeColor::Black;
			RightRotation(arc->Parent);
		}
	}
}

////////////////////////////////////////////////////////////////////
Arc* FortunesAlgorithm::GetUncle(Arc* parent) {
	Arc* grandparent = parent->Parent;
	if (grandparent->Left == parent) {
		return grandparent->Right;
	}
	else if (grandparent->Right == parent) {
		return grandparent->Left;
	}
}



////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::PrintTree()
{
	if(nullptr != Root)
		PrintTreeInOrder(Root);
}

////////////////////////////////////////////////////////////////////
bool FortunesAlgorithm::IsComplete()
{
	return Complete;
}

////////////////////////////////////////////////////////////////////
double FortunesAlgorithm::GetHeight()
{
	return SweepHeight;
}

////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::PrintTreeInOrder(Arc* a, int depth)
{
	if (nullptr != a->Left)
		PrintTreeInOrder(a->Left, depth + 1);

	if (nullptr != a->Edge)
		std::cout << "<" << a->Edge->Left->index << ", " << a->Edge->Right->index << "> ";
	else if (nullptr != a->Site)
		std::cout << "P" << a->Site->index << " ";

	if (nullptr != a->Right)
		PrintTreeInOrder(a->Right, depth + 1);
}

////////////////////////////////////////////////////////////////////
const std::vector<Arc*>& FortunesAlgorithm::InOrder()
{
	InOrderArcs.clear();
	if (nullptr != Root)
		TreeInOrder(Root);

	return InOrderArcs;
}

////////////////////////////////////////////////////////////////////
void FortunesAlgorithm::TreeInOrder(BL::Arc* a)
{
	if (nullptr != a->Left)
		TreeInOrder(a->Left);

	InOrderArcs.push_back(a);

	if (nullptr != a->Right)
		TreeInOrder(a->Right);
}


////////////////////////////////////////////////////////////////////
BL::Arc::Arc(VoronoiSite* site)
	: Site(site)
	, Edge(nullptr)
	, CircleEvent(nullptr)
	, Parent(nullptr)
	, Left(nullptr)
	, Right(nullptr)
	, Leaf(true)
	, Color(Arc::TreeColor::Black)
{
}

////////////////////////////////////////////////////////////////////
Arc::Arc(BL::Edge* edge)
	: Site(nullptr)
	, Edge(edge)
	, CircleEvent(nullptr)
	, Parent(nullptr)
	, Left(nullptr)
	, Right(nullptr)
	, Leaf(false)
	, Color(Arc::TreeColor::Red)
{
}

////////////////////////////////////////////////////////////////////
void Arc::SetLeft(Arc* a)
{
	a->Parent = this;
	Left = a;
}

////////////////////////////////////////////////////////////////////
void Arc::SetRight(Arc* a)
{
	a->Parent = this;
	Right = a;
}


////////////////////////////////////////////////////////////////////
BL::Edge::Edge(Point* start, VoronoiSite* left, VoronoiSite* right)
	: Start(start)
	, End(nullptr)
	, Left(left)
	, Right(right)
	, HalfEdge(nullptr)
	, TriHalfEdge(nullptr)
	, Neighbour(nullptr)
	, Line({ 0,0 })
	, Direction({ 0,0 })
	, IsVertical(false)
{
	if (left->point.y == right->point.y)
		IsVertical = true;

	Line.x = (right->point.x - left->point.x) / (left->point.y - right->point.y);
	Line.y = (start->y - Line.x * start->x);

	Direction.x = (right->point.y - left->point.y);
	Direction.y = (left->point.x - right->point.x);
}

////////////////////////////////////////////////////////////////////
BL::Edge::~Edge()
{
}

////////////////////////////////////////////////////////////////////
Point* Edge::Intersect(Edge* edge)
{
	// Test for parellel
	if (Line.x == edge->Line.x) return nullptr;
	if (IsVertical && edge->IsVertical)
	{
		return nullptr;
	}

	// Cacluation intersection
	double x = 0.0, y = 0.0;
	if (edge->IsVertical)
	{
		x = edge->Start->x;
		y = Line.x * x + Line.y;
	}
	else if (IsVertical)
	{
		x = Start->x;
		y = edge->Line.x * x + edge->Line.y;
	}
	else {
		x = (edge->Line.y - Line.y) / (Line.x - edge->Line.x);
		y = Line.x * x + Line.y;
	}

	// Test to ensure its on proper side of the line
	if ((x - Start->x) / (Direction.x) < 0.0) return nullptr;
	if (Direction.y && (y - Start->y) / (Direction.y) < 0.0) return nullptr;

	if ((x - edge->Start->x) / (edge->Direction.x) < 0.0) return nullptr;
	if (edge->Direction.y && (y - edge->Start->y) / (edge->Direction.y) < 0.0) return nullptr;

	Point* intersection = new Point(x, y);
	return intersection;
}


////////////////////////////////////////////////////////////////////
Point Edge::RenderEdge(double y)
{
	double x = IntersectionX(Left->point, Right->point, y);
	return Point(x, CalculateParabolaY(x, y, Left->point));
}

////////////////////////////////////////////////////////////////////
double IntersectionX(const Point& left, const Point& right, const double lh)
{
	const double dl = 1.0 / (2.0 * (left.y - lh));
	const double dr = 1.0 / (2.0 * (right.y - lh));

	// Quadratic Coffecients
	const double a = dl - dr;
	const double b = -2.0 * dl * left.x + 2.0 * dr * right.x;
	const double c = dl * (left.x * left.x + left.y * left.y - lh * lh)
		- dr * (right.x * right.x + right.y * right.y - lh * lh);

	double x1 = (-b + sqrt(b * b - 4 * a * c)) / (2.0 * a);
	double x2 = (-b - sqrt(b * b - 4 * a * c)) / (2.0 * a);

	if (left.y < right.y)
		return std::max(x1, x2);
	else
		return std::min(x2, x1);
}

////////////////////////////////////////////////////////////////////
double CalculateParabolaY(const double x, const double lh, const Point& point)
{
	double px = point.x;
	double py = point.y;

	double d = 2.0 * (py - lh);

	if (d == 0) return point.y;

	double cof = 1.0 / d;

	return cof * (x * x - 2.0 * px * x + px * px + py * py - lh * lh);
}


