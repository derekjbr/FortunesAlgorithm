#pragma once

#include "PriorityQueue.h"
#include "../types/Event.h"

#include <algorithm>
#include <vector>
#include <ostream>

PriorityQueue::PriorityQueue() : Elements()
{

}

void PriorityQueue::Push(EventPoint* e)
{
	Elements.push_back(e);
	percolateUp(Elements.size() - 1);
}

EventPoint* PriorityQueue::Peek()
{
	if (0 >= Elements.size())
		return nullptr;
	return Elements.at(0);
}

EventPoint* PriorityQueue::Pop()
{
	swap(0, Elements.size() - 1);
	EventPoint* top = Elements.back();
	Elements.pop_back();
	percolateDown(0);
	return top;
}

bool PriorityQueue::IsEmpty()
{
	return Elements.empty();
}

void PriorityQueue::percolateUp(size_t index)
{
	size_t parent = (index - 1) / 2;
	while (index > 0 && Elements[index]->isGreater(*Elements[parent]))
	{
		swap(index, parent);
		index = parent;
		parent = (index - 1) / 2;
	}
}

void PriorityQueue::percolateDown(size_t index)
{
	size_t child = 2 * index + 1;
	while (child < Elements.size())
	{
		if (child + 1 < Elements.size())
		{
			if (Elements[child + 1]->isGreater(*Elements[child]))
				child = child + 1;
		}

		if (Elements[index]->isGreater(*Elements[child]))
			return;

		swap(index, child);
		index = child;
		child = 2 * index + 1;
	}
}

void PriorityQueue::swap(size_t index, size_t swappedIndex)
{
	EventPoint* tmp = std::move(Elements[index]);
	Elements[index] = std::move(Elements[swappedIndex]);
	Elements[swappedIndex] = std::move(tmp);
}

std::ostream& operator<<(std::ostream& os, const PriorityQueue& queue)
{
	for (EventPoint* e : queue.Elements)
		os << "(" << e->Site->point.x << ", " << e->Site->point.y << ") ";
	return os;
}






