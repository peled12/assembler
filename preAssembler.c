#include "header.h"
#include "preAssembler.h"

/* the pre assembler */
int preAssembler(FILE *fp, FILE *outputFilePointer)
{
    MacroNode *macroHead = NULL;   /* initialize macro head */
    MacroNode **head = &macroHead; /* pointer to the macro head */

    char line[MAX_LINE_LENGTH + 1];        /* intialize the line buffer (+1 for \n) */
    int onMcro = FALSE;                    /* flag if a macro is on */
    char macroName[MAX_MACRO_NAME_LENGTH]; /* initialize a macro name */
    char *macroDefinition = NULL;          /* initialize a macro definition */
    int macroDefSize = INITIAL_DEF_SIZE;   /* initialize the size of the macro definition */
    int isError = FALSE;                   /* initialize the error flag */
    int lineNum = 0;                       /* initialize the line number */

    while (fgets(line, MAX_LINE_LENGTH + 1, fp) != NULL) /* +1 to check for overflowing lines */
    {
        /* initialize the macro call */
        char *macroCall = NULL;
        MacroNode *currentMacro = *head;
        int lineLength = strlen(line);

        char *currentLine = line; /* initialize a pointer to the current character in line */

        int macroStartResult;

        lineNum++; /* increment the line number */

        /* if its a comment line, skip it */
        if (*currentLine == COMMENT_CHAR)
        {
            continue;
        }

        /* lines cant be over the length of 80. check that */
        if (lineLength > MAX_LINE_LENGTH)
        {
            printErrorInLine(LINE_TOO_BIG_ERROR, lineNum, MAX_LINE_LENGTH);
            isError = TRUE; /* set the error flag */
        }

        skipWhiteSpaces(&currentLine); /* skip leading white spaces in a line */

        /* check if the line starts with a macro definition */
        macroStartResult = isMacroStart(currentLine);

        if (macroStartResult == NO_ERROR)
        {
            /* find the position of where the macro name starts */
            char *macroStart = currentLine + MACRO_START_LENGTH;
            int macroNameLength; /* initialize the length of a macro name */

            /* skip white spaces after mcro */
            skipWhiteSpaces(&macroStart);

            /* calculate the length of a macro name */
            macroNameLength = strcspn(macroStart, WHITE_SPACES);

            /* check if the macro name is too big */
            if (macroNameLength > MAX_MACRO_NAME_LENGTH)
            {
                printErrorInLine(MACRO_NAME_TOO_BIG_ERROR, lineNum, MAX_MACRO_NAME_LENGTH);
                isError = TRUE; /* set the error flag */
            }

            /* get the macro name */
            strcpy(macroName, macroStart);
            macroName[macroNameLength] = NULL_TERMINATOR; /* remove the new line */

            onMcro = TRUE; /* set the onMcro flag to true */

            /* check if the macro name is an instruction name (the function also prints the error) */
            if (!isValidMacroName(macroName, lineNum))
            {
                /* printing the error was already handled */
                isError = TRUE; /* set the error flag */
            }

            continue;
        }
        else if (macroStartResult == EXTRANOUS_CHARACTERS)
        {
            printErrorInLine(EXTRA_MACRO_START_CHARS_ERROR, lineNum);
            isError = TRUE;
        }

        /* inside a macro, collect its definition */
        if (onMcro)
        {
            int macroEndResult = isMacroEnd(currentLine);

            if (macroEndResult == NO_ERROR)
            {
                onMcro = FALSE; /* set the flag to false */
                macroDefSize = INITIAL_DEF_SIZE /* reset the size of the macro definition */;

                if (addMacro(head, macroName, macroDefinition) == FALSE) /* add the macro to the list */
                {
                    /* a memory allocation error was found */
                    customHandleMemoryError(fp, head);
                }

                /* reset the macro definition and macro name after adding the macro */
                free(macroDefinition);
                macroDefinition = NULL;
                strcpy(macroName, EMPTY_STRING);
            }
            else if (macroEndResult == EXTRANOUS_CHARACTERS)
            {
                printErrorInLine(EXTRA_MACRO_END_CHARS_ERROR, lineNum);
                isError = TRUE;
            }
            else
            {
                /* add the current line to macro definition (reallocate memory) */

                if (!macroDefinition)
                {
                    /* initialize the macro definition */
                    macroDefinition = malloc(lineLength + 1);
                    if (!macroDefinition)
                    {
                        customHandleMemoryError(fp, head);
                    }

                    strcpy(macroDefinition, line); /* add the line to the macro definition */
                    macroDefSize = lineLength;     /* update the size of the macro definition */
                }
                else
                {
                    /* reallocate memory for the macro definition */
                    macroDefinition = realloc(macroDefinition, macroDefSize + lineLength + 1);
                    if (!macroDefinition)
                    {
                        customHandleMemoryError(fp, head);
                    }

                    strcat(macroDefinition, line); /* add the line to the macro definition */
                }

                macroDefSize += lineLength; /* update the size of the macro definition */
            }

            continue; /* dont write any code */
        }

        /* not inside a macro and not a macro start */

        /* check if the line contains a macro call */
        while (currentMacro)
        {
            char *foundMacro = strstr(line, currentMacro->name); /* get the macro */

            if (foundMacro != NULL)
            {
                int macroLength = strlen(currentMacro->name); /* get the length of the macro */
                /* ensure it actually is a macro (the word ends here) */
                if (isspace(foundMacro[macroLength]) || foundMacro[macroLength] == NULL_TERMINATOR)
                {
                    macroCall = foundMacro; /* found the macro call */

                    break; /* found the macro call */
                }

                /* ensure its not also a label */
                if (foundMacro[macroLength] == COLON)
                {
                    printErrorInLine(MACRO_NAME_IS_LABEL_ERROR, lineNum, macroLength, currentLine);
                    isError = TRUE; /* set the error flag */
                }
            }
            currentMacro = currentMacro->next;
        }

        if (macroCall)
        {
            /* write the parts with the macro definition to the output file */
            fprintf(outputFilePointer, "%s", currentMacro->definition);
        }
        else
        {
            /* no macro call founds, write the line as it is */
            fputs(line, outputFilePointer);
        }
    }

    rewind(outputFilePointer); /* rewind the output file pointer */

    freeMacroList(head); /* free the macro list */

    return isError; /* return the final error state */
}

