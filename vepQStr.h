#ifndef QSTR_LIB_H
#define QSTR_LIB_H

#include <stddef.h>

typedef struct {
  char *data;
  size_t length;
  size_t allocated;
} QStr;

// Initialize a new QStr
QStr *qstr_new(const size_t initialCap, const char *initial_data);

// Free the memory used by a QStr
void qstr_free(QStr *str);

// Get the length of the QStr
size_t qstr_length(const QStr *str);

// Get the character at a specific index
char qstr_char_at(const QStr *str, size_t index);

int qstr_cat(QStr *s, const char *data);

// Concatenate two QStrs
QStr *qstr_concat(const QStr *str1, const QStr *str2);

// Copy a QStr
QStr *qstr_dup(const QStr *str);

// Substring of a QStr
QStr *qstr_substring(const QStr *str, size_t start, size_t end);

// Right trim QStr
void qstr_trimr(QStr *str);

void qstr_trunc(QStr *str, size_t idx);

// Print a QStr
void qstr_print(const QStr *str);

#endif // QSTR_LIB_H
