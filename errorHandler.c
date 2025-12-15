#include "header.h"

/* prints an error given a format and arguments */
void printError(char *format, ...)
{
    va_list args;

    va_start(args, format);

    /* print the provided error message with the arguments */
    fprintf(stdout, "Error: ");
    vfprintf(stdout, format, args);
    fprintf(stdout, ".\n");

    va_end(args);
}

/* prints an error given a line, format and arguments */
void printErrorInLine(char *format, int lineNum, ...)
{
    va_list args;

    va_start(args, lineNum);

    /* print the provided error message with the arguments */
    fprintf(stdout, "Error in line %d: ", lineNum);
    vfprintf(stdout, format, args);
    fprintf(stdout, ".\n");

    va_end(args);
}

/* function that prints and throws a memory error */
void handleMemoryError()
{
    printError("Memory allocation failed.\n");
    exit(EXIT_FAILURE);
}