#include "header.h"
#include "assemble.h"
#include "instructionsHandler.h"

/* function that handles an instruction line */
int handleInstruction(char **line, MemoryQueue *queue, int lineNum)
{
    Instruction *instruction; /* initialize the instruction */

    /* initialize the instruction name (+1 for null-terminator)*/
    char instructionName[MAX_INSTRUCTION_NAME_LENGTH] = {NULL_TERMINATOR};

    /* get the instruction name */
    getInstructionName(line, instructionName);

    if ((instruction = getInstruction(instructionName)) == NULL)
    {
        /* not a valid instruction name */
        printErrorInLine(INVALID_INSTRUCTION_NAME, lineNum);
        return SYNTAX_ERROR; /* indicate an error */
    }
    else
    {
        int codeInstructionResult;

        if ((codeInstructionResult = codeInstructionLine(queue, instruction, line, lineNum)) != NO_ERROR)
        {
            return codeInstructionResult; /* return the error code */
        }
    }

    return TRUE; /* handled successfully */
}

/* function that codes an instruction line */
int codeInstructionLine(MemoryQueue *queue, Instruction *instruction, char **line, int lineNum)
{
    /* initialize the 2 operands */
    char *firstOperand = NULL;
    char *secondOperand = NULL;

    int result;

    if (**line != NULL_TERMINATOR)
    {
        int commaFound = FALSE; /* flag for if a comma was found or not */

        firstOperand = *line; /* set first operand pointer */
        while (!isspace(**line) && **line != NULL_TERMINATOR && **line != COMMA)
        {
            (*line)++;
        }

        if (**line != NULL_TERMINATOR)
        {
            if (**line == COMMA)
            {
                commaFound = TRUE; /* set the commaFound flag */
            }

            **line = NULL_TERMINATOR; /* null-terminate first operand */
            (*line)++;
            skipWhiteSpaces(line);

            /* check for commas between the operands */
            while (**line == COMMA)
            {
                /* check if there are multiple commas (an error) */
                if (commaFound == TRUE)
                {
                    printErrorInLine(CONSECUTIVE_COMMAS_ERROR, lineNum);
                    return SYNTAX_ERROR;
                }
                else
                {
                    commaFound = TRUE; /* set the commaFound flag */
                    (*line)++;
                    skipWhiteSpaces(line);
                }
            }

            if (commaFound)
            {
                if (**line != NULL_TERMINATOR)
                {
                    secondOperand = *line; /* set second operand pointer */

                    /* skip after the second operand */
                    while (!isspace(**line) && **line != NULL_TERMINATOR)
                    {
                        (*line)++;
                    }

                    if (**line != NULL_TERMINATOR)
                    {
                        **line = NULL_TERMINATOR; /* null-terminate second operand */
                        (*line)++;
                    }

                    skipWhiteSpaces(line);

                    /* check for extra characters */
                    if (**line != NULL_TERMINATOR)
                    {
                        printErrorInLine(EXTRA_CHARACTERS_ERROR, lineNum);
                        return SYNTAX_ERROR;
                    }
                }
                else
                {
                    /* a comma without another operand (an error) */
                    printErrorInLine(INVALID_COMMA_ERROR, lineNum);
                    return SYNTAX_ERROR;
                }
            }
            else
            {
                skipWhiteSpaces(line);

                /* here, if there is another operand, a comma is missing between the 2 */
                if (**line != NULL_TERMINATOR)
                {
                    printErrorInLine(MISSING_COMMA_ERROR, lineNum);
                    return SYNTAX_ERROR;
                }
            }
        }
    }

    if (firstOperand != NULL && secondOperand != NULL)
    {
        result = handle2operands(queue, instruction, firstOperand, secondOperand);
    }
    else if (firstOperand != NULL)
    {
        result = handle1operands(queue, instruction, firstOperand);
    }
    else
    {
        result = handle0operands(queue, instruction);
    }

    if (result != NO_ERROR)
    {
        /* print the error message */
        switch (result)
        {
        case MEMORY_ERROR:
            return MEMORY_ERROR; /* simply return the memory error code */
        case MEMORY_OVERFLOW:
            return MEMORY_OVERFLOW; /* simply return the memory overflow error code */
        case TOO_MANY_OPERANDS:
            printErrorInLine(TOO_MANY_OPERANDS_ERROR, lineNum, instruction->name);
            break;
        case NOT_ENOUGH_OPERANDS:
            printErrorInLine(NOT_ENOUGH_OPERANDS_ERROR, lineNum, instruction->name);
            break;
        case INVALID_ADDRESSING_METHOD_FIRST_OP:
            printErrorInLine(INVALID_ADDRESSING_METHOD_FIRST_OP_ERROR, lineNum);
            break;
        case INVALID_ADDRESSING_METHOD_SECOND_OP:
            printErrorInLine(INVALID_ADDRESSING_METHOD_SECOND_OP_ERROR, lineNum);
            break;
        case MISSING_NUMBER:
            printErrorInLine(MISSING_NUMBER_ERROR, lineNum);
            break;
        case INVALID_CHARACTER:
            printErrorInLine(INVALID_CHARACTER_ERROR, lineNum);
            break;
        }

        return SYNTAX_ERROR;
    }

    return NO_ERROR;
}

