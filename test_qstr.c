// Test the QStr functionality
#include "vepQStr.h"
#include <stdbool.h>
#include <stdio.h>
#include <strings.h>

static QStr *file_gets(FILE *file) {
  char inp[100];
  QStr *s = qstr_new(100, "");

  while (fgets(inp, 100, file) != NULL) {
    qstr_cat(s, inp);
    if ((s->data)[s->length - 1] == '\n') {
      break;
    }
  }
  return s;
}

void test_qstr() {
  QStr *s = qstr_new(10, "Hello");

  for (int i = 0; i < 3; i++)
    qstr_cat(s, " world");

  printf("str='%s' (%ld)\n", s->data, s->length);
  qstr_cat(s, " and end....     \t\n\n     ");
  qstr_trimr(s);
  printf("str='%s' (%ld)\n", s->data, s->length);

  qstr_trunc(s, 4);
  printf("str='%s' (%ld)\n", s->data, s->length);

  qstr_trunc(s, 0);
  qstr_cat(s, "     \t\n\n     ");
  qstr_trimr(s);
  printf("str='%s' (%ld)\n", s->data, s->length);
}

void test_read_stdin() {
  QStr *str;
  while ((str = file_gets(stdin))->length > 0) {
    printf("'%s'", str->data);
  }
  printf("\nCiao\n");
}

int main(int argc, char const *argv[]) {
  // test_read_stdin();
  test_qstr();
  return 0;
}
