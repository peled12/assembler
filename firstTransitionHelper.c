#include "header.h"
#include "assemble.h"
#include "firstTransitionHeader.h"

/* assisting functions for the first transition: */

/* function that gets the label name (e.g: LABEL: .data 5 gets LABEL) */
int getLabel(char **line, char *label, int lineNum)
{
    int labelLength;
    char *colon = (char *)strchr(*line, COLON);
    if (colon == NULL)
    {
        return NO_ERROR; /* no label found */
    }

    labelLength = colon - *line; /* get the label length */

    /* ensure the name of the label is valid */
    if ((checkLabelName(*line, labelLength, lineNum)) == SYNTAX_ERROR)
    {
        /* the error message would be printed in the function */
        return SYNTAX_ERROR;
    }

    /* ensure there is a whitespace after the colon */
    if (!isspace(*(colon + 1)))
    {
        printErrorInLine(MISSING_WHITE_SPACE_AFTER_COLON, lineNum);
        return SYNTAX_ERROR;
    }

    /* copy the label to the label buffer */
    strncpy(label, *line, labelLength);
    label[labelLength] = NULL_TERMINATOR;

    *line = colon + 1; /* move the line pointer to the next character after the colon */

    skipWhiteSpaces(line); /* skip the white spaces after the colon */

    return NO_ERROR; /* found label; no error */
}

/* function that parses a number in data directive */
int parseNumberInData(char **line, int *result)
{
    int number = 0;         /* init the searched number */
    int isNegative = FALSE; /* init the isNegative flag */

    if (**line == NULL_TERMINATOR || isspace(**line))
    {
        return MISSING_NUMBER;
    }

    if (**line == NEGATIVE_SIGN || **line == POSITIVE_SIGN)
    {
        if (**line == NEGATIVE_SIGN)
        {
            isNegative = TRUE; /* a negative number */
        }
        (*line)++; /* move to the next character */
    }

    /* ensure the line is only folowed by digits until next whitespace or comma */
    while (!isspace(**line) && **line != COMMA && **line != NULL_TERMINATOR)
    {
        if (!isdigit(**line))
        {
            /* not a number */
            return INVALID_CHARACTER;
        }
        else
        {
            /* valid digit */
            int digit = **line - ZERO_CHAR;
            number = number * 10 + digit; /* adjust the number */
        }

        (*line)++; /* move to the next character */
    }

    /* get the final number */
    if (isNegative)
    {
        number = -number; /* change to negative */
    }

    *result = number; /* change the result */

    return NO_ERROR; /* indicate no error */
}