/* function that handles an instruction line where 2 operands were found */
int handle2operands(MemoryQueue *queue, Instruction *instruction, char *sourceOperand, char *destOperand)
{
    int sourceAddressingMethod;
    int destAddressingMethod;

    int sourceRegisterNumber;
    int destRegisterNumber;

    int code; /* init the code */

    int sourceCodeResult;
    int destCodeResult;

    MemoryNode *firstWordNode; /* init the first code node */

    if (!SHOULD_HAVE_2_OPERANDS(instruction->name))
    {
        return TOO_MANY_OPERANDS;
    }

    /* get the addresing methods */
    sourceAddressingMethod = getAddressingMethod(sourceOperand);
    destAddressingMethod = getAddressingMethod(destOperand);

    /* for source operand */

    /* cannot be addressed with the relative addressing method */
    if (sourceAddressingMethod == RELATIVE_ADDRESSING)
    {
        return INVALID_ADDRESSING_METHOD_FIRST_OP;
    }

    /* lea instruction can only accept the direct addressing method */
    if (strcmp(instruction->name, "lea") == 0 && sourceAddressingMethod != DIRECT_ADDRESSING)
    {
        return INVALID_ADDRESSING_METHOD_FIRST_OP;
    }

    /* for dest operand */

    /* none can use the relative addressing method */
    if (destAddressingMethod == RELATIVE_ADDRESSING)
    {
        return INVALID_ADDRESSING_METHOD_SECOND_OP;
    }

    /* only cmp can use the immediate addressing method */
    if (strcmp(instruction->name, "cmp") != 0 && destAddressingMethod == IMMEDIATE_ADDRESSING)
    {
        return INVALID_ADDRESSING_METHOD_SECOND_OP;
    }

    /* code the instruction */

    code = instruction->opcode << OPCODE_POS;

    code |= sourceAddressingMethod << SOURCE_ADDRESSING_POS;

    if ((sourceRegisterNumber = getRegister(sourceOperand)) > 0)
    {
        /* not an error; code the register */
        code |= sourceRegisterNumber << SOURCE_REGISTER_POS;
    }

    code |= destAddressingMethod << DEST_ADDRESSING_POS;

    if ((destRegisterNumber = getRegister(destOperand)) > 0)
    {
        /* not an error; code the register */
        code |= destRegisterNumber << DEST_REGISTER_POS;
    }

    code |= instruction->funct << FUNCT_POS;

    code |= 1 << A_POS; /* the A is on for the first word. R and E are off */

    if ((firstWordNode = addToMemoryTable(queue, IC, code)) == NULL)
    {
        return MEMORY_ERROR;
    }
    IC++; /* increment IC */

    /* ensure the maximum memory has been surpassed */
    if (IS_MEMORY_OVERFLOW(IC + DC))
    {
        return MEMORY_OVERFLOW;
    }

    firstWordNode->wordsAmount++; /* increment the first word's wordsAmount */

    /* code the operands */

    sourceCodeResult = handleCodeOperand(queue, sourceOperand, sourceAddressingMethod);
    if (sourceCodeResult != NO_ERROR)
    {
        return sourceCodeResult; /* return the error code */
    }

    /* check if it used another word */
    if (sourceAddressingMethod != DIRECT_REGISTER_ADDRESSING)
    {
        firstWordNode->wordsAmount++; /* increment the first word's wordsAmount */
    }

    destCodeResult = handleCodeOperand(queue, destOperand, destAddressingMethod);
    if (destCodeResult != NO_ERROR)
    {
        return destCodeResult; /* return the error code */
    }

    /* check if it used another word */
    if (destAddressingMethod != DIRECT_REGISTER_ADDRESSING)
    {
        firstWordNode->wordsAmount++; /* increment the first word's wordsAmount */
    }

    return NO_ERROR;
}

