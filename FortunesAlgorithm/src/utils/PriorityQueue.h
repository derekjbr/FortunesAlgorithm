#pragma once

#include "../types/Event.h"

#include <ostream>
#include <vector>
// 2i+1 and 2i + 2, and its parent's index is floor((i − 1)/2)
class PriorityQueue
{
public:
	PriorityQueue();

	void Push(EventPoint* e);
	EventPoint* Peek();
	EventPoint* Pop();

	bool IsEmpty();

	std::vector<EventPoint*> Elements;

private:
	void percolateUp(size_t index);
	void percolateDown(size_t index);
	void swap(size_t index, size_t swappedIndex);
};

std::ostream& operator<<(std::ostream& os, const PriorityQueue& queue);

