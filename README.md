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
```