/* function that handles instruction line where 1 operand was found */
int handle1operands(MemoryQueue *queue, Instruction *instruction, char *destOperand)
{
    int destAddressingMethod;
    int destRegisterNumber;

    int code; /* init the code */

    int destCodeResult;

    MemoryNode *firstWordNode; /* init the first code node */

    /* variable to hold if the instruction is one of those */
    int isBneJsrJmp = strcmp(instruction->name, "jmp") == 0 || strcmp(instruction->name, "bne") == 0 || strcmp(instruction->name, "jsr") == 0;

    if (!SHOULD_HAVE_1_OPERAND(instruction->name))
    {
        if (SHOULD_HAVE_0_OPERANDS(instruction->name))
        {
            return TOO_MANY_OPERANDS;
        }
        else
        {
            return NOT_ENOUGH_OPERANDS;
        }
    }

    /* get the addresing method */
    destAddressingMethod = getAddressingMethod(destOperand);

    /* jmp, bne and jsr can only be addressed with direct or relative addressing methods */
    if (isBneJsrJmp && destAddressingMethod != DIRECT_ADDRESSING && destAddressingMethod != RELATIVE_ADDRESSING)
    {
        return INVALID_ADDRESSING_METHOD_FIRST_OP;
    }

    /* the rest can be addressed only with direct or direct-register addressing methods
       (or if its prn immediate addressing could be used) */
    if (!isBneJsrJmp && (destAddressingMethod == RELATIVE_ADDRESSING || (destAddressingMethod == IMMEDIATE_ADDRESSING && strcmp(instruction->name, "prn") != 0)))
    {
        return INVALID_ADDRESSING_METHOD_FIRST_OP;
    }

    /* code the instruction (here source addressing method and source register are 0) */

    code = instruction->opcode << OPCODE_POS;

    code |= destAddressingMethod << DEST_ADDRESSING_POS;

    if ((destRegisterNumber = getRegister(destOperand)) > 0)
    { /* not an error; code the register */
        code |= destRegisterNumber << DEST_REGISTER_POS;
    }

    code |= instruction->funct << FUNCT_POS;

    code |= 1 << A_POS; /* the A is on for the first word. R and E are off */

    if ((firstWordNode = addToMemoryTable(queue, IC, code)) == NULL)
    {
        return MEMORY_ERROR;
    }
    IC++; /* increment IC */

    /* ensure the maximum memory has been surpassed */
    if (IS_MEMORY_OVERFLOW(IC + DC))
    {
        return MEMORY_OVERFLOW;
    }

    firstWordNode->wordsAmount++; /* increment the first word's wordsAmount */

    /* code the operand */
    destCodeResult = handleCodeOperand(queue, destOperand, destAddressingMethod);
    if (destCodeResult != NO_ERROR)
    {
        return destCodeResult; /* return the error code */
    }

    /* check if it used another word */
    if (destAddressingMethod != DIRECT_REGISTER_ADDRESSING)
    {
        firstWordNode->wordsAmount++; /* increment the first word's wordsAmount */
    }

    return NO_ERROR;
}

