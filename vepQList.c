/*-------------------------------------------------------
= Program Specification for QList.c
:toc:     

== Description
Provides a generic List structure (which grows).

== Usage
----
QList* my_list = q_list_new(5);

q_list_push(my_list,"going");
q_list_insert(my_list,0, "Why!!!");
q_list_insert(my_list,1, "is");
q_list_push(my_list,"on!!");
q_list_set(my_list, 0, "what");
printf("My list has %d\n", q_list_size(my_list));
q_list_foreach(my_list, print_list_item, NULL);

q_list_free(my_list);
----

== Functions
@spec
 +-----------------------------------------------------*/
/*-------------------------------------------------------
 | Include Files
 +-----------------------------------------------------*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "vepQList.h"

/*-------------------------------------------------------
 | Prototype
 +-----------------------------------------------------*/
static int list_check_size(QList* obj, int size) {
    if (obj == NULL) return -1;
    if (size > obj->alloc_size) {
        obj->alloc_size = obj->alloc_size << 1;
        if (size > obj->alloc_size) obj->alloc_size = size;
        obj->data = realloc(obj->data, sizeof(void*) * obj->alloc_size);
    }
    return obj->alloc_size;
}

static void inspect(QList* obj) {
    printf("QList{ addr: %08X", obj);
    printf(",  size: %d", obj->size);
    printf(",  alloc_size: %d", obj->alloc_size);
    printf(",  data: %08X", obj->data);
    printf("}\n");
}

/*-------------------------------------------------------
 | Global Functions
 +-----------------------------------------------------*/


/*-------------------------------------------------------
Description  ::    Creates a pointer to a QList structure

Parameters   :: 
init_size    : Initial allocated size +   
Return Value : QList structure
@specfun
 +-----------------------------------------------------*/
QList* q_list_new(int init_size) {
    QList* out = calloc(1,sizeof(QList));
    out->size = 0;
    out->alloc_size = init_size;
    out->data = malloc(sizeof(void*) * out->alloc_size);
    out->free_fn = NULL;
    return out;
};

/*-------------------------------------------------------
Description  ::  Create duplicate list

Parameters   :: 
obj          : QList to duplicate +
Return Value : New QList
@specfun
 +-----------------------------------------------------*/
QList* q_list_dup(QList* obj) {
    QList* out = q_list_new(obj->alloc_size);
    q_list_concat(out, obj);
    return out;
};

/*-------------------------------------------------------
Description :: Frees up the QList. Should not be used after the call.
               
Note: It does not free values inside list unless free_fn is set.

Parameters  ::   
obj : Pointer to QList
@specfun
 +-----------------------------------------------------*/
void q_list_free(QList* obj) {
    q_list_clear(obj);
    free(obj->data);
    free(obj);    
};

/*-------------------------------------------------------
Description :: Frees up the QList. Should not be used after the call.

Note: It does free values inside list with passed free_fn.

Parameters  ::   
obj : Pointer to QList +
free_fn : function to free objects

==== Usage
----
QList* my_list = q_list_new(5);

q_list_push(my_list, strdup("Going"));
q_list_insert(my_list,0, strdup("crazy"));
q_list_free_all(my_list, free);
----


@specfun
 +-----------------------------------------------------*/
void q_list_free_all(QList* obj, QFreeFn free_fn) {
    obj->free_fn = free_fn;
    q_list_free(obj);
};

/*-------------------------------------------------------
Description :: Concat another list at the end of this one.

Parameters  ::   
obj : Pointer to QList +
concat_list : function to free objects

@specfun
 +-----------------------------------------------------*/
void q_list_concat(QList* obj, QList* concat_list) {
    int i;
    if (concat_list == NULL) return;
    if (q_list_size(concat_list) == 0) return;
    list_check_size(obj, q_list_size(concat_list) + q_list_size(obj));
    for (i=0; i< q_list_size(concat_list); i++ ) {
        q_list_push(obj, (concat_list->data)[i]);
    }
};

/*-------------------------------------------------------
Description :: Pushes item to end of list

Parameters  ::   
obj : Pointer to QList +
item : function to free objects +
Returns : Size of list

@specfun
 +-----------------------------------------------------*/
int q_list_push(QList* obj, void* item) {
    list_check_size(obj, obj->size+1);
    (obj->data)[obj->size] = item;
    (obj->size)++;
    return obj->size;
};

/*-------------------------------------------------------
Description :: Inserts items into list

Parameters  ::   
obj : Pointer to QList +
idx : Insertion point (Ignored if negative) +
item : Item to insert 

@specfun
 +-----------------------------------------------------*/
void q_list_insert(QList* obj, int idx, void* item) {
    int i, size_to_move;
    if (idx < 0) return; // DO NOTHING

    list_check_size(obj, obj->size+1);
    if (idx > q_list_size(obj)) idx = q_list_size(obj);
    size_to_move = (obj->size - idx);

    memmove(&(obj->data[idx+1]), &(obj->data[idx]), sizeof(void*) * size_to_move);
    obj->data[idx] = item;
    obj->size++;
};

