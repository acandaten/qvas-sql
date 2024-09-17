#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int opt;
  char *input_file = NULL;
  char *output_file = NULL;
  int verbose = 0;
  char opts[100] = "";

  while ((opt = getopt(argc, argv, "i:o:vg")) != -1) {
    switch (opt) {
    case 'i':
      input_file = optarg;
      break;
    case 'o':
      output_file = optarg;
      break;
    case 'v':
      verbose = 1;
      strcat(opts, "v");
      break;
    case 'g':
      strcat(opts, "g");
      break;
    default:
      fprintf(stderr, "Usage: %s [-i input_file] [-o output_file] [-v] [-g]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  // Print parsed options
  printf("Input file: %s\n", input_file ? input_file : "Not specified");
  printf("Output file: %s\n", output_file ? output_file : "Not specified");
  printf("Verbose mode: %s\n", verbose ? "On" : "Off");
  printf("opts : %s\n", opts);

  // You can access non-option arguments using argv[optind] to argv[argc-1]
  for (int i = optind; i < argc; i++) {
    printf("Non-option argument: %s\n", argv[i]);
  }

  return 0;
}
