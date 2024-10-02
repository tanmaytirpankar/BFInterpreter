import os
import sys
import time

if __name__ == '__main__':
    path = sys.argv[1]

    enable_profiler = False
    if sys.argv[2] == "-p":
        enable_profiler = True

    scratch_dir = os.getcwd() + "/../scratch/"

    # get filename from path (remove the extension)
    root = os.path.dirname(path)
    full_root = os.path.abspath(root)
    filename = os.path.splitext(os.path.basename(path))[0]

    # Call the bf_compiler on the filename
    os.system("../build-debug/bin/brainfuck_compiler " +
              root + '/' + filename + ".b " +
              scratch_dir + filename + ".asm" +
              " -p" if enable_profiler else "")
    os.system("nasm -f elf64 " + scratch_dir + filename + ".asm")
    os.system("ld " + scratch_dir + filename + ".o -o " + scratch_dir + filename)

    print("\n")
    # Measure execution time of next command
    start = time.time()

    os.system(scratch_dir + filename)

    end = time.time()
    print("Execution time: ", end-start, " seconds")
