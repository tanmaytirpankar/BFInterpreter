#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Function to open file and read content
char *read_file(const char *file_name) {
  FILE *file = fopen(file_name, "r");
  if (!file) {
    fprintf(stderr, "Error opening file: %s\n", file_name);
    exit(1);
  }

  // Move file pointer to the end to determine file size
  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  // Allocate memory for the file contents
  char *content = (char *)malloc(length + 1);
  if (!content) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }

  // Read the file into the buffer
  fread(content, 1, length, file);
  content[length] = '\0'; // Null-terminate the string

  fclose(file);
  return content;
}

// Function to filter and return only '+-<>,.[]' characters from the text
char *filter_text(const char *text) {
  char *filtered = (char *)malloc(strlen(text) + 1);
  if (!filtered) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }

  int j = 0;
  for (int i = 0; text[i]; i++) {
    if (strchr("+-<>,.[]", text[i])) {
      filtered[j++] = text[i];
    }
  }
  filtered[j] = '\0';
  return filtered;
}

// Preprocessing to match loops and store their positions
void preprocess_loops(const char *text, int *loop_map) {
  int loop_stack[1024]; // Stack for storing loop start positions
  int stack_ptr = -1;

  for (int i = 0; text[i]; i++) {
    if (text[i] == '[') {
      loop_stack[++stack_ptr] = i; // Push position of '[' onto the stack
    } else if (text[i] == ']') {
      if (stack_ptr < 0) {
        fprintf(stderr, "Mismatched ']' at position %d\n", i);
        exit(1);
      }
      int open_pos = loop_stack[stack_ptr--]; // Pop the matching '[' position
      loop_map[open_pos] = i; // Store matching positions
      loop_map[i] = open_pos;
    }
  }

  if (stack_ptr >= 0) {
    fprintf(stderr, "Mismatched '[' at position %d\n", loop_stack[stack_ptr]);
    exit(1);
  }
}

// Function to parse and execute the brainfuck-like program
void parse_program(const char *text, const int *loop_map) {
  int ip = 0;  // instruction pointer
  int ptr = 0; // memory pointer

  // Initialize tape with 30,000 cells to avoid frequent resizing
  int tape[30000] = {0};

  // Process each character in the input program
  while (text[ip]) {
    switch (text[ip]) {
      case '>':  // move pointer right
        ptr++;
        break;

      case '<':  // move pointer left
        if (ptr > 0) ptr--;
        break;

      case '+':  // increment the value at current cell
        tape[ptr] = (tape[ptr] + 1) % 256;
        break;

      case '-':  // decrement the value at current cell
        tape[ptr] = (tape[ptr] - 1 + 256) % 256;
        break;

      case '.':  // output the value at current cell as character
        putchar(tape[ptr]);
        break;

      case ',':  // read a character from input into the current cell
        tape[ptr] = getchar();
        break;

      case '[':  // begin loop
        if (tape[ptr] == 0) {
          ip = loop_map[ip];  // jump to the matching ']'
        }
        break;

      case ']':  // end loop
        if (tape[ptr] != 0) {
          ip = loop_map[ip];  // jump back to the matching '['
        }
        break;

      default:
        // Ignore any other characters
        break;
    }
    ip++; // move to the next instruction
  }
}

// Main function to execute the program
int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
    return 1;
  }

  // Record start time
  clock_t start = clock();

  // Read and filter the brainfuck program
  char *text = read_file(argv[1]);
  char *filtered_text = filter_text(text);
  free(text); // Free the original unfiltered content

  printf("Program Length: %lu\n", strlen(filtered_text));

  // Allocate memory for loop positions
  int *loop_map = (int *)malloc(strlen(filtered_text) * sizeof(int));
  if (!loop_map) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }

  // Preprocess loop start and end positions
  preprocess_loops(filtered_text, loop_map);

  // Execute the brainfuck-like program
  parse_program(filtered_text, loop_map);

  // Record end time
  clock_t end = clock();
  double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
  printf("\nTime taken: %.6f seconds\n", elapsed);

  // Free allocated memory
  free(filtered_text);
  free(loop_map);

  return 0;
}
