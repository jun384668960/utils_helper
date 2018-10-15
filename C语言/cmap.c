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
	p->front->key.i_key = 0xffffffff;
	memset(p->front->key.p_key, 0, 64);
	(p->front)->next = null;

}
void cmap_clear(cmap *q)
{
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
	q->front->key.i_key = 0xffffffff;
	memset(q->front->key.p_key, 0, 64);
	q->size = 0;
}
status cmap_destory(cmap *q)
{
	while (q->front)
	{
		q->rear = q->front->next;
		if(q->front->data)
			free(q->front->data);
		free(q->front);
		q->front = q->rear;
	}
	q->size = 0;

	return 0;
}
status cmap_is_empty(cmap *q)
{
	int v;
	if (q->front == q->rear) 
		v = 1;
	else              
		v = 0;
	return v;
}

status cmap_ikey_insert(cmap *q, int key, elem e)
{
	if(cmap_ikey_find(q, key) != null) return -2;
		
	q->rear->next = (cmapnode *)malloc(sizeof(cmapnode));
	q->rear = q->rear->next;
	if (!q->rear) 
		return -1;
	q->rear->data = e;
	q->rear->key.i_key = key;
	q->rear->next = null;
	q->size += 1;

	return 0;
}

elem cmap_ikey_find(cmap *q, int key)
{
	elem v = null;
	cmapnode* p = q->front->next;
	while (p)
	{	
		if(p->key.i_key == key)
		{
			v = p->data;
			break;
		}
		p = p->next;
	}

	return v;
}

status cmap_ikey_erase(cmap *q, int key)
{
	cmapnode* t = q->front;
	cmapnode* p = q->front->next;
	while (p)
	{	
		if(p->key.i_key == key)
		{
			t->next = p->next;
			if(q->rear == p)
			{
				q->rear = t;
			}
			
			free(p);

			q->size -= 1;
			return 0;
		}
		t = p;
		p = p->next;
	}

	return -1;
}

status cmap_pkey_insert(cmap *q, char* key, elem e)
{
	if(cmap_pkey_find(q, key) != null) return -2;

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
		strcpy(q->rear->key.p_key, key);
		q->rear->next = null;
		q->size += 1;

		cmtx_leave(q->lock);
		
		return 0;
	}

}

//将释放节点数据内存
status cmap_pkey_erase(cmap *q, char* key)
{
	cmtx_enter(q->lock);
	cmapnode* t = q->front;
	cmapnode* p = q->front->next;
	while (p)
	{	
		if(strcmp(p->key.p_key, key) == 0)
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

elem cmap_pkey_find(cmap *q, char* key)
{
	cmtx_enter(q->lock);
	elem v = null;
	cmapnode* p = q->front->next;
	while (p)
	{	
		if(strcmp(p->key.p_key, key) == 0)
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

	return v;
}

int cmap_size(cmap *q)
{
	return q->size;
}

#ifdef __cplusplus
}
#endif

