#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <chrono>

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

// Preprocessing to match loops and store their positions
unordered_map<int, int> preprocess_loops(const string &text) {
  unordered_map<int, int> loop_map;
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
      loop_stack.pop();
      loop_map[open_pos] = i;
      loop_map[i] = open_pos;
    }
  }

  if (!loop_stack.empty()) {
    cerr << "Mismatched '[' at position " << loop_stack.top() << endl;
    exit(1);
  }

  return loop_map;
}

// Function to parse and execute the brainfuck-like program
void parse_program(const string &text, const unordered_map<int, int> &loop_map) {
  int ip = 0;  // instruction pointer
  int ptr = 0; // memory pointer

  // Start with a larger tape size to avoid frequent resizing
  vector<int> tape(30000, 0);

  // Process each character in the input program
  while (ip < text.size()) {
    char command = text[ip];

    switch (command) {
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
          ip = loop_map.at(ip);  // jump to the matching ']'
        }
        break;

      case ']':  // end loop
        if (tape[ptr] != 0) {
          ip = loop_map.at(ip);  // jump back to the matching '['
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
    cerr << "Usage: " << argv[0] << " <file_name>" << endl;
    return 1;
  }

  // Record time
  const auto start = chrono::high_resolution_clock::now();

  string text = read_file(argv[1]);
  text = filter_text(text); // filter out non-instruction characters
  cout << "Program Length: " << text.size() << endl;

  // Preprocess loop start and end positions
  unordered_map<int, int> loop_map = preprocess_loops(text);

  // Execute the brainfuck-like program
  parse_program(text, loop_map);

  // Record time
  const auto end = chrono::high_resolution_clock::now();
  chrono::duration<double> elapsed = end - start;
  cout << "\nTime taken: " << elapsed.count() << " seconds" << endl;

  return 0;
}
