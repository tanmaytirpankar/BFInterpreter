# Check for cell limit and wrap back
# Double size of tape if needed

import sys

# Function to open file and read
def read_file(file_name):
    with open(file_name, 'r') as file:
        return file.read()

def mark_loops(text, loops):
    # Add tuples of start and end index of loop in text to loops
    loop_stack = []
    for i in range(len(text)):
        if text[i] == '[':
            loop_stack.append(i)
            loops.append([i, -1])
        elif text[i] == ']':
            loop_stack.pop()
            loops[len(loop_stack)][1] = i

def parse_program(text):
    ip = 0
    ptr = 0
    tape = [[0]]
    stack = []
    # loop till the end of the text

    while ip < len(text):
        # print(str(ip)+":"+text[ip])
        # match on first character
        if text[ip] == '>':
            ptr+=1
        elif text[ip] == '<':
            ptr-=1
        elif text[ip] == '+':
            while ptr >= len(tape[0]):
                tape[0]=tape[0]+[0]*len(tape[0])
            tape[0][ptr]=(tape[0][ptr]+1)%256
        elif text[ip] == '-':
            while ptr >= len(tape[0]):
                tape[0]=tape[0]+[0]*len(tape[0])
            tape[0][ptr]=(tape[0][ptr]-1)%256
        elif text[ip] == '.':
            while ptr >= len(tape[0]):
                tape[0] = tape[0] + [0] * len(tape[0])
            print(chr(tape[0][ptr]), end='')
        elif text[ip] == ',':
            while ptr >= len(tape[0]):
                tape[0]=tape[0]+[0]*len(tape[0])
            tape[0][ptr]=ord(input())
        elif text[ip] == '[':
            while ptr >= len(tape[0]):
                tape[0]=tape[0]+[0]*len(tape[0])
            if tape[0][ptr] != 0:
                stack.append(ip)
            else:
                temp_len_stack = len(stack)
                stack.append(ip)
                while len(stack) > temp_len_stack:
                    ip+=1
                    if text[ip] == '[':
                        stack.append(ip)
                    elif text[ip] == ']':
                        stack.pop()
        elif text[ip] == ']':
            while ptr >= len(tape[0]):
                tape[0]=tape[0]+[0]*len(tape[0])
            if tape[0][ptr] != 0:
                ip=stack.pop()
                ip-=1
            else:
                stack.pop()
        else:
            pass
        ip+=1
        # print(tape[0])
        # print(ptr)
        # print(stack)
        # print('\n')

# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    text = read_file(sys.argv[1])
    # filter out everything except '+-<>,.[]'
    text = ''.join(filter(lambda x: x in '+-<>,.[]', text))
    print(text)
    print(len(text))
    loops = []
    # mark_loops(text, loops)
    parse_program(text)
    # print(text)
