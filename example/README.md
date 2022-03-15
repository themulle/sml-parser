# SML parser example

## Building

```bash
cd example
mkdir build
cd build
cmake .
cmake --build .
```

## Running

After the build finished, a binary log file can be piped into the parser:

```bash
cat path/to/meter/log.bin | ./parser
```
