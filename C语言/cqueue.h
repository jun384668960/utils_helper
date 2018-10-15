#ifndef C_QUEUE_H
#define C_QUEUE_H
#include "lock_utils.h"

#define null 0 

typedef int status;
typedef void* elem;

typedef struct cqueuenode_s{
	elem data;
	struct cqueuenode_s *next;
}cqueuenode;
typedef struct
{
	cqueuenode *front;
	cqueuenode *rear;
	int size;
	CMtx lock;
}cqueue;

#ifdef __cplusplus
extern "C"{
#endif

void cqueue_init(cqueue *p);
//���ͷŽڵ������ڴ�
void cqueue_clear(cqueue *q);
//���ͷŽڵ������ڴ�
status cqueue_destory(cqueue *q);
status cqueue_is_empty(cqueue *q);
//�ⲿmallco�ڵ�ṹ
status cqueue_enqueue(cqueue *q, elem e);
//��Ҫ�ֶ�ɾ���ڵ�
elem cqueue_dequeue(cqueue *q);
elem cqueue_gethead(cqueue *q);
int cqueue_size(cqueue *q);

#ifdef __cplusplus
}
#endif

#endif
