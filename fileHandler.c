#include "header.h"
#include "fileHandler.h"

/* returns the final filename */
char *getFileName(char *filename)
{
    char *dot = strrchr(filename, DOT); /* find the content after the dot (including the dot) */

    if (dot != NULL && strcmp(dot, ASSEMBLY_FILE_EXTENTION) != 0)
    {
        printError("%s is not an assembly file", filename);
        return NULL; /* indicate an error */
    }
    else if (dot != NULL && strcmp(dot, ASSEMBLY_FILE_EXTENTION) == 0)
    {
        dot = NULL_TERMINATOR; /* ensure to return only the name */
        return filename;       /* extention was already inserted */
    }

    return filename;
}

/* opens a file and ensures its assembly */
FILE *openAssemblyFile(char *filename)
{
    FILE *file;

    /* allocate memory for the new filename with the extension (+ 1 for null-terminator) */
    char *finalFilename = malloc(strlen(filename) + strlen(ASSEMBLY_FILE_EXTENTION) + 1);
    if (!finalFilename)
    {
        handleMemoryError();
    }

    /* copy the file name and concatenate the extension */
    strcpy(finalFilename, filename);
    strcat(finalFilename, ASSEMBLY_FILE_EXTENTION);

    /* open the file */
    file = fopen(finalFilename, READ);
    if (file == NULL)
    {
        printError("Couldn't open file: %s\n", finalFilename);
        free(finalFilename);
        return NULL; /* indicate an error */
    }

    free(finalFilename);

    return file; /* return the file pointer */
}

/* opens / creates the preAssembler file */
FILE *openPreAssemblerFile(char *finalFilename)
{
    /* open the file */
    FILE *preAssemblerFile = fopen(finalFilename, WRITE_AND_READ);
    if (preAssemblerFile == NULL)
    {
        printError("Couldn't create file: %s\n", finalFilename);
        free(finalFilename);
        return NULL; /* indicate an error */
    }

    return preAssemblerFile; /* return the file pointer */
}

/* opens / creates the object file */
FILE *openObjectFile(char *filename)
{
    FILE *objectFile;

    /* allocate memory for the new filename with the extension (+ 1 for null-terminator) */
    char *finalFilename = malloc(strlen(filename) + strlen(OBJECT_FILE_EXTENSION) + 1);
    if (!finalFilename)
    {
        handleMemoryError();
    }

    /* copy the file name and concatenate the extension */
    strcpy(finalFilename, filename);
    strcat(finalFilename, OBJECT_FILE_EXTENSION);

    /* open the file */
    objectFile = fopen(finalFilename, WRITE);
    if (objectFile == NULL)
    {
        printError("Couldn't open file: %s\n", finalFilename);
        free(finalFilename);
        return NULL; /* indicate an error */
    }

    free(finalFilename);

    return objectFile; /* return the file pointer */
}

/* opens / creates the external file */
FILE *openExternFile(char *filename)
{
    FILE *externFile;

    /* allocate memory for the new filename with the extension (+ 1 for null-terminator) */
    char *finalFilename = malloc(strlen(filename) + strlen(EXTERNAL_FILE_EXTENSTION) + 1);
    if (!finalFilename)
    {
        handleMemoryError();
    }

    /* copy the file name and concatenate the extension */
    strcpy(finalFilename, filename);
    strcat(finalFilename, EXTERNAL_FILE_EXTENSTION);

    /* open the file */
    externFile = fopen(finalFilename, WRITE);
    if (externFile == NULL)
    {
        printError("Couldn't open file: %s\n", finalFilename);
        free(finalFilename);
        return NULL; /* indicate an error */
    }

    free(finalFilename);

    return externFile; /* return the file pointer */
}

/* function that creates / opens the entry file */
FILE *openEntryFile(char *filename)
{
    FILE *entryFile;

    /* allocate memory for the new filename with the extension (+ 1 for null-terminator) */
    char *finalFilename = malloc(strlen(filename) + strlen(ENTRY_FILE_EXTENSTION) + 1);
    if (!finalFilename)
    {
        handleMemoryError();
    }

    /* copy the file name and concatenate the extension */
    strcpy(finalFilename, filename);
    strcat(finalFilename, ENTRY_FILE_EXTENSTION);

    /* open the file */
    entryFile = fopen(finalFilename, WRITE);
    if (entryFile == NULL)
    {
        printError("Couldn't open file: %s\n", finalFilename);
        free(finalFilename);
        return NULL; /* indicate an error */
    }

    free(finalFilename);

    return entryFile; /* return the file pointer */
}
