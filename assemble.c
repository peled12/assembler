#include "header.h"
#include "assemble.h"

/* define IC and DC */
unsigned int IC;
unsigned int DC;

/* assembles the code, calls the first and second transitions */
int assemble(FILE *fp, char *filename)
{
    int isError; /* initialize the error flag */

    /* initialize ICF and DCF */
    unsigned int ICF;
    unsigned int DCF;

    SymbolNode *symbolHead = NULL;             /* initialize the head of the symbol table */
    ExternalWordNode *externalWordHead = NULL; /* initialize the head of the external word table */
    MemoryQueue *instructionQueue;             /* initialize the instruction queue */

    /* create the memory queues */
    MemoryQueue *dataQueue = initializeMemoryQueue();

    if (dataQueue == NULL)
    {
        /* there were a memory error */
        handleMemoryError();
    }

    instructionQueue = initializeMemoryQueue();

    if (instructionQueue == NULL)
    {
        /* there was a memory error */
        free(dataQueue); /* free the already allocated memory */
        handleMemoryError();
    }

    /* initialize the IC and DC values */
    IC = INITIAL_IC;
    DC = INITIAL_DC;

    isError = firstTransition(fp, &symbolHead, dataQueue, instructionQueue, &ICF, &DCF);

    if (isError == MEMORY_OVERFLOW)
    {
        freeMemoryList(instructionQueue);
        freeMemoryList(dataQueue);
        freeExternalWordList(&externalWordHead);
        freeSymbolList(&symbolHead);
        return TRUE; /* if there was a memory overflow, skip to the next file (true means an error was found) */
    }

    /* check if there was an error on the seconds transition */
    if (secondTransition(fp, symbolHead, instructionQueue, dataQueue, &externalWordHead))
    {
        isError = TRUE; /* set the error flag */
    };

    if (!isError)
    {
        /* ensure it didn't exceed the memory size */
        if (ICF + DCF > MAX_MEMORY_SIZE)
        {
            printError(MEMORY_OVERFLOW_ERROR);
            isError = TRUE; /* set the error flag */
        }
        else
        {
            /* create the files */
            writeObjectFile(ICF, DCF, filename, instructionQueue->head, dataQueue->head); /* write the object file */
            writeExternalFile(filename, externalWordHead);                                /* write the external file (if needed) */
            writeEntryFile(filename, symbolHead);                                         /* write the entry file (if needed) */
        }
    }

    /* free the data */
    freeMemoryList(instructionQueue);
    freeMemoryList(dataQueue);
    freeExternalWordList(&externalWordHead);
    freeSymbolList(&symbolHead);

    return isError;
}

/* the first transition of the assembler - here is when you get the entire symbol table,
   start coding the codable words and check the findable errors */
