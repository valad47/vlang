#define NOB_IMPLEMENTATION
#include "nob.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define STACK_BASE 8

typedef enum Flags {
  ASM_ONLY = 0b1,
} Flags;

typedef struct Stack {
  void **data_array;
  int sp;
  int stack_size;
} Stack;

void stack_push(Stack *stack, void* data) {
  stack->sp++;
  if (stack->sp >= stack->stack_size) {
    stack->stack_size += STACK_BASE;
    stack->data_array = realloc(stack->data_array, sizeof(void*) * stack->stack_size);
  }
  stack->data_array[stack->sp-1] = data;
}

void *stack_pop(Stack *stack) {
  if (stack == NULL) {
    fprintf(stderr, "stack pointer is NULL\n");
    exit(1);
  }
  if (stack->sp == 0) {
    return NULL;
  }
  stack->sp--;
  void *ret_val = stack->data_array[stack->sp];
  stack->data_array[stack->sp] = NULL;
  return ret_val;
}
  
int main(int argc, char **argv) {
  Flags compile_flags = 0;
  Stack files = {0};
  for (int i = 1; i < argc; i++) {
    char *arg = argv[i];
    if (arg[0] == '-') {
      for (char *flag = arg+1; *flag != '\0'; flag++) {
	switch (*flag) {
	case 'S':
	  compile_flags |= ASM_ONLY;
	  break;
	default:
	  fprintf(stderr, "No such flag \"%c\"\n", *flag);
	  exit(1);
	}
      }
      continue;
    }
    if(nob_file_exists(arg)) {
      stack_push(&files, arg);
    } else {
      fprintf(stderr, "File \"%s\" does not exist.\n", arg);
      exit(1);
    }
  }
  if(files.sp == 0) {
    fprintf(stderr, "No files were specified.\n");
    exit(1);
  }
}
