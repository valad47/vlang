#define NOB_IMPLEMENTATION
#include "nob.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#define STACK_BASE 8

typedef enum Flags {
  ASM_ONLY,
  FLAGS_NUMBER
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

typedef enum TOKEN_TYPE {
  INT,
  INT_PTR,
  CHAR,
  CHAR_PTR,
  VOID,
  VOID_PTR,
  EQUAL,
  FUNC_DEF,
  SCOPE_START,
  SCOPE_END
} TOKEN_TYPE;

typedef struct TOKEN {
  void *data;
  TOKEN_TYPE type;
  struct TOKEN *next;
} TOKEN;

const char program_entry[] = {
  "    .globl _start\n"
  "_start:\n"
  "    push %rbp\n"
  "    mov %rsp, %rbp\n"
  "    call main\n"
  "    mov %rax, %rdi\n"
  "    mov $60, %rax\n"
  "    syscall\n\n"
};

const char *gen_asm(const char *filename) {
  Nob_String_Builder file_src = {0};
  if (!nob_read_entire_file(filename, &file_src)) {
    fprintf(stderr, "[ERROR] Failed to read file \"%s\"", filename);
    exit(1);
  }

  FILE *f = fopen("out.s", "w+");
  
  fwrite(program_entry, 1, strlen(program_entry), f);
  
  for(char *word = strtok(file_src.items, " "); word != NULL; word = strtok(NULL, " ")) {
    if(strcmp("function", word) == 0) {
      word = strtok(NULL, "(");
      char set_glob[] = {
	"    .globl "
      };
      fwrite(set_glob, 1, strlen(set_glob), f);
      fwrite(word, 1, strlen(word), f);
      fwrite("\n", 1, 1, f);
      fwrite(word, 1, strlen(word), f);
      char stack_start[] = {
	":\n"
	"    push %rbp\n"
	"    mov %rsp, %rbp\n"
      };

      fwrite(stack_start, 1, strlen(stack_start), f);
    } else if(strcmp("return", word) == 0) {
	word = strtok(NULL, ";");
	char ret[] = {
	  "    mov $"
	};
	char ret2[] = {
	  ", %rax\n"
	  "    pop %rbp\n"
	  "    ret\n"
	};
	
	fwrite(ret, 1, strlen(ret), f);
	fwrite(word, 1, strlen(word), f);
	fwrite(ret2, 1, strlen(ret2), f);
      }
    printf("%s\n", word);
  }
  fclose(f);

  return "out.s";
}

int main(int argc, char **argv) {
  void *compile_flags[FLAGS_NUMBER] = {0};
  Stack files = {0};
  for (int i = 1; i < argc; i++) {
    char *arg = argv[i];
    if (arg[0] == '-') {
      for (char *flag = arg+1; *flag != '\0'; flag++) {
	switch (*flag) {
	case 'S':
	  compile_flags[ASM_ONLY] = (void*)1;
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
  gen_asm(stack_pop(&files));
}
