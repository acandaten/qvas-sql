#include "vepQStr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16

static int qstr_check_allocated(QStr *obj, int size) {
  if (obj == NULL)
    return -1;
  if (size > obj->allocated) {
    obj->allocated = obj->allocated << 1;
    if (size > obj->allocated)
      obj->allocated = size;
    obj->data = realloc(obj->data, sizeof(void *) * obj->allocated);
  }
  return obj->allocated;
}

QStr *qstr_new(const size_t initialCap, const char *initial_data) {
  QStr *str = (QStr *)malloc(sizeof(QStr));
  if (str == NULL)
    return NULL;

  size_t len = strlen(initial_data);
  size_t capacity = (len + 1 > initialCap) ? len + 1 : initialCap;

  str->data = (char *)malloc(capacity);
  if (str->data == NULL) {
    free(str);
    return NULL;
  }

  strcpy(str->data, initial_data);
  str->length = len;
  str->allocated = capacity;

  return str;
}

void qstr_free(QStr *str) {
  if (str != NULL) {
    free(str->data);
    free(str);
  }
}

size_t qstr_length(const QStr *str) { return str->length; }

char qstr_char_at(const QStr *str, size_t index) {
  if (index >= str->length)
    return '\0';
  return str->data[index];
}

int qstr_cat(QStr *s, const char *data) {
  if (s == NULL)
    return -3;

  size_t new_len = strlen(data);
  qstr_check_allocated(s, new_len + s->length + 1);
  strcat(s->data + s->length, data);
  (s->length) += new_len;
  return 0;
}

QStr *qstr_concat(const QStr *str1, const QStr *str2) {
  size_t new_len = str1->length + str2->length;
  QStr *result = (QStr *)malloc(sizeof(QStr));
  if (result == NULL)
    return NULL;

  result->data = (char *)malloc(new_len + 1);
  if (result->data == NULL) {
    free(result);
    return NULL;
  }

  strcpy(result->data, str1->data);
  strcat(result->data, str2->data);
  result->length = new_len;
  result->allocated = new_len + 1;

  return result;
}

QStr *qstr_dup(const QStr *str) { return qstr_new(0, str->data); }

QStr *qstr_substring(const QStr *str, size_t start, size_t end) {
  if (start >= str->length || end > str->length || start > end) {
    return qstr_new(0, "");
  }

  size_t sub_len = end - start;
  char *sub_data = (char *)malloc(sub_len + 1);
  if (sub_data == NULL)
    return NULL;

  strncpy(sub_data, str->data + start, sub_len);
  sub_data[sub_len] = '\0';

  QStr *result = qstr_new(0, sub_data);
  free(sub_data);

  return result;
}

void qstr_print(const QStr *str) { printf("%s", str->data); }
