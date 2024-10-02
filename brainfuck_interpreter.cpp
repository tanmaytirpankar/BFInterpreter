#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <chrono>
#include <algorithm>

using namespace std;

bool enable_profiler = false;

unordered_map<int, int> instruction_count;
unordered_map<int, int> loop_map;
unordered_map<string, int> non_simple_loops;
unordered_map<string, int> simple_loops;

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

  if(enable_profiler) {
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

// Function to parse and execute the brainfuck-like program
void parse_program(const string &text) {
  int ip = 0;  // instruction pointer
  int ptr = 0; // memory pointer

  // Start with a larger tape size to avoid frequent resizing
  vector<char> tape(30000, 0);

  // Process each character in the input program
  while (ip < text.size()) {
    char command = text[ip];
    instruction_count[ip]++;

    switch (command) {
      case '>':  // move pointer right
        ptr++;
        break;

      case '<':  // move pointer left
        if (ptr > 0)
          ptr--;
        else {
          cout << "ptr cannot be negative" << endl;
          exit(1);
        };
        break;

      case '+':  // increment the value at current cell
        tape[ptr]++;
        break;

      case '-':  // decrement the value at current cell
        tape[ptr]--;
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
        } else if(enable_profiler) {
          if(non_simple_loops.find(text.substr(ip, loop_map[ip] - ip + 1)) != non_simple_loops.end()) {
            non_simple_loops[text.substr(ip, loop_map[ip] - ip + 1)]++;
          }
          if(simple_loops.find(text.substr(ip, loop_map[ip] - ip + 1)) != simple_loops.end()) {
            simple_loops[text.substr(ip, loop_map[ip] - ip + 1)]++;
          }
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

  if(enable_profiler) {
    // Print instruction count
    cout << "Instruction counts:" << endl;
    for (auto &pair : instruction_count) {
      cout << pair.first << "(" << text[pair.first] << "):" << pair.second << endl;
    }
    cout << endl;

    // Print sizes of loop_map, non_simple_loops and simple_loops
//    cout << "#Loops: " << loop_map.size() / 2  << endl;
    cout << "#Simple loops: " << simple_loops.size() << endl;
    cout << "#Non-simple loops: " << non_simple_loops.size() << endl;
    cout << endl;

    // sort the simple and non-simple loops by frequency
    vector<pair<string, int>> simple_loops_vec(simple_loops.begin(), simple_loops.end());
    vector<pair<string, int>> non_simple_loops_vec(non_simple_loops.begin(), non_simple_loops.end());
    sort(simple_loops_vec.begin(), simple_loops_vec.end(), [](const pair<string, int> &a, const pair<string, int> &b) {
      return a.second > b.second;
    });
    sort(non_simple_loops_vec.begin(), non_simple_loops_vec.end(), [](const pair<string, int> &a, const pair<string, int> &b) {
      return a.second > b.second;
    });

    // Print the simple loop bodies
    cout << "Simple loop bodies:" << endl;
    for (auto &pair : simple_loops_vec) {
      cout << pair.first << ": " << pair.second << endl;
    }
    cout << endl;

    // Print the non-simple loop bodies
    cout << "Non-simple loop bodies:" << endl;
    for (auto &pair : non_simple_loops_vec) {
      cout << pair.first << ": " << pair.second << endl;
    }
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
