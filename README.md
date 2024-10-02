# Building

```bash
mkdir build && cd build
cmake ..
make
```

# Running

```bash
cd bin
./brainfuck_interpreter_c_threaded ../../benchmarks/mandelbrot.b
./brainfuck_interpreter_c ../../benchmarks/mandelbrot.b
./brainfuck_interpreter_cpp ../../benchmarks/mandelbrot.b
./brainfuck_compiler ../../benchmarks/mandelbrot.b <output_file.asm>
```

Use the `-p` flag with the brainfuck_interpreter_cpp target to enable the profiler

# Using the brainfuck compiler
```bash
./brainfuck_compiler <brainfuck_file> <output_file.asm> [-p]
nasm -f elf64 <output_file.asm>
ld <output_file.o> -o <executable_name>
./executable
```

The `-p` flag is to enable the profiler which gathers information about loops.

# Using the compile and execute script
Instead of using the brainfuck compiler executable in the way described above, you can use this script to do it in a 
single command
```bash
cd scripts
python3 compile_and_execute <relative/path/to/benchmark> [-p]
```

## Notes for Brainfuck to X86_64 compiler

Note: We are using the nasm assembler so some notes pertain to that assembler specifically

### Memory allocation
We allocate memory for 30,000 cells (30,000 b) and label the start as tape in the `.bss` section.
`tape resb 30000`

### Data pointer
We will use the `rsi` register as our data pointer on tape so we initialize it to point to tape

### Syscalls
The syscall number should be set in register `rax`
`rdi` contains the filedescriptor (0 for stdin and 1 for stdout)
`rsi` should contain the pointer to the buffer
`rdx` contains the number of bytes