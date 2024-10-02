import os
import sys

if __name__ == '__main__':
    path = sys.argv[1]
    scratch_dir = os.getcwd() + "/../scratch/"

    # get filename from path (remove the extension)
    root = os.path.dirname(path)
    full_root = os.path.abspath(root)
    filename = os.path.splitext(os.path.basename(path))[0]

    # Call the bf_compiler on the filename
    os.system("../build-debug/bin/brainfuck_compiler " + root + '/' + filename + ".b " + scratch_dir + filename + ".asm")
    os.system("nasm -f elf64 " + scratch_dir + filename + ".asm")
    os.system("ld " + scratch_dir + filename + ".o -o " + scratch_dir + filename)
