# CS2102 MiniProject-2026 — IITPSx Processor

## Project Overview

The aim of this mini project is to write a **two-pass assembler** for the IITPSx processor instruction set. Then write and test programs in IITPSx assembly. A final part is to write an **emulator** for the IITPSx processor (replacing the one provided).

## Tasks

1. Write a two-pass assembler for the assembly language.
   - Read assembly language from a text file, assigning label values and instruction opcodes. The format of the assembly language is described below.
   - Diagnose common assembly errors such as unknown instruction, no such label, and duplicate label.
   - Produce an object file of the produced machine code. This file should be a binary file. Code starts at address zero.
   - Produce a listing file. There is a choice of the format of the listing file. It can either be a simple memory dump, or show the bytes produced for each instruction and that instruction's mnemonic.
   - You must write the assembler in **ISO C89**. (`gcc -std=c89 -pedantic -W -Wall` is a good way to check things.)
2. Test your assembler with the sample programs listed.
3. Test your assembler with additional programs and submit evidence of this.
4. Write a **bubble sort** program in IITPSx assembly. The start of this file is provided; you have to fill in the blanks.
5. Write an emulator for the IITPSx processor. This should have some, but not need all, of the functionality of the `emu` program provided.
6. Implement and test the two new IITPSx arithmetic instructions, `mul` and `div`, in both the assembler and the emulator.

Sample `asm` and `emu` executables are given.

---

# IITPSx Assembly Language

This assembly language is for the IITPSx processor, which has four registers:

- Two registers, `A` & `B`, arranged as an internal stack.
- A program counter, `PC`.
- A stack pointer, `SP`.

These registers are **32 bits** in size.

Instructions have either no operands or a single operand. The operand is a signed 2's complement value.

The encoding uses:

- Bottom 8 bits: opcode
- Upper 24 bits: operand

## Assembly Syntax

As with most assembly languages, this is line based (one statement per line).

- Comments begin with `;` and anything on the line after the `;` is ignored.
- Blank lines and lines containing only a comment are permitted and ignored.
- White space (` ` and tabs) is permitted at the beginning of a line and is ignored.
- Label definitions consist of the label name followed by a `:`, and an optional statement. There does not necessarily need to be a space between the `:` and the statement.
- A label use is just the label name.
- For branch instructions, label use should calculate the branch displacement.
- For non-branch instructions, the label value should be used directly.
- A valid label name is an alphanumeric string beginning with a letter.
- An operand is either a label or a number.
- Numbers can be decimal, hexadecimal, or octal.

### Valid Lines

```asm
; a comment
 ; another comment
label1: ; a label on its own
ldc 5 ; an instruction
label2: ldc 5 ; a label and an instruction
 adc 5 ; an instruction
label3:ldc label3 ; look no space between label and mnemonic
```

Each statement consists of a mnemonic (instruction name) and an optional operand (number or label).

---

# IITPSx Instruction Set

The instruction semantics do not show the incrementing of the `PC` to the next instruction. This is implicitly performed by each instruction before the actions of the instruction are done.

The IITPSx processor adds two required arithmetic instructions, `mul` and `div`, using opcodes **19** and **20**.

| Mnemonic | Opcode | Operand | Formal Specification | Description |
|---|---:|---|---|---|
| `data` | — | value | — | Reserve a memory location, initialized to the value specified |
| `ldc` | 0 | value | `B := A; A := value;` | Load accumulator with the value specified |
| `adc` | 1 | value | `A := A + value;` | Add the value specified to the accumulator |
| `ldl` | 2 | offset | `B := A; A := memory[SP + offset];` | Load local |
| `stl` | 3 | offset | `memory[SP + offset] := A; A := B;` | Store local |
| `ldnl` | 4 | offset | `A := memory[A + offset];` | Load non-local |
| `stnl` | 5 | offset | `memory[A + offset] := B;` | Store non-local |
| `add` | 6 | — | `A := B + A;` | Addition |
| `sub` | 7 | — | `A := B - A;` | Subtraction |
| `shl` | 8 | — | `A := B << A;` | Shift left |
| `shr` | 9 | — | `A := B >> A;` | Shift right |
| `adj` | 10 | value | `SP := SP + value;` | Adjust SP |
| `a2sp` | 11 | — | `SP := A; A := B` | Transfer A to SP |
| `sp2a` | 12 | — | `B := A; A := SP` | Transfer SP to A |
| `call` | 13 | offset | `B := A; A := PC; PC := PC + offset;` | Call procedure |
| `return` | 14 | — | `PC := A; A := B;` | Return from procedure |
| `brz` | 15 | offset | `if A == 0 then PC := PC + offset;` | If accumulator is zero, branch to specified offset |
| `brlz` | 16 | offset | `if A < 0 then PC := PC + offset;` | If accumulator is less than zero, branch to specified offset |
| `br` | 17 | offset | `PC := PC + offset;` | Branch to specified offset |
| `HALT` | 18 | — | — | Stop the emulator. This is not a "real" instruction, but is needed to tell the emulator when to finish. |
| `mul` | 19 | — | `A := B * A;` | Multiplication |
| `div` | 20 | — | `if A == 0 then runtime error; else A := B / A;` | Signed integer division (B divided by A); division by zero must be reported by the emulator |
| `SET` | — | value | — | Set the label on this line to the specified value rather than the PC. This is an optional extension, for which additional marks are available. |

