# Compiler
Compiler/Parser generator project based on 'Compilers: Principles, Techniques, and Tools' aka the Dragon book

This is a hobby project I'm working on occasionally in my free time. It is not (yet) production quality code 
but hopefully with time it will become a full fledged parser generator.

The end goal is to have a parser generator similar to yack/bison where you can specify a grammar in a text file from which
the parser generator will generate the source code for a parser that can handle the grammar you specified.

Currently the grammar has to be specified directly in code, similar to how grammar is defined using boost spirit. It is not
using templates though for grammar definition as the end goal is to load the grammar at runtime like is done in yack/bison.

Features:
- regex parser with full unicode support.
- LR, SLR and CLR type parsers can be created

Final word:
As the title suggests this code is heavily inspired by the book 'Compilers: Principles, Techniques, and Tools' which 
I consider the best book out there on compiler design. The terminology used in the code can all be found in this book.
