#include "header.h"
#include "assemble.h"

/* assisting functions for the second transition: */

/* function that skips the label in line */
void skipLabel(char **line)
{
    char *colon = (char *)strchr(*line, COLON); /* find the colon position */
    if (colon != NULL)
    {
        /* if a colon was found, skip after it */
        *line = colon + 1;

        skipWhiteSpaces(line); /* skip white spaces as well */
    }
}

/* function that skips an operand */
void skipOperand(char **line)
{
    while (!isspace(**line) && **line != COMMA)
    {
        (*line)++;
    }

    skipWhiteSpaces(line); /* skip the whitespaces */

    if (**line == COMMA)
    {
        (*line)++; /* skip the commma if exists */
    }

    skipWhiteSpaces(line); /* skip remaining whitespaces */
}

/* function that adds the entry flag to an entry symbol */
int addEntryFlag(SymbolNode *head, char *symbolName, int lineNum)
{
    SymbolNode *current = head;

    /* loop through the nodes and check for this symbolname */
    while (current != NULL)
    {
        if (strcmp(current->symbol, symbolName) == 0)
        {
            /* found symbol */

            /* ensure its not type external */
            if (strcmp(current->type, TYPE_EXTERNAL) == 0)
            {
                printErrorInLine(EXTERNAL_AND_ENTRY_ERROR, lineNum, symbolName);
                return SYNTAX_ERROR;
            }

            current->isEntry = TRUE;
            return NO_ERROR;
        }

        current = current->next;
    }

    /* no symbol was found - print the error message */
    printErrorInLine(LABEL_DOESNT_EXIST_ERROR, lineNum, symbolName);
    return SYNTAX_ERROR;
}

/* function that fills the code left to be coden from the first transition (specific for relative addressing) */
void fillRelativeAddressingCode(MemoryNode *instructionNode, SymbolNode *symbol, int originalValue)
{
    int code;

    /* get the value difference */
    int valueDifference = symbol->value - originalValue;

    /* code the symbol distance from its declaration (also A is on, R and E are off) */
    code = valueDifference << (A_POS + 1); /* the 3-23 bits */
    code |= 1 << A_POS;

    instructionNode->code = code; /* update the code */
}

/* function that fills the code left to be coden from the first transition (specific for direct addressing) */
void fillDirectAddressingCode(MemoryNode *instructionNode, SymbolNode *symbol)
{
    int code;

    /* code the symbol value */
    code = symbol->value << (A_POS + 1); /* the 3-23 bits */

    /* code the ARE (either E or R is on.) */
    if (strcmp(symbol->type, TYPE_EXTERNAL) == 0)
    {
        code |= (1 << E_POS); /* E on, R and A off */
    }
    else
    {
        code |= (1 << R_POS); /* R on, E and A off */
    }

    instructionNode->code = code; /* update the code */
}

/* function that gets the operand symbol node */
SymbolNode *getOperandSymbol(char **line, SymbolNode *head, int lineNum)
{
    char *start;    /* initialize the start of the symbol */
    int length = 0; /* initialize the length */

    SymbolNode *current;

    start = *line; /* get the start of the symbol name */

    while (**line != NULL_TERMINATOR && !isspace(**line) && **line != COMMA)
    {
        (*line)++; /* move to the next character */
        length++;  /* increment length */
    }

    current = head;

    while (current)
    {
        if (strncmp(current->symbol, start, length) == 0)
        {
            /* found a symbol */

            /* skip already to the next word (error messages should be handled in the first transition) */

            skipWhiteSpaces(line);

            while (**line == COMMA)
            {
                (*line)++;
                skipWhiteSpaces(line);
            }

            skipWhiteSpaces(line);

            return current;
        }
        current = current->next;
    }

    /* the symbol is not defined, print an error */
    printErrorInLine(SYMBOL_DOESNT_EXIST_ERROR, lineNum, length, start);

    return NULL; /* indicate that the symbol doesnt exist */
}

/* function that adds a symbol to the external symbol list */
int addToExternalList(ExternalWordNode **head, char *symbol, int address)
{
    /* allocate memory for the symbol */
    ExternalWordNode *newNode = (ExternalWordNode *)malloc(sizeof(ExternalWordNode));

    if (!newNode)
    {
        return MEMORY_ERROR; /* memory allocation failed */
    }

    /* copy the values */
    strcpy(newNode->symbol, symbol);
    newNode->value = address;

    newNode->next = *head;
    *head = newNode;

    return NO_ERROR;
}
