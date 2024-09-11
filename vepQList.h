
#ifndef QLIST_H_   /* Include guard */
#define QLIST_H_
#include "vepQType.h"

typedef struct _list {
    int size;
    int alloc_size;
    void** data;
    QFreeFn free_fn ;
} QList;

QList* q_list_new(int init_size);
QList* q_list_dup(QList* obj);
void q_list_free(QList* obj);
void q_list_free_all(QList* obj, QFreeFn free_fn);

int q_list_size(QList* obj);
void q_list_concat(QList* obj, QList* concat_list);
int q_list_push(QList* obj, void* item);
void* q_list_pop(QList* obj);
void* q_list_shift(QList* obj);
void q_list_insert(QList* obj, int idx, void* item);

void* q_list_get(QList* obj, int idx);
void* q_list_set(QList* obj, int idx, void* item); 
void* q_list_delete(QList* obj, int idx);
void q_list_foreach(QList* obj, QFn fn, void* user_data);

void q_list_clear(QList* obj);
void q_list_set_free_fn(QList* obj, QFreeFn free_fn);
void q_list_sort(QList* obj, QCompareFn compare_fn);


#endif // QLIST_H_
