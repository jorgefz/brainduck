
# Brainduck

An interpreter for the Brainfuck programming language (https://en.wikipedia.org/wiki/Brainfuck).

It has been tested on GNU/Linux (Ubuntu 20.04) with the GCC compiler (version 9.3.0).

## Building from source

Simply run the script 'make.sh', which will compile Brainduck and create an executable on the current directory.

## Run Brainfuck scripts

There is a small sample of Brainfuck scripts in the 'scripts' folder, which can be run with the following command:

```
./brainduck scripts/helloworld.bf
```

If you want to print out the final state of the stack, use the '--debug' option:

```
./brainduck scripts/helloworld.bf --debug
```