---

# Listing File Format

The listing file is produced by the assembler and is a human-readable file showing what value is stored at each address.

The format is:

- An address
- Followed by zero or one 32-bit values, represented as 8 hexadecimal characters.

With the output, you can choose to show the human-readable mnemonic and operand that each instruction corresponds to. You can also show labels by simply listing the address followed by no data bytes.

## Example: Simple Memory Dump

```text
00000000 00000111
00000001 00005AB4
00000002 00006500
00000003 00009D01
```

## Example: Four Locations per Line

```text
00000000 00000111 00005AB4 00006500 00009D01
```

## Preferred Format: Labels and Mnemonics

```text
00000000 00000111 br start
00000001 00005AB4 data 0x5ab4
00000002 start:
00000002 00006500 ldc 0x65
00000003 00009D01 adc 0x9d
```

---

# Example Programs

## `test1.asm`

This is a valid, but nonsense assembly file. The assembler should not issue any errors, although it could issue warnings.

```asm
; test1.asm
label: ; an unused label
ldc 0
ldc -5
ldc +5
loop: br loop ; an infinite loop
br next ; offset should be zero
next:
ldc loop ; load code address
ldc var1 ; forward ref
var1: data 0 ; a variable
```

---

## `test2.asm` — Error Handling

This example contains many errors. The assembler should spot them all. It does not need to copy the exact error messages, but should issue something appropriate.

```asm
; test2.asm
; Test error handling
label:
label: ; duplicate label definition
br nonesuch ; no such label
ldc 08ge ; not a number
ldc ; missing operand
add 5 ; unexpected operand
ldc 5, 6; extra on end of line
0def: ; bogus label name
fibble; bogus mnemonic
0def ; bogus mnemonic
```

Expected error categories include:

- Duplicate label definition
- Missing label
- Invalid number
- Missing operand
- Unexpected operand
- Extra text/operand at end of line
- Invalid label name
- Unknown/bogus mnemonic

---

## `test3.asm` — SET

If the `SET` pseudo-instruction is implemented:

```asm
; test3.asm
; Test SET
val: SET 75
ldc val
adc val2
val2: SET 66
```

---

## `test4.asm` — `mul` and `div`

This program demonstrates the two IITPSx arithmetic extensions. It should assemble and, when executed, leave **21 in register A**.

```asm
; test4.asm
; Test IITPSx mul and div
ldc 6
ldc 7
mul ; A = 42
ldc 2
div ; A = 21
HALT
```

---

# Notes

## Binary File

Files may be either text or binary. The default is text.

The manual page for `fopen` explains how to open a file in binary mode.

---

# Two Passes

A two-pass assembler naturally needs to scan the source file twice.

You can and should use a **single routine to do both passes**, provided that:

- On the first pass it outputs no code.
- On the first pass it does not fail on undefined labels.
- On the second pass it outputs code.
- On the second pass it does fail on an undefined label.

This is a better solution than having two different routines for each pass because it provides code reuse.

## One Read

Alternatively, you could read the program into an internal form and then process it twice.

This uses less file I/O, at the cost of using more memory. If you do this, the internal form should be more advanced than simply storing each line of the file as a string. It is a more complicated solution.

---

# Reading a Line

The functions `gets` and `fscanf` should be handled with care.

- `gets` suffers from buffer overrun. If lines are longer than the programmer expected, it can overwrite subsequent memory.
- `fscanf` can also suffer from the same problem when reading strings.
- `fscanf` has a worse flaw in that it does not recover well from errors. If the line does not contain what its format string expects, it is very hard to resynchronize.

A robust approach is to use something like:

- `fgets` to read a buffer
- Then `sscanf` or `strtok` for parsing

Also note that `getchar` and similar functions return an `int`, not a `char`. This is because they need to return all possible character values plus another value to indicate end-of-file. Therefore, storing the return value directly into a `char` variable is wrong.

---

# Tables

If you define your instructions in a suitable data structure, you can define all instructions in a table.

This is better than hard-coding particular instruction semantics throughout the program.

---

# Number Format

Numbers in the source file are written as:

- An optional sign
- A digit
- Trailing digits and letters

Not all such strings will be valid numbers.

You can use `strtol` to convert a string to a number and determine how far it got into the string.