/* add a macro to the list (returns true if successful, false otherwise) */
int addMacro(MacroNode **head, char *name, char *definition)
{
    MacroNode *newNode = malloc(sizeof(MacroNode)); /* create a new macro */
    if (!newNode)
    {
        return FALSE; /* memory allocation failed */
    }

    strcpy(newNode->name, name); /* copy the name */

    /* copy the definition */
    if (definition == NULL)
    {
        /* handle case when definition is empty */
        newNode->definition = malloc(1); /* allocate memory for a single charater (the null-terminator) */
        if (!newNode->definition)
        {
            free(newNode); /* free the allocated node */
            return FALSE;  /* indicate memory allocation error */
        }
        newNode->definition[0] = NULL_TERMINATOR; /* set the null terminator10 */
    }
    else
    {
        newNode->definition = malloc(strlen(definition) + 1);
        if (newNode->definition)
        {
            strcpy(newNode->definition, definition);
        }
    }

    if (!newNode->definition)
    {
        return FALSE; /* memory allocation failed */
    }

    newNode->next = *head; /* point to the current head */

    *head = newNode; /* update the head of the list */

    return TRUE;
}

/* frees the allocated memory for the macro table */
void freeMacroList(MacroNode **head)
{
    MacroNode *current = *head;
    while (current)
    {
        MacroNode *temp = current;
        current = current->next;

        /* free the alocated memory of this node */
        free(temp->definition);
        free(temp);
    }
}