/* function that codes the data after a data directive */
int codeData(MemoryQueue *queue, char **line, int lineNum)
{
    /* data should display like this format: n, n, n.... */

    int commaFound = TRUE; /* initialize the comma found flag */
    int addToMemoryResult; /* initialize the add to memory result */

    /* ensure there are indeed arguments */
    if (**line == NULL_TERMINATOR)
    {
        printErrorInLine(NO_DATA_ARGS_ERROR, lineNum);
        return SYNTAX_ERROR;
    }

    while (**line != NULL_TERMINATOR)
    {
        skipWhiteSpaces(line); /* skip the white spaces */

        /* ensure its not the end of the line now */
        if (**line == NULL_TERMINATOR)
        {
            break;
        }

        if (**line != COMMA && commaFound)
        {
            /* valid place to look for a number */

            /* extract the number */
            if (isdigit(**line) || **line == POSITIVE_SIGN || **line == NEGATIVE_SIGN)
            {
                int number;

                int parsedNumberResult = parseNumberInData(line, &number);

                /* check for possible errors */
                if (parsedNumberResult == INVALID_CHARACTER)
                {
                    printErrorInLine(FOUND_ILLEGAL_NUMBER, lineNum);
                    return SYNTAX_ERROR;
                }
                else if (parsedNumberResult == MISSING_NUMBER)
                {
                    printErrorInLine(FOUND_ILLEGAL_COMMA, lineNum);
                    return SYNTAX_ERROR;
                }

                /* ensure the number contains less than 24 bits */
                if (IS_LARGER_THAN_24_BITS(number))
                {
                    printErrorInLine(NUMBER_TOO_BIG, lineNum);
                    return SYNTAX_ERROR;
                }

                /* add to memory table */
                if ((addToMemoryResult = (handleAddToMemoryTable(queue, DC, number))) != NO_ERROR)
                {
                    return addToMemoryResult;
                }

                DC++; /* increment DC */

                commaFound = FALSE;
            }
            else
            {
                /* the value is not a valid number */
                printErrorInLine(FOUND_ILLEGAL_NUMBER, lineNum);
                return SYNTAX_ERROR;
            }
        }
        else
        {
            if (**line == COMMA)
            {
                if (commaFound)
                {
                    /* an illegal comma was found */
                    printErrorInLine(FOUND_ILLEGAL_COMMA, lineNum);
                    return SYNTAX_ERROR;
                }

                commaFound = TRUE; /*  set the coma flag */
                (*line)++;         /* move past the comma */
            }
            else
            {
                /* there is a missing comma */
                printErrorInLine(MISSING_COMMA_ERROR, lineNum);
                return SYNTAX_ERROR;
            }
        }
    }

    /* if there is a comma at the end */
    if (commaFound)
    {
        printErrorInLine(FOUND_ILLEGAL_COMMA, lineNum);
        return SYNTAX_ERROR;
    }

    return NO_ERROR; /* no error have occured */
}

/* function that codes the string after a string directive */
int codeString(MemoryQueue *queue, char **line, int lineNum)
{
    int stringLength;
    int i;
    int addToMemoryResult;

    if (**line != QUOTE_MARK)
    {
        printErrorInLine(CHAR_OUT_QUOTES, lineNum);
        return SYNTAX_ERROR;
    }

    (*line)++; /* move past the first quote mark */

    /* ensure the string ends with a quote mark */

    /* ignore trailing whitespaces */
    stringLength = strlen(*line) - 1;
    while (stringLength >= 0 && isspace((*line)[stringLength]))
    {
        stringLength--;
    }

    /* check if the last non-whitespace character is a quote */
    if (stringLength < 0 || (*line)[stringLength] != QUOTE_MARK)
    {
        printErrorInLine(CHAR_OUT_QUOTES, lineNum);
        return SYNTAX_ERROR;
    }

    for (i = 0; i < stringLength; i++, (*line)++)
    {
        if ((addToMemoryResult = (handleAddToMemoryTable(queue, DC, **line))) != NO_ERROR)
        {
            return addToMemoryResult;
        }
        DC++; /* increment DC */
    }

    skipWhiteSpaces(line);

    /* ensure there are no more characters */
    if (**line != QUOTE_MARK)
    {
        /* reached the end of the line without the ending quote mark */
        printErrorInLine(CHAR_OUT_QUOTES, lineNum);
        return SYNTAX_ERROR;
    }

    /* code the null terminator */
    if ((addToMemoryResult = (handleAddToMemoryTable(queue, DC, NULL_TERMINATOR))) != NO_ERROR)
    {
        return addToMemoryResult;
    }
    DC++; /* increment DC */

    return NO_ERROR; /* no error have occured */
}

/* function that checks if a symbol was already defined */
int isSymbolDefined(SymbolNode *head, char *symbol)
{
    SymbolNode *current = head;
    while (current)
    {

        if (strcmp(current->symbol, symbol) == 0)
        {
            return TRUE;
        }
        current = current->next;
    }

    return FALSE;
}