/*-------------------------------------------------------
Description :: Inserts items into list

Parameters  ::   
obj : Pointer to QList +
idx : Insertion point (Ignored if negative) +
Returns: Item deleted (For cleanup)

@specfun
 +-----------------------------------------------------*/
void* q_list_delete(QList* obj, int idx) {
    void* out = NULL;
    int size_to_move = 0;
    if (idx < 0 || idx >= q_list_size(obj)) return out;
    size_to_move = (q_list_size(obj) - idx - 1);
    out = q_list_get(obj, idx);
    memmove(&(obj->data[idx]), &(obj->data[idx+1]), sizeof(void*) * size_to_move);
    obj->size--;
    return out;
};

/*-------------------------------------------------------
Description :: Sets item in particular point

Parameters  ::   
obj : Pointer to QList +
idx : Insertion point (Ignored if negative) +
item : Item to insert into list +
Returns: Old item in list (for cleanup) (NULL if none)

@specfun
 +-----------------------------------------------------*/
void* q_list_set(QList* obj, int idx, void* item) {
    void* out;
    if (obj == NULL) return NULL;
    if (idx < 0 || idx > obj->size-1) return NULL;
    out = (obj->data)[idx];
    (obj->data)[idx] = item;

    if ((out != NULL) && (obj->free_fn != NULL)) {
        obj->free_fn(out);
        out = NULL;
    }
    return out;
}; 

/*-------------------------------------------------------
Description :: Size of list

Parameters  ::   
obj : Pointer to QList +
Returns: Size of list

@specfun
 +-----------------------------------------------------*/
int q_list_size(QList* obj) { 
    return obj->size;
};

/*-------------------------------------------------------
Description :: Gets items from list using index

Parameters  ::   
obj : Pointer to QList +
idx : Index of items +
Returns: Item (NULL if not found)

@specfun
 +-----------------------------------------------------*/
void* q_list_get(QList* obj, int idx) {
    if (obj == NULL) return NULL;
    if (idx < 0) return NULL;
    if (idx > obj->size-1) return NULL;
    return (obj->data)[idx];
};

/*-------------------------------------------------------
Description :: Iterated through list with function

Parameters  ::   
obj : Pointer to QList +
fn :  Function +
user_data : Object to store staored for function 

@specfun
 +-----------------------------------------------------*/
void q_list_foreach(QList* obj, QFn fn, void* user_data) {
    if (obj == NULL) return;
    int i=0;
    for (i=0; i< obj->size; i++ ) {
        (*fn)((obj->data)[i], user_data);
    }
};

/*-------------------------------------------------------
Description :: Clears the list of all items

NOTE: Items not freed unless `free_fn` is set.

Parameters  ::   
obj : Pointer to QList 

@specfun
 +-----------------------------------------------------*/
void q_list_clear(QList* obj) {
    if (obj == NULL) return;
    if (obj->free_fn != NULL) {
        int i=0;
        for (i=0; i< obj->size; i++ ) {
            (obj->free_fn)((obj->data)[i]);
        }
    }
    obj->size = 0;
};

/*-------------------------------------------------------
Description :: Sets the `free_fn` to clean up objects.

NOTE: Use only if list is taking ownership of items.

Parameters  ::   
obj : Pointer to QList +
free_fn : Function to free items in list.

@specfun
 +-----------------------------------------------------*/
void q_list_set_free_fn(QList* obj, QFreeFn free_fn) {
    if (obj == NULL) return;
    obj->free_fn = free_fn;
}

/*-------------------------------------------------------
Description :: Sorts the list (using qsort)

Parameters  ::   
obj : Pointer to QList +
compare_fn : Function to compare items in list.

@specfun
+-----------------------------------------------------*/
void q_list_sort(QList* obj, QCompareFn compare_fn) {
    if (obj == NULL) return;
    qsort(obj->data, obj->size, sizeof(void*), compare_fn);
}

/*-------------------------------------------------------
Description :: Pops items from end of list

NOTE: Consider non-destructive as passing ownership of object
to caller.

Parameters  ::   
obj : Pointer to QList +
Return : Last item in list (NULL if none)

@specfun
+-----------------------------------------------------*/
void* q_list_pop(QList* obj) {
    if (obj == NULL) return NULL;
    if (obj->size == 0) return NULL;
    void* out = (obj->data)[obj->size-1];
    (obj->size)--;
    return out;
};

/*-------------------------------------------------------
Description :: Shift off the first item of list

NOTE: Consider non-destructive as passing ownership of object
to caller.

Parameters  ::   
obj : Pointer to QList +
Return : First item in list (NULL if none)

@specfun
+-----------------------------------------------------*/
void* q_list_shift(QList* obj) {
    int i;
    if (obj == NULL) return NULL;
    if (obj->size == 0) return NULL;
    void* out = (obj->data)[0];

    for (i=1; i < obj->size; i++) {
        (obj->data)[i-1] = (obj->data)[i];
    }
    obj->size--;
    return 0;
};

