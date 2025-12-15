#include "header.h"
#include "assemble.h"

/* creates / wrights to the object file (if needed) */
void writeObjectFile(int icf, int dcf, char *filename, MemoryNode *instructionHead, MemoryNode *dataHead)
{
    FILE *objectFile;

    /* ensure the object file wouldn't be empty */
    if (icf == INITIAL_IC && dcf == INITIAL_DC)
    {
        return;
    }

    objectFile = openObjectFile(filename);
    if (objectFile == NULL)
    {
        return; /* an error was found opening the file (already printed) */
    }

    /* write the 'title' of the file */
    fprintf(objectFile, "%7d %d\n", icf - INITIAL_IC, dcf);

    /* write the instruction code first */
    while (instructionHead != NULL)
    {
        fprintf(objectFile, "%07d %06x\n", instructionHead->value, instructionHead->code & MASK_24BIT);
        instructionHead = instructionHead->next;
    }

    /* now write the data code */
    while (dataHead != NULL)
    {
        /* + icf because the addresses start at INITIAL_IC */
        fprintf(objectFile, "%07d %06x\n", dataHead->value + icf, dataHead->code & MASK_24BIT);
        dataHead = dataHead->next;
    }

    fclose(objectFile); /* close the file */
}

/* creates / wrights to the external file (if needed) */
void writeExternalFile(char *filename, ExternalWordNode *head)
{
    FILE *externalFile;

    /* if the external list is empty, dont create the file */
    if (head == NULL)
    {
        return;
    }

    externalFile = openExternFile(filename); /* open / create the file */

    if (externalFile == NULL)
    {
        return; /* an error was found opening the file (already printed) */
    }

    while (head != NULL)
    {
        fprintf(externalFile, "%s %07d\n", head->symbol, head->value);
        head = head->next;
    }

    fclose(externalFile); /* close the file */
}

/* creates / wrights to the entry file (if needed) */
void writeEntryFile(char *filename, SymbolNode *head)
{
    FILE *entryFile = NULL; /* initialize the object file */

    while (head != NULL)
    {
        /* only print the entry symbols */
        if (head->isEntry)
        {
            /* if its the first entry, create the file (avoid creating when uneccesary) */
            if (entryFile == NULL)
            {
                entryFile = openEntryFile(filename);
                if (entryFile == NULL)
                {
                    return; /* an error was found opening the file (already printed) */
                }
            }

            fprintf(entryFile, "%s %07d\n", head->symbol, head->value);
        }
        head = head->next;
    }

    if (entryFile != NULL)
    {
        fclose(entryFile); /* close the file */
    }
}