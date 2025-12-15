/* create the instructions table */

typedef struct
{
    char *name;
    int funct;  /* in decimal */
    int opcode; /* in decimal */
} Instruction;

/* define an instruction table initializer */
#define INITIALIZE_INSTRUCTION_TABLE \
    {                                \
        {"mov", 0, 0},               \
        {"cmp", 0, 1},               \
        {"add", 1, 2},               \
        {"sub", 2, 2},               \
        {"lea", 0, 4},               \
        {"clr", 1, 5},               \
        {"not", 2, 5},               \
        {"inc", 3, 5},               \
        {"dec", 4, 5},               \
        {"jmp", 1, 9},               \
        {"bne", 2, 9},               \
        {"jsr", 3, 9},               \
        {"red", 0, 12},              \
        {"prn", 0, 13},              \
        {"rts", 0, 14},              \
        {"stop", 0, 15},             \
    }

/* define the length */
#define INSTRUCTION_TABLE_LENGTH 16

/* declare a function that codes the instruction line */
int codeInstructionLine(MemoryQueue *, Instruction *, char **, int);

/* declare a function that handles 2-operands instructions */
int handle2operands(MemoryQueue *, Instruction *, char *, char *);

/* declare a function that handles 1-operand instructions */
int handle1operands(MemoryQueue *, Instruction *, char *);

/* declare a function that handles 0-operand instructions */
int handle0operands(MemoryQueue *, Instruction *);

/* declare a function that codes an operand */
int handleCodeOperand(MemoryQueue *, char *, int);

/* declare a function that gets the insruction name */
void getInstructionName(char **, char *);

/* declare a function that parses a number */
int parseNumber(char *, int *);

/* declare a function that checks if a word is an instruction and returns it if so */
Instruction *getInstruction(char *);

/* declare a function that returns the addressing method */
int getAddressingMethod(char *);

/* define addressing methods */
#define IMMEDIATE_ADDRESSING 0
#define DIRECT_ADDRESSING 1
#define RELATIVE_ADDRESSING 2
#define DIRECT_REGISTER_ADDRESSING 3

/* define the first and last register numbers */
#define FIRST_REGISTER_NUM 0
#define LAST_REGISTER_NUM 7

/* define some error messages */
#define MISSING_NUMBER_ERROR "Found '#' without a following number. Ensure to insert the number"
#define MISSING_COMMA_ERROR "Found a missing comma in instruction line. Ensure to insert a comma between operands"
#define INVALID_CHARACTER_ERROR "'#' can only be followed by a number. Found a character after '#'s. Ensure to enter a valid number"
#define TOO_MANY_OPERANDS_ERROR "Found too many operands for instruction '%s'. Ensure to enter the correct amount"
#define NOT_ENOUGH_OPERANDS_ERROR "Not enough operands were inserted for instruction '%s'. Ensure to enter the correct amount"
#define INVALID_ADDRESSING_METHOD_FIRST_OP_ERROR "Invalid addressing method for the first operand. Ensure to use a valid one"
#define INVALID_ADDRESSING_METHOD_SECOND_OP_ERROR "Invalid addressing method for the second operand. Ensure to use a valid one"
#define CONSECUTIVE_COMMAS_ERROR "Found consecutive commas after the first operand. Ensure to seperate operands with a single comma"
#define INVALID_COMMA_ERROR "Found unexpected comma after first operand. Ensure to remove this comma, or insert a second operand, if wanted"
#define EXTRA_CHARACTERS_ERROR "found extra characters after the second operand. Try removing those"

/* define a macro that checks if an instruction name should have 2 operands */
#define SHOULD_HAVE_2_OPERANDS(name) \
    (strcmp((name), "mov") == 0 ||   \
     strcmp((name), "add") == 0 ||   \
     strcmp((name), "sub") == 0 ||   \
     strcmp((name), "lea") == 0 ||   \
     strcmp((name), "cmp") == 0)

/* define a macro that checks if an instruction name should have 1 operand */
#define SHOULD_HAVE_1_OPERAND(name) \
    (strcmp((name), "clr") == 0 ||  \
     strcmp((name), "not") == 0 ||  \
     strcmp((name), "inc") == 0 ||  \
     strcmp((name), "dec") == 0 ||  \
     strcmp((name), "jmp") == 0 ||  \
     strcmp((name), "bne") == 0 ||  \
     strcmp((name), "jsr") == 0 ||  \
     strcmp((name), "red") == 0 ||  \
     strcmp((name), "prn") == 0)

/* define a macro that checks if an instruction name should have 0 operands */
#define SHOULD_HAVE_0_OPERANDS(name) \
    (strcmp((name), "rts") == 0 ||   \
     strcmp((name), "stop") == 0)

/* define a macro that checks if the maximum memory has been surpassed */
#define IS_MEMORY_OVERFLOW(memory) ((memory) > MAX_MEMORY_SIZE)
