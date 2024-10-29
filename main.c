#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

typedef enum Flags {
  ASM_ONLY = 0b1,
} Flags;

int main(int argc, char **argv) {
  Flags compile_flags = 0;
  for (int i = 1; i < argc; i++) {
    char *arg = argv[i];
    if (arg[0] == '-')
      for (char *flag = arg+1; *flag != '\0'; flag++) {
	switch (*flag) {
	case 'S':
	  compile_flags |= ASM_ONLY;
	  break;
	default:
	  fprintf(stderr, "No such flag \"%c\"", *flag);
	  exit(1);
	}
      }
  }
}
