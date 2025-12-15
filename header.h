#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

/* define true and false */
#define TRUE 1
#define FALSE 0

#define MAX_SYMBOL_LENGTH 31
#define MAX_TYPE_LENGTH 2

#define MAX_MACRO_NAME_LENGTH 31

/* define the null-terminator */
#define NULL_TERMINATOR '\0'

/* define the new-line */
#define NEW_LINE '\n'

/* define the empty string */
#define EMPTY_STRING ""

/* define the comma */
#define COMMA ','

/* define the colon */
#define COLON ':'

/* define the amount of registers */
#define REGISTERS_AMOUNT 8

/* declare a function that returns the final filename */
char *getFileName(char *);

/* declare a function that opens a file and returns its fp */
FILE *openAssemblyFile(char *);

/* declare a function that opens / creates the preAssembler file */
FILE *openPreAssemblerFile(char *);

/* decalre a function that opens / creates the object file */
FILE *openObjectFile(char *);

/* decalre a function that opens / creates the extern file */
FILE *openExternFile(char *);

/* decalre a function that opens / creates the entry file */
FILE *openEntryFile(char *);

/* decalare a function that prints an error */
void printError(char *, ...);

/* declare a function that prints an error with a specified line number */
void printErrorInLine(char *, int, ...);

/* daclare a function that handles a memory allocation error */
void handleMemoryError();

/* declare the pre-assembler function (takes a file pointer and a filename, returns TRUE if no
   error, FALSE otherwize) */
int preAssembler(FILE *, FILE *);

/* declare the assembler function (takes the spread out assembly file and the filename) */
int assemble(FILE *, char *);

/* declare a function that changes a pointer to point to the first non-white-space character */
void skipWhiteSpaces(char **);

/* define error code */
#define SYNTAX_ERROR -1

/* define no error code */
#define NO_ERROR 1

/* define the preAssember file name */
#define PRE_ASSEMBLER_FILE_EXTENTION ".am"

/* define the error message after an error was found while assembling */
#define ERROR_WHILE_ASSEMBLING "Problem removing the pre-assembler file after an error was found while assembling"

/* define non-error code */
#define NO_ERROR 1
