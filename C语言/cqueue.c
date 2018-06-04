#include <stdlib.h>
#include <stdio.h>
#include "cqueue.h"

#ifdef __cplusplus
extern "C"{
#endif

void cqueue_init(cqueue *p)
{
	p->front = (cqueuenode *)malloc(sizeof(cqueuenode));
	p->front->data = NULL;
	p->rear = p->front;
	p->size = 0;
	(p->front)->next = null;

	p->lock = cmtx_create();
}

status cqueue_destory(cqueue *q)
{
	cmtx_enter(q->lock);
	while (q->front)
	{
		q->rear = q->front->next;
		if(q->front->data)
			free(q->front->data);
		free(q->front);
		q->front = q->rear;
	}
	q->size = 0;
	cmtx_leave(q->lock);
	cmtx_delete(q->lock);
	return 0;
}

void cqueue_clear(cqueue *q)
{
	cmtx_enter(q->lock);
	
	while (cqueue_is_empty(q) != 1)
	{
		q->rear = q->front->next;
		if(q->front->data)
			free(q->front->data);
		free(q->front);
		q->front = q->rear;
	}
	if(q->front->data)
		free(q->front->data);
		
	q->front->data = NULL;
	q->rear = q->front;
	q->size = 0;
	(q->front)->next = null;
	
	cmtx_leave(q->lock);
}

status cqueue_is_empty(cqueue *q)
{
	cmtx_enter(q->lock);
	int v;
	if (q->front == q->rear) 
		v = 1;
	else              
		v = 0;
	cmtx_leave(q->lock);
	return v;
}

elem cqueue_get(cqueue *q, int index)
{
	int i = 0;
	cmtx_enter(q->lock);
	cqueuenode *p = q->front;
	elem v = null;
	while(q->front != q->rear)
	{
		p = p->next;
		v = p->data;
		if(i == index)
			break;
		i++;
	}
	cmtx_leave(q->lock);
	return v;
}

elem cqueue_gethead(cqueue *q)
{
	cmtx_enter(q->lock);
	elem v;
	if (q->front == q->rear)
		v = null;
	else
		v = (q->front)->next->data;
	cmtx_leave(q->lock);
	return v;
}

status cqueue_enqueue(cqueue *q, elem e)
{
	cmtx_enter(q->lock);
	q->rear->next = (cqueuenode *)malloc(sizeof(cqueuenode));
	q->rear = q->rear->next;
	if (!q->rear){
		cmtx_leave(q->lock);
		return -1;
	}
	else
	{
		q->rear->data = e;
		q->rear->next = null;
		q->size += 1;
		cmtx_leave(q->lock);
		
		return 0;
	}
}

elem cqueue_dequeue(cqueue *q)
{
	cmtx_enter(q->lock);
	cqueuenode *p;
	elem e;
	if (q->front == q->rear)
	{
		e = null;
	}
	else 
	{
		p = (q->front)->next;
		(q->front)->next = p->next;
		e = p->data;
		if (q->rear == p)
			q->rear = q->front;
		free(p);
		q->size -= 1;
	}
	cmtx_leave(q->lock);
	return(e);
}

int cqueue_size(cqueue *q)
{
	cmtx_enter(q->lock);
	int size = q->size;
	cmtx_leave(q->lock);
	return size;
}

#ifdef __cplusplus
}
#endif

