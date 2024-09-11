#ifndef QTYPE_H
#define QTYPE_H

typedef int (*QCompareFn)(const void* a, const void* b);
typedef int (*QCompareDataFn)(void* a, void* b, void* data);
typedef void (*QFreeFn)(void* a);
typedef void (*QFn)(void* data, void* user_data);

#endif // QTYPE_H