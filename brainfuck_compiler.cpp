#include <iostream>
#include <fstream>
#include <stack>
#include <unordered_map>

using namespace std;

bool enable_profiler = false;

unordered_map<int, int> loop_map;
unordered_map<string, int> non_simple_loops;
unordered_map<string, int > simple_loops;
unordered_map<string, string> optimizations;


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

// Initialize global variables
void initialize_globals() {
  loop_map.clear();
  non_simple_loops.clear();
  simple_loops.clear();
}

// Preprocessing to match loops and store their positions
bool preprocess_loops(const string &text) {
  stack<int> loop_stack;

  for (int i = 0; i < text.size(); i++) {
    if (text[i] == '[') {
      loop_stack.push(i);
    } else if (text[i] == ']') {
      if (loop_stack.empty()) {
        cerr << "Mismatched ']' at position " << i << endl;
        exit(1);
      }
      int open_pos = loop_stack.top();
      loop_map[open_pos] = i;
      loop_stack.pop();
      loop_map[i] = open_pos;
    }
  }

  if (!loop_stack.empty()) {
    cerr << "Mismatched '[' at position " << loop_stack.top() << endl;
    exit(1);
  }

  if (enable_profiler) {
    bool is_inner_most_loop = false;
    bool is_simple_loop = false;
    int loop_body_pointer_offset = 0;
    int start_pointer_change = 0;

    for (int i = 0; i < text.size(); i++) {
      if (text[i] == '[') {
        is_inner_most_loop = true;
        is_simple_loop = true;
        loop_body_pointer_offset = 0;
        start_pointer_change = 0;
      } else if (text[i] == ']') {
        if (is_inner_most_loop) {
          is_inner_most_loop = false;
          if (is_simple_loop && loop_body_pointer_offset == 0 &&
              (start_pointer_change == -1 || start_pointer_change == 1)) {
            simple_loops[text.substr(loop_map[i], i - loop_map[i] + 1)] = 0;
            is_simple_loop = false;
          } else {
            non_simple_loops[text.substr(loop_map[i], i - loop_map[i] + 1)] = 0;
          }
        }
      } else if (is_inner_most_loop && is_simple_loop) {
        if (text[i] == '.' || text[i] == ',') {
          is_simple_loop = false;
        } else if ((text[i] == '+' || text[i] == '-') && loop_body_pointer_offset == 0) {
          if (text[i] == '+') {
            start_pointer_change++;
          } else {
            start_pointer_change--;
          }
        } else if (text[i] == '<' || text[i] == '>') {
          loop_body_pointer_offset += (text[i] == '<') ? -1 : 1;
        }
      }
    }
  }

  return true;
}

void add_optimizations() {
  // Add optimizations here
  // Example:
  optimizations["[-]"] = "   mov byte [rsi], 0";
  optimizations["[+]"] = "   mov byte [rsi], 0";
  optimizations["[-<<<<<<<<->>>>>>>>]"] = "   sub rsi, 8";
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
  int loop_counter = 0;
  int curr_loop=0;

  int ip = 0;  // instruction pointer

  while (ip < text.size()) {
    char bf_op = text[ip];
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
        loop_stack.push(loop_counter);
        // Label the loop
        asm_file << "loop_" << loop_counter << ":" << endl;
        // Compare the byte at the data pointer to 0
        asm_file << "   cmp byte [rsi], 0" << endl;
        // Jump to the end of the loop if the byte is 0
        asm_file << "   je loop_end_" << loop_counter << endl;
        loop_counter++;

        if(enable_profiler) {
            if(non_simple_loops.find(text.substr(ip, loop_map[ip] - ip + 1)) != non_simple_loops.end()) {
              non_simple_loops[text.substr(ip, loop_map[ip] - ip + 1)]++;
            }
            if(simple_loops.find(text.substr(ip, loop_map[ip] - ip + 1)) != simple_loops.end()) {
              simple_loops[text.substr(ip, loop_map[ip] - ip + 1)]++;
            }
        }
        break;
      case ']':
        // End of a loop
        if (loop_stack.empty()) {
          cerr << "Unmatched ']' exiting" << endl;
          return;
        }
        curr_loop = loop_stack.top();
        loop_stack.pop();
        // Label loop end
        asm_file << "loop_end_" << curr_loop << ":" << endl;
        // Compare the byte at the data pointer to 0
        asm_file << "   cmp byte [rsi], 0" << endl;
        // Jump back to the start of the loop if the byte is not 0
        asm_file << "   jne loop_" << curr_loop << endl;
        break;
      default:
        // Ignore any other character
        break;
    }

    ip++; // Move to the next instruction
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

    // Check for flag "-p" to enable profiler
    if (argc == 4 && string(argv[3]) == "-p") {
      enable_profiler = true;
    }

    string text = read_file(argv[1]);
    text = filter_text(text);
    initialize_globals();
    // Preprocess loop start and end positions
    preprocess_loops(text);

    // Add optimizations
    add_optimizations();

    if(enable_profiler) {
      cout << "#Simple loops: " << simple_loops.size() << endl;
      cout << "#Non-simple loops: " << non_simple_loops.size() << endl;
      cout << endl;

      // Print the simple loop bodies
      cout << "Simple loop bodies:" << endl;
      for (auto &pair: simple_loops) {
        cout << pair.first << ": " << pair.second << endl;
      }
      cout << endl;

      // Print the non-simple loop bodies
      cout << "Non-simple loop bodies:" << endl;
      for (auto &pair: non_simple_loops) {
        cout << pair.first << ": " << pair.second << endl;
      }
    }

    compile_program(text, argv[2]);

    return 0;
}