int firstTransition(FILE *fp, SymbolNode **symbolHead, MemoryQueue *dataQueue, MemoryQueue *instructionQueue, unsigned int *ICF, unsigned int *DCF)
{
    char line[MAX_LINE_LENGTH]; /* intialize the line buffer */
    int lineNum = 0;            /* initialize the line number */

    int isError = FALSE; /* initialize the error flag */

    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
    {
        char *currentLine = line;    /* initialize a pointer to the current character in line */
        int isEntryDirective;        /* initialize a helper flag */
        int isExternDirective;       /* initialize a helper flag */
        int dataOrString;            /* initialize a helper flag */
        int handleInstructionResult; /* initialize the handleInstruction function result */

        /* initialize the label (+1 for null-trminator)*/
        char label[MAX_SYMBOL_LENGTH + 1] = EMPTY_STRING;

        lineNum++; /* increment the line number */

        /* skip leading white spaces */
        skipWhiteSpaces(&currentLine);

        /* ensure its not an empty line or a comment line */
        if (*currentLine == NULL_TERMINATOR || *currentLine == NEW_LINE)
        {
            continue;
        }

        /* check if the line is a label */
        if ((getLabel(&currentLine, label, lineNum)) == SYNTAX_ERROR)
        {
            isError = TRUE; /* set the error flag */
            continue;
        }

        /* check if its a directive */
        if (*currentLine == DOT)
        {
            int i;

            if ((dataOrString = isDataOrString(&currentLine)))
            {
                /* check if there is a label */
                if (strcmp(label, EMPTY_STRING) != 0)
                {
                    /* if so, ensure its not defined already */
                    if (isSymbolDefined(*symbolHead, label))
                    {
                        printErrorInLine(SYMBOL_ALREADY_DEFINED, lineNum, label);
                        isError = TRUE; /* set the error flag */
                        continue;
                    }
                    else
                    {
                        /* if the label wasn't defined, add the label to the symbol table */

                        if ((addToSymbolTable(symbolHead, label, DC, TYPE_DATA)) == MEMORY_ERROR)
                        {
                            customMemoryErrorHandler(fp, symbolHead, instructionQueue, dataQueue, NULL);
                        }
                    }
                }

                if (dataOrString == DATA_FOUND)
                {
                    /* this function also handles syntax errors */
                    int codeDataResult = codeData(dataQueue, &currentLine, lineNum);

                    switch (codeDataResult)
                    {
                    case MEMORY_ERROR:
                        customMemoryErrorHandler(fp, symbolHead, instructionQueue, dataQueue, NULL);
                    case SYNTAX_ERROR:
                        isError = TRUE; /* set the error flag */
                        break;
                    case MEMORY_OVERFLOW:
                        printErrorInLine(MEMORY_OVERFLOW_ERROR, lineNum);
                        return MEMORY_OVERFLOW; /* return the error code to be handled in the assemble function */
                    }
                }
                else
                {
                    /* this function also handles syntax errors */
                    int codeStringResult = codeString(dataQueue, &currentLine, lineNum);

                    switch (codeStringResult)
                    {
                    case MEMORY_ERROR:
                        customMemoryErrorHandler(fp, symbolHead, instructionQueue, dataQueue, NULL);
                    case SYNTAX_ERROR:
                        isError = TRUE; /* set the error flag */
                        break;
                    case MEMORY_OVERFLOW:
                        printErrorInLine(MEMORY_OVERFLOW_ERROR, lineNum);
                        return MEMORY_OVERFLOW; /* return the error code to be handled in the assemble function */
                    }
                }

                continue;
            }

            isEntryDirective = isEntry(&currentLine);
            isExternDirective = isExtern(&currentLine);

            if (isEntryDirective || isExternDirective)
            {
                /* initialize the operand label (+1 for null-terminator) */
                char operandLabel[MAX_SYMBOL_LENGTH + 1];

                if (isEntryDirective)
                {
                    /* entry would be addressed in the second transition */
                    continue;
                }

                /* get operand label for extern */
                if ((getOperandLabel(&currentLine, operandLabel, lineNum)) != NO_ERROR)
                {
                    isError = TRUE; /* set the error flag if an error was found */
                    continue;
                }

                /* ensure the symbol doesnt already exist */
                if (isSymbolDefined(*symbolHead, operandLabel))
                {
                    printErrorInLine(SYMBOL_ALREADY_EXISTS_ERROR, lineNum, operandLabel);
                    isError = TRUE;
                    continue;
                }

                /* insert external symbols with the value 0 */
                if ((addToSymbolTable(symbolHead, operandLabel, 0, TYPE_EXTERNAL)) == MEMORY_ERROR)
                {
                    customMemoryErrorHandler(fp, symbolHead, instructionQueue, dataQueue, NULL);
                }

                continue;
            }

            /* if reached here its an invalid directive name */

            i = 0; /* intialize the length of the word */
            while (!isspace(currentLine[i]))
            {
                i++;
            }

            printErrorInLine(INVALID_DIRECTIVE_NAME, lineNum, i, currentLine);
            isError = TRUE;

            continue;
        }

        /* if reached here, its an instruction line */

        /* if there is a label, insert it to the symbol table */
        if (strcmp(label, EMPTY_STRING) != 0)
        {
            if (isSymbolDefined(*symbolHead, label))
            {
                printErrorInLine(SYMBOL_ALREADY_DEFINED, lineNum, label);
                isError = TRUE; /* set the error flag */
                continue;
            }
            else
            {
                /* add to the symbol table */

                if ((addToSymbolTable(symbolHead, label, IC, TYPE_CODE)) == MEMORY_ERROR)
                {
                    customMemoryErrorHandler(fp, symbolHead, instructionQueue, dataQueue, NULL);
                }
            }
        }

        /* handle the instruction */
        handleInstructionResult = handleInstruction(&currentLine, instructionQueue, lineNum);

        switch (handleInstructionResult)
        {
        case MEMORY_ERROR:
            customMemoryErrorHandler(fp, symbolHead, instructionQueue, dataQueue, NULL);
        case SYNTAX_ERROR:
            isError = TRUE; /* set the error flag */
            break;
        case MEMORY_OVERFLOW:
            printErrorInLine(MEMORY_OVERFLOW_ERROR, lineNum);
            return MEMORY_OVERFLOW; /* return the error code to be handled in the assemble function */
        }
    }

    /* save IC and DC to ICF and DCF (will be used to create output files) */
    *ICF = IC;
    *DCF = DC;

    /* update every data symbol's value by adding ICF */
    updateDataSymbols(symbolHead, *ICF);

    rewind(fp); /* rewind the file to the beginning */

    return isError;
}

