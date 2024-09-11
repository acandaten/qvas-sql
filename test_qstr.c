// Test the QStr functionality
#include "vepQStr.h"
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
  QStr *s = qstr_new(100, "Hello");

  for (int i = 0; i < 1000; i++)
    qstr_cat(s, " world\n");

  printf("str='%s' (%ld)\n", s->data, s->allocated);
}

void test_read_stdin() {
  QStr *str;
  while ((str = file_gets(stdin))->length > 0) {
    printf("'%s'", str->data);
  }
  printf("\nCiao\n");
}

int main(int argc, char const *argv[]) {
  test_read_stdin();
  return 0;
}