`strtol` will also determine whether the number is:

- Decimal
- Hexadecimal — begins with `0x`
- Octal — begins with `0`

---

# Printing Label Values

One example listing output shows an instruction printed as:

```asm
br start
```

Although not explicitly mentioned, it would be permissible to show this as:

```asm
br 75
```

if the branch offset is 75. However, `br start` is preferred.

As the assembler has replaced label names with their values, the assembler can reconstruct a label name when producing the listing:

1. Check if the operand is a PC offset.
2. Calculate where the branch would go.
3. Look in the table of labels for a label at that address.
4. If one is found, use that label in the listing.
5. Otherwise, print the raw offset.

For example, the assembly source might contain:

```asm
br 7
```

There is no need to check that this actually lands on an instruction. If it does not, that is the assembly programmer's responsibility.

---

# `ldc result`

The instruction:

```asm
ldc result
```

may initially seem confusing because `ldc` loads a constant, but here it is being used with a label.

The purpose is to obtain the **address of the result data location**, so that it can be accessed as an array.

The assembler simply:

1. Looks up the label's value.
2. Places that value in the instruction's operand.

The assembler does not understand what an instruction does.

The address can later be accessed using instructions such as `ldnl`.

---

# Warnings

An **error** is something incorrect that prevents completion of the task.

A **warning** is something that is not strictly incorrect but is strange or dubious. A warning does not prevent completion of the task.

For example:

```asm
label:
```

declares a label without using it.

This is not incorrect, but it is strange and may indicate a more important problem. For example, the programmer may have misspelled a label when trying to use it and accidentally used a different label.

This is one reason why variable and function names should be very different from each other.

Avoid names such as:

```text
var1
var2
var3
```

---

# Submission

Submission is electronic.

The submission link given in the project brief is:

`https://u.pcloud.com/#/puplink?code=F0wXZD4XBYBEvrpVoSK2TR6suKbtX5MAy`

## Required Files

Submit the following files:

- Source code for the assembler. The main (and possibly only) file should be called `asm.c`.
- Source code for the emulator, if that part was completed. The main (and possibly only) file should be called `emu.c`.
- Programs should either be compilable with:

```bash
gcc asm.c -o asm
gcc emu.c -o emu
```

  or, if the project is more complicated, submit a `Makefile` containing the correct rules.
- Test assembly files used to verify the assembler and emulator:
  - `test01.asm`
  - `test02.asm`
  - etc.
- Output files from tests:
  - `test01.log`
  - etc.
- Assembler listing files:
  - `test01.lst`
  - etc.
- The `claims` text file explained below.

## Important Note

You will receive **no marks** if you do not deliver the following:

- Evidence that your assembler works. It is not acceptable to submit only a program listing.
- A text file named `claims` that lists, according to the marking scheme, the claims you make about your submission.
- When asked by the marker, you should be able to explain in deep detail the inner workings and structure of your code.

---

# Submission Deadline

**Midnight, 11th October 2026**

---

# Marking Scheme

## General Marks

This applies to all submitted code, including both the assembler and emulator.

- Correctly submitted source files with your name in them.
- All files should have your name and user ID at the start, along with a declaration of authorship. *
- Compiles without error when being pedantic:

```bash
gcc -std=c89 -pedantic
```

- Compiles without warnings:

```bash
gcc -std=c89 -pedantic -W -Wall -Wpointer-arith -Wwrite-strings -Wstrict-prototypes
```

- Consistent and sensible formatting with sensible program structure.
- Sensible variable, function, and type names with explanatory comments. * (very important issue)

## Assembler — 60%

- Either uses a single process/routine for both passes, or reads into an internal form on the first pass. *  
  **Only one of these approaches should be used, not both.**
- Diagnoses label errors such as duplicate and missing labels. *
- Produces a listing file, at least a memory dump. *
- Advanced listing file.
- Has a table of instruction names and expected operands. *
- Assembles the test program. *
- Additional test programs for both failure and success, including a brief explanation as comments within each test file. *
- Implements and demonstrates the `SET` instruction.
- Implements and demonstrates the IITPSx `mul` and `div` instructions. *
- Bubble sort program:
  - Assembles. *
  - Works. *

## Emulator — 25%

- Loads object file.
- Produces memory dump.
- Executes test program.
- Executes `mul` and `div` correctly, including detection of division by zero. *
- Detects errant programs.

## Test Cases — 15%

Create multiple algorithms in IITPSx assembly as test cases using the IITPSx instruction set.

Examples include:

- Adding `n` numbers
- Computing a polynomial
- Mean
- Median
- Standard deviation
- Other algorithms

Include **at least one test that exercises `mul` and `div`**.

> **Note:** The `*` symbol labels tasks considered essential, i.e. the minimum for a satisfactory standard.
