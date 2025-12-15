#include "header.h"
#include "assemble.h"

/* general assisting functions for the assembler: */

/* function that gets the operand label (e.g. .entry LABEL) */
int getOperandLabel(char **line, char *operandLabel, int lineNum)
{
    /* init variables */
    int length;
    char *end = *line;

    /* move to the next space or null-terminator */
    while (*end != NULL_TERMINATOR && !isspace(*end))
    {
        end++;
    }

    length = end - *line;

    /* ensure the name of the label is valid */
    if ((checkLabelName(*line, length, lineNum)) == SYNTAX_ERROR)
    {
        /* the error message would be printed in the function */
        return SYNTAX_ERROR;
    }

    /* get the operand */
    strncpy(operandLabel, *line, length);
    operandLabel[length] = NULL_TERMINATOR;

    *line += length; /* skip after the label */

    /* ensure no extranous characters after label declaration */
    skipWhiteSpaces(line);
    if (**line != NULL_TERMINATOR)
    {
        printErrorInLine(EXTRA_TEXT_AFTER_LABEL_ERROR, lineNum);
        return SYNTAX_ERROR;
    }

    return NO_ERROR; /* no errors found */
}

/* function that checks if a directive is .entry */
int isEntry(char **line)
{
    char *temp;
    int entryDirectiveLength = strlen(ENTRY_DIRECTIVE);

    if (strncmp(*line, ENTRY_DIRECTIVE, entryDirectiveLength) == 0)
    {
        temp = *line + entryDirectiveLength;

        /* ensure no extra characters before the next whitespace */
        if (isspace(*temp))
        {
            *line = temp;
            skipWhiteSpaces(line);
            return TRUE;
        }
    }
    return FALSE;
}

/* function that ensures the label starts with a character and contains only characters and numbers */
int checkLabelName(char *start, int length, int lineNum)
{
    int i; /* initialize the iterator */

    /* check for an empty label */
    if (length == 0)
    {
        /* empty label */
        printErrorInLine(EMPTY_LABEL_ERROR, lineNum);
        return SYNTAX_ERROR;
    }

    /* check for overflow */
    if (length > MAX_SYMBOL_LENGTH)
    {
        /* label too long */
        printErrorInLine(LABEL_TOO_LONG_ERROR, lineNum, MAX_SYMBOL_LENGTH);
        return SYNTAX_ERROR;
    }

    /* ensure the first character is a a letter */
    if (!isalpha(start[0]))
    {
        printErrorInLine(LABEL_NOT_START_WITH_LETTER_ERROR, lineNum, length, start);
        return SYNTAX_ERROR;
    }

    /* ensure all characters are digits or numbers */
    for (i = 1; i < length; i++)
    {
        if (!isalnum(start[i]))
        {
            printErrorInLine(INVALID_CHARACTER_IN_LABEL_ERROR, lineNum, length, start);
            return SYNTAX_ERROR;
        }
    }

    /* ensure its not an instruction name */
    if (isInstruction(start, length))
    {
        printErrorInLine(LABEL_IS_INSTRUCTION_ERROR, lineNum, length, start);
        return SYNTAX_ERROR;
    }

    /* ensure its not a register name */
    if (IS_REGISTER(start, length))
    {
        printErrorInLine(LABEL_IS_REGISTER_ERROR, lineNum, length, start);
        return SYNTAX_ERROR;
    }

    return NO_ERROR; /* no error was found */
}

/* function that frees the symbol list */
void freeSymbolList(SymbolNode **head)
{
    SymbolNode *current = *head;
    while (current)
    {
        SymbolNode *temp = current;
        current = current->next;

        free(temp); /* free the node */
    }
}

/* function that frees a memory list from the tail */
void freeMemoryList(MemoryQueue *queue)
{
    MemoryNode *current = queue->head;
    MemoryNode *temp;

    /* free each node */
    while (current != NULL)
    {
        temp = current;
        current = current->next;
        free(temp); /* free the node */
    }

    /* finally, free the queue itself */
    free(queue);
}

/* function that frees the external word list */
void freeExternalWordList(ExternalWordNode **head)
{
    ExternalWordNode *current = *head;
    while (current)
    {
        ExternalWordNode *temp = current;
        current = current->next;

        free(temp); /* free the node */
    }
}

/* function that initializes a memory queue */
MemoryQueue *initializeMemoryQueue()
{
    MemoryQueue *queue = (MemoryQueue *)malloc(sizeof(MemoryQueue));
    if (queue == NULL)
    {
        return NULL;
    }
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

/* function that changes a pointer to point to the first non-white-space character */
void skipWhiteSpaces(char **line)
{
    while (isspace(**line))
    {
        (*line)++;
    }
}

/* a function that frees all the data before throwing a memory error */
void customMemoryErrorHandler(FILE *fp, SymbolNode **symbolHead, MemoryQueue *instructionQueue, MemoryQueue *dataQueue, ExternalWordNode **externalWordHead)
{
    freeSymbolList(symbolHead);

    freeMemoryList(instructionQueue);
    freeMemoryList(dataQueue);

    freeExternalWordList(externalWordHead);

    fclose(fp); /* close the opened file */

    handleMemoryError();
}