/* function that handles an instruction line where 0 operands were found */
int handle0operands(MemoryQueue *queue, Instruction *instruction)
{
    int code; /* init the code */

    MemoryNode *firstWordNode; /* init the first code node */

    if (!SHOULD_HAVE_0_OPERANDS(instruction->name))
    {
        return NOT_ENOUGH_OPERANDS;
    }

    /* code the insruction (here it only has opcode, rest is 0 accept the A) */
    code = instruction->opcode << OPCODE_POS;

    /* funct is 0 */

    code |= 1 << A_POS; /* the A is on for the first word. R and E are off */

    if ((firstWordNode = addToMemoryTable(queue, IC, code)) == NULL)
    {
        return MEMORY_ERROR;
    }
    IC++; /* increment IC */

    /* ensure the maximum memory has been surpassed */
    if (IS_MEMORY_OVERFLOW(IC + DC))
    {
        return MEMORY_OVERFLOW;
    }

    firstWordNode->wordsAmount++; /* increment the first word's wordsAmount */

    return NO_ERROR;
}

/* function that handles coding an operand */
int handleCodeOperand(MemoryQueue *queue, char *operand, int addressingMethod)
{
    if (addressingMethod == DIRECT_ADDRESSING || addressingMethod == RELATIVE_ADDRESSING)
    {
        /* still unable to get the value. insert a filler to the memory table (will be coded on the second transition) */

        if ((addToMemoryTable(queue, IC, CODE_FILLER)) == NULL)
        {
            return MEMORY_ERROR;
        }
        IC++; /* increment IC */
    }
    else if (addressingMethod == IMMEDIATE_ADDRESSING)
    {
        /* can code it right now */
        int number;            /* init the number */
        int parseNumberResult; /* init the result code */

        /* +1 to skip the hashtag */
        if ((parseNumberResult = parseNumber(operand + 1, &number)) == NO_ERROR)
        {
            /* code the number (with ARE as A is on, R and E are off) */
            int code = (number << ARE_LENGTH) | (1 << A_POS);
            if ((addToMemoryTable(queue, IC, code)) == NULL)
            {
                return MEMORY_ERROR;
            }
            IC++; /* increment IC */
        }
        else
        {
            return parseNumberResult; /* return the result error */
        }
    }

    /* ensure the maximum memory has been surpassed */
    if (IS_MEMORY_OVERFLOW(IC + DC))
    {
        return MEMORY_OVERFLOW;
    }

    return NO_ERROR;
}

/* function that returns the instruction found (NULL if not found) */
Instruction *getInstruction(char *word)
{
    static Instruction instructionTable[] = INITIALIZE_INSTRUCTION_TABLE;

    Instruction *found = NULL; /* initialize the returned insrtuction */
    int i;                     /* initialize iterator */

    for (i = 0; i < INSTRUCTION_TABLE_LENGTH; i++)
    {
        if (strcmp(word, instructionTable[i].name) == 0)
        {
            found = &instructionTable[i]; /* found a suspect to be an instruction name */
            break;
        }
    }

    return found;
}

/* function that returns the addressing method of an operand */
int getAddressingMethod(char *operand)
{
    int registerResult;

    if (*operand == HASHTAG)
    {
        return IMMEDIATE_ADDRESSING;
    }

    if (*operand == AMPERSAND)
    {
        return RELATIVE_ADDRESSING;
    }

    /* check about the register */
    registerResult = getRegister(operand);

    if (registerResult >= FIRST_REGISTER_NUM && registerResult <= LAST_REGISTER_NUM)
    {
        return DIRECT_REGISTER_ADDRESSING;
    }

    /* if its not one of those, its direct addressing */
    return DIRECT_ADDRESSING;
}

void getInstructionName(char **line, char *instruction)
{
    int i = 0;

    /* copy the word while ignoring any trailing whitespace */
    while (**line != NULL_TERMINATOR && !isspace(**line) && i < MAX_INSTRUCTION_NAME_LENGTH)
    {
        instruction[i++] = **line;
        (*line)++; /* move to the next character */
    }

    instruction[i] = NULL_TERMINATOR; /* null-terminate the instruction */

    /* ensure there is no additional characters */

    skipWhiteSpaces(line); /* skip whitespaces */
}

