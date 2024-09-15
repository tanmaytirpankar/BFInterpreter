#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Define the size of the tape
#define TAPE_SIZE 30000

// Declare function pointers for each BF command
typedef void (*bf_command_fn)(void);
unsigned char tape[TAPE_SIZE];
unsigned int ptr = 0;

// Instruction pointer and program text
const char *program;
unsigned int ip = 0;

// Function declarations for threading
void bf_increment_ptr(void);
void bf_decrement_ptr(void);
void bf_increment_value(void);
void bf_decrement_value(void);
void bf_output_value(void);
void bf_input_value(void);
void bf_jump_forward(void);
void bf_jump_backward(void);

// Command array: Maps characters to corresponding functions
bf_command_fn command_map[256];

void bf_increment_ptr(void) {
  ptr++;
  if (ptr >= TAPE_SIZE) {
    ptr = 0;  // Wrap pointer around
  }
}

void bf_decrement_ptr(void) {
  if (ptr == 0) {
    ptr = TAPE_SIZE - 1;  // Wrap around to end of tape
  } else {
    ptr--;
  }
}

void bf_increment_value(void) {
  tape[ptr]++;
}

void bf_decrement_value(void) {
  tape[ptr]--;
}

void bf_output_value(void) {
  putchar(tape[ptr]);
}

void bf_input_value(void) {
  tape[ptr] = getchar();
}

void bf_jump_forward(void) {
  if (tape[ptr] == 0) {
    int loop = 1;
    while (loop > 0) {
      ip++;
      if (program[ip] == '[') loop++;
      if (program[ip] == ']') loop--;
    }
  }
}

void bf_jump_backward(void) {
  if (tape[ptr] != 0) {
    int loop = 1;
    while (loop > 0) {
      ip--;
      if (program[ip] == '[') loop--;
      if (program[ip] == ']') loop++;
    }
  }
}

// Set up the command map to link characters to their respective functions
void setup_command_map() {
  command_map['>'] = bf_increment_ptr;
  command_map['<'] = bf_decrement_ptr;
  command_map['+'] = bf_increment_value;
  command_map['-'] = bf_decrement_value;
  command_map['.'] = bf_output_value;
  command_map[','] = bf_input_value;
  command_map['['] = bf_jump_forward;
  command_map[']'] = bf_jump_backward;
}

// Main interpreter function
void interpret(const char *bf_program) {
  program = bf_program;

  // Loop over the Brainfuck program
  while (program[ip] != '\0') {
    bf_command_fn command = command_map[(unsigned char)program[ip]];

    // If a valid command exists, execute it
    if (command) {
      command();
    }

    ip++;  // Move to the next instruction
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <brainfuck_program>\n", argv[0]);
    return 1;
  }

  // Record start time
  clock_t start = clock();

  // Load the Brainfuck program from a file
  FILE *file = fopen(argv[1], "r");
  if (!file) {
    fprintf(stderr, "Error: could not open file %s\n", argv[1]);
    return 1;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *program = malloc(length + 1);
  if (!program) {
    fprintf(stderr, "Memory allocation error\n");
    return 1;
  }

  fread(program, 1, length, file);
  program[length] = '\0';  // Null-terminate the program
  fclose(file);

  // Set up the command map
  setup_command_map();

  // Initialize tape and pointer
  for (int i = 0; i < TAPE_SIZE; i++) {
    tape[i] = 0;
  }
  ptr = 0;
  ip = 0;

  // Run the Brainfuck interpreter
  interpret(program);

  free(program);
  // Record end time
  clock_t end = clock();
  double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
  printf("\nTime taken: %.6f seconds\n", elapsed);
  return 0;
}
