#ifndef C_MAP_H
#define C_MAP_H
#include "lock_utils.h"

#define null 0 

typedef int status;
typedef void* elem;

typedef union
{
	int i_key;
	char p_key[64];
}cmap_key;

typedef struct cmapnode_s{
	cmap_key key;
	elem data;
	struct cmapnode_s *next;
}cmapnode;
typedef struct
{
	cmapnode *front;
	cmapnode *rear;
	int size;
	CMtx lock;
}cmap;

#ifdef __cplusplus
extern "C"{
#endif
void cmap_init(cmap *p);
//将释放节点数据内存
void cmap_clear(cmap *q);
//将释放节点数据内存
status cmap_destory(cmap *q);
status cmap_is_empty(cmap *q);
status cmap_ikey_insert(cmap *q, int key, elem e);
//将释放节点数据内存
status cmap_ikey_erase(cmap *q, int key);
elem cmap_ikey_find(cmap *q, int key);
status cmap_pkey_insert(cmap *q, char* key, elem e);
//将释放节点数据内存
status cmap_pkey_erase(cmap *q, char* key);
elem cmap_pkey_find(cmap *q, char* key);
cmapnode* cmap_index_get(cmap *q, int index);
int cmap_size(cmap *q);
#ifdef __cplusplus
}
#endif

#endif