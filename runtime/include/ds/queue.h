#ifndef _QUEUE_H
#define _QUEUE_H

#include <ds/list.h>

typedef ListNode _QueueNode;

class _Queue : public List {
public:
	inline _QueueNode* Enqueue(_QueueNode*);
	inline _QueueNode* Dequeue();
};

inline _QueueNode* _Queue::Enqueue(_QueueNode *element)
{
	return AddToTail(element);
}

inline _QueueNode* _Queue::Dequeue()
{
	_QueueNode *node = GetHead();
	if (node == 0)
		return 0;

	node->RemoveFromList();
	return node;
}           

#endif
