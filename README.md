# Decimal Computer Simulator

This educational tool designed to simulate a single-address decimal computer architecture. This project provides a complete environment for writing, compiling and debugging assembly code with real-time hardware visualization.

## Key Features
* **CPU and Memory Simulation:** Modeling of decimal registers (Accumulator, Program Counter, Instruction Register, Stack Pointer, Index Register) and flags (overflow, waiting for input, halted). Instruction format: 00111, where 00 are digits for the opcode and 111 - for the address (which means that the memory has a size of 1000 cells)
* **Assembler:** Convert human-readable assembly instructions into machine code
* **Interactive Debugger:** Support for step-by-step execution, breakpoints and adjustable minimum step duration
* **Statistics:** Real-time monitoring of ALU, Memory, Flow Control, and I/O instruction distribution using Qt Graph

## Tech Stack
* **Language:** C++17
* **Framework:** Qt 6.10.2 (Widgets, QuickWidgets, Graphs, QML)
* **Build System:** CMake 3.16 (using CMake Presets)
* **Compiler:** MSVC 2022
* **Build Tool:** Ninja

## Important
Ensure you have created QT_DIR environment variable in your Path

(e.g. C:\Qt\6.10.2\msvc2022_64\bin)

## Project Structure
src/core/ — Core emulation logic (CPU, Memory, Computer)

src/assembler/ — Assembler and Disassembler implementation

src/gui/ — Graphical user interface and Qt Graphs integration

assets/ — Application icons, and Windows resource files

## Simulator uses own assembly grammar

program ::= { line }

line ::= [ whitespaces ] [ statement ] [ whitespaces ] [ comment ] NEWLINE

statement ::= label_decl | instruction | label_decl whitespaces instruction

label_decl ::= label COLON

label ::= identifier

instruction ::= no_operand_cmd | operand_cmd whitespaces operand | pseudo_cmd_dat [ whitespaces value ]

no_operand_cmd ::= "HLT" | "NOP" | "INP" | "OUT" | "NEG" | "INC" | "DEC" | "PUSH" | "POP" | "RET" | "INX" | "DEX"

operand_cmd ::= "LDA" | "STA" | "ADD" | "SUB" | "MUL" | "DIV" | "MOD" | "ADDI" | "SUBI" | "MULI" | "DIVI" | "MODI" | "BRA" | "BRZ" | "BRP" | "BRN" | "BRO" | "CALL" | "LDX" | "STX" | "LDXI" | "LDAX" | "STAX" | "ADDX" | "SUBX" | "MULX" | "DIVX"

pseudo_cmd_dat ::= "DAT"

operand ::= number | identifier

value ::= number | identifier

identifier ::= letter_or_underscore { letter_or_underscore | digit }

number ::= [ MINUS ] digit { digit }

comment ::= SEMICOLON { ANY_CHARACTER }

whitespaces ::= space { space }

letter_or_underscore ::= 'a' | 'b' | ... | 'z' | 'A' | 'B' | ... | 'Z' | '_'

digit ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'

space ::= ' ' | '\t'

COLON ::= ':'

SEMICOLON ::= ';'

MINUS ::= '-'

NEWLINE ::= '\n'

ANY_CHARACTER ::= ? all characters except NEWLINE ?

## Instructions

Instruction Name (Opcode, Type, Has Operand)

HLT (00, CONTROL, No)

Halts program execution (sets halted flag to true).

NOP (01, CONTROL, No)

No operation; consumes one CPU cycle.

LDA (10, MEMORY, Yes)

Loads the value from the specified memory address into the accumulator.

STA (11, MEMORY, Yes)

Stores the current accumulator value into the specified memory address.

INP (18, IO, No)

Pauses CPU execution and waits for user input.

OUT (19, IO, No)

Pushes the current accumulator value into the output buffer.

ADD (20, ALU, Yes)
Adds the value read from the specified memory address to the accumulator.

SUB (21, ALU, Yes)

Subtracts the value read from the specified memory address from the accumulator.

MUL (22, ALU, Yes)

Multiplies the accumulator by the value read from the specified memory address.

DIV (23, ALU, Yes)

Divides the accumulator by the value read from the specified memory address.

MOD (24, ALU, Yes)

Finds the remainder of the accumulator divided by the value read from the specified memory address.

ADDI (30, ALU, Yes)

Adds the operand's immediate value to the accumulator.

SUBI (31, ALU, Yes)

Subtracts the operand's immediate value from the accumulator.

MULI (32, ALU, Yes)

Multiplies the accumulator by the operand's immediate value.

DIVI (33, ALU, Yes)

Divides the accumulator by the operand's immediate value.

MODI (34, ALU, Yes)

Finds the remainder of the accumulator divided by the operand's immediate value.

NEG (37, ALU, No)

Changes the sign of the value in the accumulator to its opposite.

INC (38, ALU, No)

Increments the accumulator value by 1.

DEC (39, ALU, No)

Decrements the accumulator value by 1.

BRA (40, CONTROL, Yes)

Unconditional branch: writes the specified address into the program counter (PC).

BRZ (41, CONTROL, Yes)

Branch if zero: jumps to the address if the accumulator equals 0.

BRP (42, CONTROL, Yes)

Branch if positive: jumps to the address if the accumulator is strictly greater than 0.

BRN (43, CONTROL, Yes)

Branch if negative: jumps to the address if the accumulator is strictly less than 0.

BRO (44, CONTROL, Yes)

Branch on overflow: jumps to the address if the overflow flag is set.

PUSH (50, MEMORY, No)

Pushes the accumulator value onto the stack and decrements the stack pointer.

POP (51, MEMORY, No)

Increments the stack pointer and pops the value from the stack into the accumulator.

CALL (52, CONTROL, Yes)

Subroutine call: pushes the current PC onto the stack and jumps to the specified address.

RET (53, CONTROL, No)

Return from subroutine: restores the PC from the stack.

LDX (60, MEMORY, Yes)

Loads the value from the specified memory address into the index register.

STX (61, MEMORY, Yes)

Stores the current index register value into the specified memory address.

LDXI (62, ALU, Yes)

Loads the operand's immediate value into the index register.

INX (63, ALU, No)

Increments the index register value by 1.

DEX (64, ALU, No)

Decrements the index register value by 1.

LDAX (65, MEMORY, Yes)

Loads the value from memory at the address (operand + index register) into the accumulator.

STAX (66, MEMORY, Yes)

Stores the accumulator value into memory at the address (operand + index register).

ADDX (67, ALU, Yes)

Adds the value from memory at the address (operand + index register) to the accumulator.

SUBX (68, ALU, Yes)

Subtracts the value from memory at the address (operand + index register) from the accumulator.

MULX (69, ALU, Yes)

Multiplies the accumulator by the value from memory at the address (operand + index register).

DIVX (70, ALU, Yes)

Divides the accumulator by the value from memory at the address (operand + index register).

## Program example

Calculate GCD of two numbers:
```asm
	INP
	STA a
	INP
	STA b
loop:	LDA b
	BRZ end
	STA temp
	LDA a
	MOD b
	STA b
	LDA temp
	STA a
	BRA loop
end:	LDA a
	OUT
	HLT
a:	DAT 0
b:	DAT 0
temp:	DAT 0
```

Notice that DAT pseudo-instruction should be used at the end of your program.
(play around and find out why!-_-)
