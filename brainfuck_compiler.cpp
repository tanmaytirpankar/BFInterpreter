#include <iostream>
#include <fstream>
#include <stack>

using namespace std;

// Function to open file and read content
string read_file(const string &file_name) {
  ifstream file(file_name);
  if (!file.is_open()) {
    cerr << "Error opening file: " << file_name << endl;
    exit(1);
  }
  string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
  file.close();
  return content;
}

// Function to filter and return only '+-<>,.[]' characters from the text
string filter_text(const string &text) {
  string filtered;
  filtered.reserve(text.size());
  for (char ch : text) {
    if (ch == '+' || ch == '-' || ch == '<' || ch == '>' || ch == ',' || ch == '.' || ch == '[' || ch == ']') {
      filtered += ch;
    }
  }
  return filtered;
}

// Compiles brainfuck to x86_64 assembly
void compile_program(const string &text, string output_file) {
  if(output_file == "") {
    output_file = "a.s";
  }
  ofstream asm_file(output_file);

  // Assembly header
  // Memory allocation section - Block start by symbol
  asm_file << "section .bss\n";

  // .lcomm directive allocates memory
  asm_file << "   tape resb 30000\n";
  asm_file << "section .text\n";
  asm_file << "global _start\n";
  asm_file << "_start:\n";
  asm_file << "   mov rsi, tape ; Initialize data pointer\n";

  stack<int> loop_stack;

  // To name loops
  int loop_count=0;

  for(char bf_op : text) {
    switch(bf_op) {
      case '>':
        // Increment the data pointer
        asm_file << "   inc rsi" << endl;
        break;
      case '<':
        // Decrement the data pointer
        asm_file << "   dec rsi" << endl;
        break;
      case '+':
        // Increment the byte the data pointer points to
        asm_file << "   inc byte [rsi]" << endl;
        break;
      case '-':
        // Increment the byte the data pointer points to
        asm_file << "   dec byte [rsi]" << endl;
        break;
      case '.':
        // Output the byte at the data pointer
        // Set up the syscall for write (1) at rax
        asm_file << "   mov rax, 1" << endl;
        // Set up the file descriptor (stdout) at rdi (First argument)
        asm_file << "   mov rdi, 1" << endl;
        // rsi is the second argument which points to the correct cell in tape, so we don't need to set it up
        // Set up the number of bytes to write at rdx
        asm_file << "   mov rdx, 1" << endl;
        // Invoke the system call
        asm_file << "   syscall" << endl;
        break;
      case ',':
        // Read a byte from stdin
        // Set up the syscall for read (0) at rax
        asm_file << "   mov rax, 0" << endl;
        // Set up the file descriptor (stdin) at rdi (First argument)
        asm_file << "   mov rdi, 0" << endl;
        // rsi is the second argument which points to the correct cell in tape, so we don't need to set it up
        // Set up the number of bytes to read at rdx
        asm_file << "   mov rdx, 1" << endl;
        // Invoke the system call
        asm_file << "   syscall" << endl;
        break;
      case '[':
        // Start of a loop
        loop_stack.push(loop_count);
        // Label the loop
        asm_file << "loop_" << loop_count << ":" << endl;
        // Compare the byte at the data pointer to 0
        asm_file << "   cmp byte [rsi], 0" << endl;
        // Jump to the end of the loop if the byte is 0
        asm_file << "   je loop_" << loop_count << endl;
        loop_count++;
        break;
      case ']':
        // End of a loop
        if (loop_stack.empty()) {
          cerr << "Unmatched ']' exiting" << endl;
          return;
        }
        loop_count = loop_stack.top();
        loop_stack.pop();
        // Compare the byte at the data pointer to 0
        asm_file << "   cmp byte [rsi], 0" << endl;
        // Jump back to the start of the loop if the byte is not 0
        asm_file << "   jne loop_" << loop_count << endl;
        break;
      default:
        // Ignore any other character
        break;
    }
  }

  // Close the program
  asm_file << "end_program:" << endl;
  // Use the exit syscall (60) to exit the program
  asm_file << "    mov rax, 60\n";
  asm_file << "    xor rdi, rdi ; exit code 0\n";
  asm_file << "    syscall ; invoke system call\n";

  asm_file.close();
  cout << "Assembly code generated and written to " << output_file << endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
      cerr << "Usage: " << argv[0] << " <file_name>" << endl;
      return 1;
    }

    string text = read_file(argv[1]);
    text = filter_text(text);

    compile_program(text, argv[2]);

    return 0;
}