/* function gets a parsed number from a word */
int parseNumber(char *word, int *result)
{
    int number = 0;         /* init the searched number */
    int isNegative = FALSE; /* init the isNegative flag */

    if (*word == NULL_TERMINATOR || isspace(*word))
    {
        return MISSING_NUMBER;
    }

    if (*word == NEGATIVE_SIGN || *word == POSITIVE_SIGN)
    {
        if (*word == NEGATIVE_SIGN)
        {
            isNegative = TRUE; /* a negative number */
        }
        word++; /* move to the next character */
    }

    /* ensure the line is only folowed by digits until next whitespace */
    while (!isspace(*word) && *word != NULL_TERMINATOR)
    {
        if (!isdigit(*word))
        {
            /* not a number */
            return INVALID_CHARACTER;
        }
        else
        {
            /* valid digit */
            int digit = *word - ZERO_CHAR;
            number = number * 10 + digit; /* adjust the number */
        }

        word++; /* move to the next character */
    }

    /* get the final number */
    if (isNegative)
    {
        number = -number; /* change to negative */
    }

    *result = number; /* change the result */

    return NO_ERROR; /* indicate no error */
}

/* function that gets the register from an operand */
int getRegister(char *operand)
{
    int firstDigit; /* init the first digit after 'r' */

    /* first charater must be 'r' */
    if (*operand != 'r')
    {
        return NOT_A_REGISTER; /* not a register */
    }

    operand++; /* move to the next character */

    /* have to be a number after that */

    /* check if the next character is a digit */
    if (!isdigit(*operand))
    {
        return NOT_A_REGISTER; /* not a register */
    }

    firstDigit = *operand;

    operand++; /* move to the next character */

    if (!isspace(*operand) && *operand != NULL_TERMINATOR)
    {
        if (isdigit(*operand))
        {
            /* move to the next non-digit character */
            while (!isdigit(*(++operand)))
                ;

            /* if its a whitespace, invalid register number (can only get 0-7 and this is multiple digits) */
            if (isspace(*operand) || *operand == NULL_TERMINATOR)
            {
                return INVALID_REGISTER_NUM;
            }
            else
            {
                /* simply not a register */
                return NOT_A_REGISTER;
            }
        }
        else
        {
            return NOT_A_REGISTER; /* not a register */
        }
    }

    /* check if the first digit is valid */
    if (firstDigit > SEVEN_CHAR)
    {
        return INVALID_REGISTER_NUM;
    }

    firstDigit = firstDigit - ZERO_CHAR; /* parse to an actual number */

    return firstDigit; /* valid register */
}

/* function that checks if a line starts with an instrcution */
int isStartWithInstruction(char **line)
{
    int length = 0;
    int i;
    Instruction instructionTable[] = INITIALIZE_INSTRUCTION_TABLE;

    /* get the length of the first word */
    while (!isspace((*line)[length]))
    {
        length++;
    }

    for (i = 0; i < INSTRUCTION_TABLE_LENGTH; i++)
    {
        if (strncmp(instructionTable[i].name, *line, length) == 0)
        {
            /* does start with an instruction */

            *line = *line + length; /* skip to after the instruction name */
            skipWhiteSpaces(line);  /* skip whitespaces */

            return TRUE;
        }
    }

    return FALSE; /* not an instruction name */
}

/* function that checks if a string is an instruction (takes the first char and its length) */
int isInstruction(char *start, int length)
{
    int i;
    Instruction instructionTable[] = INITIALIZE_INSTRUCTION_TABLE;

    /* check each instruction name */
    for (i = 0; i < INSTRUCTION_TABLE_LENGTH; i++)
    {
        if (strncmp(instructionTable[i].name, start, length) == 0)
        {
            return TRUE; /* is an instruction name */
        }
    }

    return FALSE; /* didnt find a corresponding instruction name */
}