/* check if a macro starts */
int isMacroStart(char *line)
{
    int foundMacroStart = FALSE;

    /* check if the first word is MACRO_START */
    if (STARTS_WITH_MACRO_START(line))
    {
        foundMacroStart = TRUE;
    }

    /* skip the first word */
    while (!isspace(*line) && *line != NULL_TERMINATOR)
    {
        line++;
    }
    skipWhiteSpaces(&line);

    if (foundMacroStart)
    {
        /* ensure there where no extra characters */

        /* skip the macro name */
        while (!isspace(*line))
        {
            line++;
        }
        skipWhiteSpaces(&line);

        /* check if there are extra characters */
        if (*line != NULL_TERMINATOR)
        {
            return EXTRANOUS_CHARACTERS;
        }
        else
        {
            return NO_ERROR;
        }
    }
    else
    {
        /* ensure no macro start exists after the first word */
        while (*line != NULL_TERMINATOR)
        {
            /* skip commas */
            if (*line == COMMA)
            {
                line++;
                skipWhiteSpaces(&line);
                continue;
            }

            if (STARTS_WITH_MACRO_START(line))
            {
                return EXTRANOUS_CHARACTERS;
            }

            /* skip word */
            while (!isspace(*line) && *line != NULL_TERMINATOR)
            {
                line++;
            }
            skipWhiteSpaces(&line);
        }
    }

    return NON_MACRO_RELATED; /* no macro end was found */
}

/* check if a macro ends */
int isMacroEnd(char *line)
{
    int foundMacroEnd = FALSE;

    /* check is the line starts with macro end */
    if (STARTS_WITH_MACRO_END(line))
    {
        foundMacroEnd = TRUE; /* found a valid macro end */
    }

    /* skip the first word */
    while (!isspace(*line) && *line != NULL_TERMINATOR)
    {
        line++;
    }
    skipWhiteSpaces(&line);

    if (foundMacroEnd)
    {

        /* ensure there were no extra characters */
        if (*line != NULL_TERMINATOR)
        {

            return EXTRANOUS_CHARACTERS;
        }
        else
        {
            return NO_ERROR;
        }
    }
    else
    {
        /* ensure no macro end exists after the first word */
        while (*line != NULL_TERMINATOR)
        {
            /* skip commas */
            if (*line == COMMA)
            {
                line++;
                skipWhiteSpaces(&line);
                continue;
            }

            if (STARTS_WITH_MACRO_END(line))
            {
                return EXTRANOUS_CHARACTERS;
            }

            /* skip word */
            while (!isspace(*line) && *line != NULL_TERMINATOR)
            {
                line++;
            }
            skipWhiteSpaces(&line);
        }
    }

    return NON_MACRO_RELATED; /* no macro end was found */
}

/* checks if a word is a valid macro name */
int isValidMacroName(char *word, int lineNum)
{
    int i;
    char *tracker = word;

    /* check if its an instruction name */
    if (IS_INSTRUCTION(word))
    {
        printErrorInLine(MACRO_IS_INSTRUCTION_ERROR, lineNum, word);
        return FALSE;
    }

    /* check if its a register */
    for (i = 0; i < REGISTERS_AMOUNT; i++)
    {
        char registerName[REGISTER_LENGTH];
        sprintf(registerName, "r%d", i);
        if (strcmp(word, registerName) == 0)
        {
            printErrorInLine(MACRO_IS_REGISTER_ERROR, lineNum, registerName);
            return FALSE;
        }
    }

    /* check if the word is a directive name */
    if (IS_DIRECTIVE(word))
    {
        printErrorInLine(MACRO_IS_DIRECTIVE_ERROR, lineNum, word);
        return FALSE;
    }

    /* check for valid syntax */

    /* start with a letter */
    if (!isalpha(*word))
    {
        printErrorInLine(MACRO_INVALID_NAME_START, lineNum, word);
        return FALSE;
    }

    /* does not contain illegal letters (only letters, numbers or underscore) */
    tracker++; /* already checked the first character */
    while (*tracker)
    {
        if (!isalnum(*tracker) && *tracker != UNDERSCORE)
        {
            printErrorInLine(MACRO_CONTAINS_INVALID_CHAR, lineNum, word);
            return FALSE;
        }
        tracker++; /* move to the next character */
    }

    return TRUE; /* otherwise its a valid name */
}

/* a function that frees the allocated data before handling a memory error */
void customHandleMemoryError(FILE *fp, MacroNode **head)
{
    fclose(fp);          /* close the file */
    freeMacroList(head); /* free the allocated memory for the macro list */

    handleMemoryError();
}