/* this is the second transition, it codes the remaning words and checks for the remaining errors */
int secondTransition(FILE *fp, SymbolNode *symbolHead, MemoryQueue *instructionQueue, MemoryQueue *dataQueue, ExternalWordNode **externalWordHead)
{
    char line[MAX_LINE_LENGTH]; /* intialize the line buffer */
    int lineNum = 0;            /* initialize the line number */

    int isError = FALSE; /* initialize the isError flag */

    MemoryNode *currentInstructionNode = instructionQueue->head; /* initialize the current instruction node */
    int originalValue;                                           /* initialize the value of the first word in an insturcion */
    int originalWordsAmount;                                     /* intialize the amount of words in an instruction line */

    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
    {
        char *currentLine = line; /* initialize a pointer to the current character in line */

        int i; /* initialize an interator */

        lineNum++; /* increment the line number */

        /* skip leading white spaces */
        skipWhiteSpaces(&currentLine);

        /* ensure its not an empty line */
        if (*currentLine == NULL_TERMINATOR || *currentLine == NEW_LINE)
        {
            continue;
        }

        /* skip the label if exists (proccessed in the first transition) */
        skipLabel(&currentLine);

        /* check if its a directive */
        if (*currentLine == DOT)
        {
            /* check if its a .entry */
            if (isEntry(&currentLine))
            {
                /* initialize the operand label (+1 for null-terminator) */
                char operandLabel[MAX_SYMBOL_LENGTH + 1] = EMPTY_STRING;

                if ((getOperandLabel(&currentLine, operandLabel, lineNum)) == NO_ERROR)
                {
                    /* set the entry flag of that label */
                    if ((addEntryFlag(symbolHead, operandLabel, lineNum)) == SYNTAX_ERROR)
                    {
                        /* error message was already printed in the function */
                        isError = TRUE; /* set the error flag */
                    }
                }
                else
                {
                    isError = TRUE; /* error in the operand label */
                }
            }

            continue; /* the rest directive have already been proccessed */
        }

        /* ensure its actually an instruction name (and not an error printed in the first transition) */
        if (!(isStartWithInstruction(&currentLine)))
        {
            isError = TRUE;
            continue;
        }

        /* ensure the instruction node actually exists and an error havn't occured in the first transition */
        if (currentInstructionNode == NULL)
        {
            isError = TRUE;
            continue;
        }

        /* if the first operand is a register, skip it to ensure to fill the code to of the second operand (if needed) */
        if (((currentInstructionNode->code >> SOURCE_ADDRESSING_POS) & REGISTER_ADDRESSING_MASK) == REGISTER_ADDRESSING_MASK)
        {
            skipOperand(&currentLine);
        }

        /* fill the binary code */

        originalValue = currentInstructionNode->value;             /* get the first value */
        originalWordsAmount = currentInstructionNode->wordsAmount; /* get the amount of words */

        for (i = 0; i < originalWordsAmount && currentInstructionNode != NULL; i++, currentInstructionNode = currentInstructionNode->next)
        {
            if (currentInstructionNode->code == CODE_FILLER)
            {
                int isRelativeAddressing = *currentLine == AMPERSAND; /* flag if the symbol is used with relative addressing */
                SymbolNode *symbol;                                   /* initialize the symbol */

                if (isRelativeAddressing)
                {
                    /* skip the ampersand */
                    currentLine++;
                }

                /* get the symbol */
                symbol = getOperandSymbol(&currentLine, symbolHead, lineNum);
                if (symbol == NULL)
                {
                    /* the symbol is not defined (error printed in the function) */
                    isError = TRUE;
                    continue;
                }

                /* its either relative or direct addressing method */
                if (isRelativeAddressing)
                {
                    /* relative addressing method */
                    fillRelativeAddressingCode(currentInstructionNode, symbol, originalValue); /* fill the code */
                }
                else
                {
                    /* direct addressing method */
                    fillDirectAddressingCode(currentInstructionNode, symbol); /* fill the code */
                }

                /* check if the symbol is type extern */
                if (strcmp(symbol->type, TYPE_EXTERNAL) == 0)
                {
                    /* add the address to the list of external words */
                    if ((addToExternalList(externalWordHead, symbol->symbol, currentInstructionNode->value)) == MEMORY_ERROR)
                    {
                        customMemoryErrorHandler(fp, &symbolHead, instructionQueue, dataQueue, externalWordHead);
                    }
                }
            }
        }
    }

    return isError;
}
