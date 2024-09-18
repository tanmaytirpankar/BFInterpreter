#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <chrono>
#include <algorithm>
#include <cassert>

using namespace std;

bool enable_profiler = false;

unordered_map<int, int> loop_map;
unordered_map<int, int> inner_most_loop_map;
unordered_map<int, int> simple_loops;
unordered_map<char, int> instruction_count;
unordered_map<int, int> loop_execution_count;

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
  inner_most_loop_map.clear();
  simple_loops.clear();

  // Initialize instruction counter
  instruction_count['>'] = 0;
  instruction_count['<'] = 0;
  instruction_count['+'] = 0;
  instruction_count['-'] = 0;
  instruction_count['.'] = 0;
  instruction_count[','] = 0;
  instruction_count['['] = 0;
  instruction_count[']'] = 0;
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

  if(enable_profiler) {
    assert(loop_stack.empty());
    bool is_inner_most_loop = false;
    bool is_simple_loop = false;
    int loop_body_pointer_offset = 0;
    int start_pointer_change = 0;

    for (int i = 0; i < text.size(); i++) {
      if (text[i] == '[') {
        loop_stack.push(i);
        is_inner_most_loop = true;
        is_simple_loop = true;
        loop_body_pointer_offset = 0;
      } else if (text[i] == ']') {
        if (loop_stack.empty()) {
          cerr << "Mismatched ']' at position " << i << endl;
          exit(1);
        }
        int open_pos = loop_stack.top();
        if (is_inner_most_loop) {
          inner_most_loop_map[open_pos] = i;
          is_inner_most_loop = false;
          if (is_simple_loop && loop_body_pointer_offset == 0 &&
              (start_pointer_change == -1 || start_pointer_change == 1)) {
            simple_loops[open_pos] = i;
            is_simple_loop = false;
          }
        }
        loop_stack.pop();
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

// Function to parse and execute the brainfuck-like program
void parse_program(const string &text) {
  int ip = 0;  // instruction pointer
  int ptr = 0; // memory pointer

  // Start with a larger tape size to avoid frequent resizing
  vector<int> tape(30000, 0);

  // Process each character in the input program
  while (ip < text.size()) {
    char command = text[ip];

    switch (command) {
      case '>':  // move pointer right
        if(enable_profiler)
          instruction_count['>']++;
        ptr++;
        break;

      case '<':  // move pointer left
        if(enable_profiler)
          instruction_count['<']++;
        if (ptr > 0)
          ptr--;
        else {
          cout << "ptr cannot be negative" << endl;
          exit(1);
        };
        break;

      case '+':  // increment the value at current cell
        if(enable_profiler)
          instruction_count['+']++;
        tape[ptr] = (tape[ptr] + 1) % 256;
        break;

      case '-':  // decrement the value at current cell
        if(enable_profiler)
          instruction_count['-']++;
        tape[ptr] = (tape[ptr] - 1 + 256) % 256;
        break;

      case '.':  // output the value at current cell as character
        if(enable_profiler)
          instruction_count['.']++;
        putchar(tape[ptr]);
        break;

      case ',':  // read a character from input into the current cell
        if(enable_profiler)
          instruction_count[',']++;
        tape[ptr] = getchar();
        break;

      case '[':  // begin loop
        if(enable_profiler) {
          instruction_count['[']++;
          if(inner_most_loop_map.find(ip) != inner_most_loop_map.end() && ip < inner_most_loop_map[ip]) {
            loop_execution_count[ip]++;
          }
        }
        if (tape[ptr] == 0) {
          ip = loop_map.at(ip);  // jump to the matching ']'
        }
        break;

      case ']':  // end loop
        if(enable_profiler)
          instruction_count[']']++;
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

  if(enable_profiler) {
    // Print instruction count
    cout << "Instruction counts:" << endl;
    for (auto &pair : instruction_count) {
      cout << pair.first << ": " << pair.second << endl;
    }
    cout << endl;

    // Print sizes of loop_map, inner_most_loop_map and simple_loops
//    cout << "#Loops: " << loop_map.size() << endl;
    cout << "#Inner most loops: " << inner_most_loop_map.size() << endl;
    cout << "#Simple loops: " << simple_loops.size() << endl;
    cout << endl;

    // sort the loop execution counts by value
    vector<pair<int, int>> sorted_loop_execution_count;
    for (auto &pair : loop_execution_count) {
      sorted_loop_execution_count.push_back(pair);
    }
    sort(sorted_loop_execution_count.begin(), sorted_loop_execution_count.end(), [](const pair<int, int> &a, const pair<int, int> &b) {
        return a.second > b.second;
    });

    // Print the inner most loop execution counts that are simple loops
    cout << "Simple loop execution counts:" << endl;
    for (auto &pair : sorted_loop_execution_count) {
      if (simple_loops.find(pair.first) != simple_loops.end()) {
        cout << pair.first << ": " << pair.second << endl;
      }
    }
    cout << endl;

    // Print the inner most loop execution counts that are not simple loops
    cout << "Non-simple loop execution counts:" << endl;
    for (auto &pair : sorted_loop_execution_count) {
      if (simple_loops.find(pair.first) == simple_loops.end()) {
        cout << pair.first << ": " << pair.second << endl;
      }
    }

    // Print the simple loop bodies
//    cout << "Simple loop bodies:" << endl;
//    for (auto &pair : simple_loops) {
//      cout << text.substr(pair.first + 1, pair.second - pair.first - 1) << endl;
//    }

  }
}

// Main function to execute the program
int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <file_name>" << endl;
    return 1;
  }

  // Check for flag "-p" to enable profiler
  if (argc == 3 && string(argv[2]) == "-p") {
    enable_profiler = true;
  }

  // Record time
  const auto start = chrono::high_resolution_clock::now();

  string text = read_file(argv[1]);
  text = filter_text(text); // filter out non-instruction characters
//  cout << "Program Length: " << text.size() << endl;

  initialize_globals();

  // Preprocess loop start and end positions
  preprocess_loops(text);

  // Execute the brainfuck program
  parse_program(text);

  // Record time
  const auto end = chrono::high_resolution_clock::now();
  chrono::duration<double> elapsed = end - start;
  cout << "\nTime taken: " << elapsed.count() << " seconds" << endl;

  return 0;
}