/* function that checks if a directive is either data or string */
int isDataOrString(char **line)
{
    char *temp;
    int dataDirectiveLength = strlen(DATA_DIRECTIVE);
    int stringDirectiveLength = strlen(STRING_DIRECTIVE);
    int found = FALSE; /* hold which directive was found (or not) */

    if (strncmp(*line, DATA_DIRECTIVE, dataDirectiveLength) == 0)
    {
        temp = *line + dataDirectiveLength;
        found = DATA_FOUND; /* found data */
    }

    if (strncmp(*line, STRING_DIRECTIVE, stringDirectiveLength) == 0)
    {
        temp = *line + stringDirectiveLength;
        found = STRING_FOUND; /* found string */
    }

    if (!found)
    {
        return FALSE; /* not a directive */
    }

    /* ensure no extra characters before the next whitespace */
    if (!isspace(*temp))
    {
        return FALSE; /* not a directive (more characters after) */
    }

    *line = temp; /* update the line pointer to after the label */

    skipWhiteSpaces(line); /* is a directive; skip the white spaces */

    return found;
}

/* function that checks if a directive is extern */
int isExtern(char **line)
{
    char *temp;
    int externDirectiveLength = strlen(EXTERN_DIRECTIVE);

    if (strncmp(*line, EXTERN_DIRECTIVE, externDirectiveLength) == 0)
    {
        temp = *line + externDirectiveLength;

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

/* function that adds a symbol to the symbol table */
int addToSymbolTable(SymbolNode **head, char *symbol, int value, char *type)
{
    /* allocate memory for the symbol */
    SymbolNode *newNode = (SymbolNode *)malloc(sizeof(SymbolNode));

    if (!newNode)
    {
        return MEMORY_ERROR; /* memory allocation failed */
    }

    newNode->value = value;

    strncpy(newNode->symbol, symbol, MAX_SYMBOL_LENGTH);
    newNode->symbol[MAX_SYMBOL_LENGTH] = NULL_TERMINATOR;

    strncpy(newNode->type, type, MAX_TYPE_LENGTH);
    newNode->type[MAX_TYPE_LENGTH] = NULL_TERMINATOR;

    newNode->isEntry = FALSE; /* will be updated in the second transition if neccessary */

    newNode->next = *head;

    *head = newNode; /* assign the new node as the head */

    return NO_ERROR;
}

/* function that adds a node to the memory table (queue). the nodes contains given value and code */
MemoryNode *addToMemoryTable(MemoryQueue *queue, int value, int code)
{
    MemoryNode *newNode = malloc(sizeof(MemoryNode));
    if (newNode == NULL)
    {
        return NULL; /* indicate a memory allocation error */
    }

    /* define the new node's properties */
    newNode->code = code;
    newNode->value = value;
    newNode->wordsAmount = 0; /* initialize as 0 */
    newNode->next = NULL;

    /* if the queue is empty, set both head and tail */
    if (queue->tail == NULL)
    {
        queue->head = queue->tail = newNode;
    }
    else
    {
        /* simply add to the tail */
        queue->tail->next = newNode;
        queue->tail = newNode;
    }

    return newNode; /* added successfully, return the new node */
}

/* function that adds the ICF to the value of each symbol that contains data, to correcten their address */
void updateDataSymbols(SymbolNode **head, int ICF)
{
    SymbolNode *current = *head;

    while (current != NULL)
    {
        /* check if the symbol is type data */
        if (strcmp(current->type, TYPE_DATA) == 0)
        {
            current->value += ICF; /* add ICF to the value */
        }

        current = current->next;
    }
}

/* function that handles the result of adding to the memory table */
int handleAddToMemoryTable(MemoryQueue *queue, int value, int code)
{
    MemoryNode *newNode;

    if ((newNode = addToMemoryTable(queue, value, code)) == NULL)
    {
        return MEMORY_ERROR; /* indicate a memory error */
    }

    /* check if the max memory sized has been reached (+1 for the new node) */
    if (IC + DC + 1 > MAX_MEMORY_SIZE)
    {
        return MEMORY_OVERFLOW;
    }

    return NO_ERROR;
}
