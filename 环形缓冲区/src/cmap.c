#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmap.h"

#ifdef __cplusplus
extern "C"{
#endif


void cmap_init(cmap *p)
{
	p->front = (cmapnode *)malloc(sizeof(cmapnode));
	p->front->data = NULL;
	p->rear = p->front;
	p->size = 0;
	memset(p->front->key, 0, 64);
	p->front->next = null;
	p->lock = cmtx_create();
}
void cmap_clear(cmap *q)
{
	cmtx_enter(q->lock);
	while (cmap_is_empty(q) != 1)
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
	memset(q->front->key, 0, 64);
	q->size = 0;
	
	cmtx_leave(q->lock);
}
status cmap_destory(cmap *q)
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
status cmap_is_empty(cmap *q)
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

status cmap_insert(cmap *q, char* key, elem e)
{
	if(cmap_find(q, key) != null) return -2;

	cmtx_enter(q->lock);
	q->rear->next = (cmapnode *)malloc(sizeof(cmapnode));
	q->rear = q->rear->next;
	if (!q->rear)
	{
		cmtx_leave(q->lock);
		return -1;
	}
	else
	{
		q->rear->data = e;
		strcpy(q->rear->key, key);
		q->rear->next = null;
		q->size += 1;

		cmtx_leave(q->lock);
		
		return 0;
	}
	
}

elem cmap_find(cmap *q, char* key)
{
	cmtx_enter(q->lock);
	elem v = null;
	cmapnode* p = q->front->next;
	while (p)
	{	
		if(strcmp(p->key, key) == 0)
		{
			v = p->data;
			break;
		}
		p = p->next;
	}
	cmtx_leave(q->lock);
	
	return v;
}

cmapnode* cmap_index_get(cmap *q, int index)
{
	cmtx_enter(q->lock);
	cmapnode* v = null;
	int idx = 0;
	cmapnode* p = q->front->next;
	while (p)
	{	
		if(idx == index)
		{
			v = p;
			break;
		}
		p = p->next;
		idx++;
	}
	cmtx_leave(q->lock);

	return v;
}


status cmap_erase(cmap *q, char* key)
{
	cmtx_enter(q->lock);
	cmapnode* t = q->front;
	cmapnode* p = q->front->next;
	while (p)
	{	
		if(strcmp(p->key, key) == 0)
		{
			t->next = p->next;
			if(q->rear == p)
			{
				q->rear = t;
			}
			
			free(p);
			q->size -= 1;
			
			cmtx_leave(q->lock);
			return 0;
		}
		t = p;
		p = p->next;
	}
	cmtx_leave(q->lock);
	
	return -1;
}
int cmap_size(cmap *q)
{
	return q->size;
}

#ifdef __cplusplus
}
#endif

