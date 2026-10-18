

## Requirements

- GCC or another C compiler with C89 support
- `make` (optional; the programs can also be compiled directly)
- Python 3 with Tkinter, only if you want to use the GUI

## Build

From this directory, run:

```sh
make
```

This builds `asm` and `emu` with the C89 and warning flags in the Makefile.
Alternatively:

```sh
gcc -std=c89 -pedantic -W -Wall -Wpointer-arith -Wwrite-strings -Wstrict-prototypes asm.c -o asm
gcc -std=c89 -pedantic -W -Wall -Wpointer-arith -Wwrite-strings -Wstrict-prototypes emu.c -o emu
```

## Assemble and run a program

The assembler accepts a source file and optional object and listing paths:

```sh
./asm tests/test04.asm out.bin out.lst
./emu out.bin
```

The emulator also supports execution tracing, a step limit, and a memory dump:

```sh
./emu -trace -steps 500000 out.bin
./emu -dump memory.txt out.bin
```

## Tests and expected behavior

`tests/` contains additional programs and their captured listings, object files,
and logs. `tests/bubble.asm` sorts the five values `[5, 3, 4, 1, 2]` into
`[1, 2, 3, 4, 5]`. The other valid programs cover `SET`, `mul`, `div`, loops,
and arithmetic. `test01.asm` is intentionally an infinite loop, `test02.asm`
intentionally contains assembly errors, and `test06.asm` intentionally divides
by zero; those error outcomes are expected.

`provided_tests/` contains the supplied sample programs and captured outputs.
The `expected/` subdirectory contains reference object, listing, and log files
for comparison. In particular, `provided_tests/test3.asm` is an assembler test
for `SET`; it has no `HALT`, so it is not intended to run to completion in the
emulator.

The `claims` file records the assembler verification commands and results,
including byte-for-byte comparisons against the supplied reference objects and
the expected diagnostics for invalid samples.

To run the additional tests one by one, assemble each `.asm` file in `tests/`
and run the resulting object with `emu`. Some programs are designed to report
an assembly or runtime error, as described above.

## GUI

Run the Tkinter front end with:

```sh
python3 gui.py
```

The GUI can open, save, assemble, run, step through, and dump programs. It
resolves project files relative to `gui.py`, so it can be launched from another
working directory. The `Run all tests` action uses the programs in `tests/`.
