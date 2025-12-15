#define MAX_INSTRUCTION_NAME_LENGTH 5 /* biggest name is "stop" (+1 to get a letter after it) */

/* define the symbol table */
typedef struct SymbolNode
{
   char symbol[MAX_SYMBOL_LENGTH + 1]; /* including the null-terminator */
   int value;
   char type[MAX_TYPE_LENGTH + 1]; /* including null-terminator */
   int isEntry;                    /* is entry flag */
   struct SymbolNode *next;
} SymbolNode;

#define BITS_IN_WORD 24

/* define the memory table */
typedef struct MemoryNode
{
   int value;
   int code : BITS_IN_WORD;
   int wordsAmount; /* for instructions (FALSE if its not the first word in an instruction line) */
   struct MemoryNode *next;
} MemoryNode;

typedef struct MemoryQueue
{
   MemoryNode *head;
   MemoryNode *tail;
} MemoryQueue;

/* define a node for the external words list (for the .ext output file) */
typedef struct ExternalWordNode
{
   int value;
   char symbol[MAX_SYMBOL_LENGTH + 1]; /* including the null-terminator */
   struct ExternalWordNode *next;
} ExternalWordNode;

/* define a function that initialized a memory queue */
MemoryQueue *initializeMemoryQueue();

/* declare the first transition function (returns a boolean value if there is an error or not)
   it also gets ICF and DCF */
int firstTransition(FILE *fp, SymbolNode **, MemoryQueue *, MemoryQueue *, unsigned int *, unsigned int *);

/* declare the second transition function (returns a boolean value if there is an error or not) */
int secondTransition(FILE *fp, SymbolNode *, MemoryQueue *, MemoryQueue *, ExternalWordNode **);

/* declare a function that checks if a symbol was already defined */
int isSymbolDefined(SymbolNode *, char *);

/* declare a function that adds to the symbol table */
int addToSymbolTable(SymbolNode **, char *, int, char *);

/* declare a function that gets a label */
int getLabel(char **, char *, int);

/* declare a function that gets operand label */
int getOperandLabel(char **, char *, int);

/* declare a function that skips the label in the line */
void skipLabel(char **);

/* declare a function that skips an operand */
void skipOperand(char **);

/* declare a function that handles label name error */
int checkLabelName(char *, int, int);

/* declare a function that checks if a line is a directive (.data or .string) */
int isDataOrString(char **);

/* declare a function that checks if a line is a directive (.extern) */
int isExtern(char **);

/* declare a function that checks if a line is a directive (.entry) */
int isEntry(char **);

/* declare a function that returns the value of a symbol */
SymbolNode *getOperandSymbol(char **, SymbolNode *, int);

/* declare a function that fills relative addressing words (second transition) */
void fillRelativeAddressingCode(MemoryNode *, SymbolNode *, int);

/* declare a function that fills direct addressing words (second transition) */
void fillDirectAddressingCode(MemoryNode *, SymbolNode *);

/* declare a function that will add a symbol and add it to the external word list */
int addToExternalList(ExternalWordNode **, char *, int);

/* declare a function that adds the isEntry flag for a certain symbol */
int addEntryFlag(SymbolNode *, char *, int);

/* declare a function that updates the value of each data symbol, by adding ICF */
void updateDataSymbols(SymbolNode **, int);

/* declare a function that adds to the memory table */
MemoryNode *addToMemoryTable(MemoryQueue *, int, int);

/* declare a function that codes data (returns the number of data inserted) */
int codeData(MemoryQueue *, char **, int);

/* declare a function that codes string */
int codeString(MemoryQueue *, char **, int);

/* declare a function that handles instruction lines */
int handleInstruction(char **, MemoryQueue *, int);

/* declare a function that returns the register number of an operand (if it a register) */
int getRegister(char *);

/* declare a function that checks if a line starts with an instruction */
int isStartWithInstruction(char **);

/* declare a function that checks if a string with a given length is an instruction name */
int isInstruction(char *, int);

/* declare a function that frees the symbol table */
void freeSymbolList(SymbolNode **);

/* declare a function that frees a memory table (from tail) */
void freeMemoryList(MemoryQueue *);

/* declare a function that frees the external word list */
void freeExternalWordList(ExternalWordNode **);

/* declare the custom handle memory allocation error */
void customMemoryErrorHandler(FILE *, SymbolNode **symbolHead, MemoryQueue *instructionQueue, MemoryQueue *dataQueue, ExternalWordNode **externalWordHead);

/* declare a function that creates the .ob file */
void writeObjectFile(int, int, char *, MemoryNode *, MemoryNode *);

/* declare a function that creates the .ext file */
void writeExternalFile(char *, ExternalWordNode *);

/* declare a function that creates the .ent file */
void writeEntryFile(char *, SymbolNode *);

/* define error codes */
#define SYNTAX_ERROR -1
#define MEMORY_ERROR -2
#define INVALID_REGISTER_NUM -3
#define TOO_MANY_OPERANDS -4
#define NOT_ENOUGH_OPERANDS -5
#define INVALID_ADDRESSING_METHOD_FIRST_OP -6
#define INVALID_ADDRESSING_METHOD_SECOND_OP -7
#define NOT_A_REGISTER -8
#define MISSING_NUMBER -9
#define INVALID_CHARACTER -10
#define MEMORY_OVERFLOW -11

/* define IC and DC as externs */
extern unsigned int IC;
extern unsigned int DC;

/* define the initial IC and DC values */
#define INITIAL_IC 100
#define INITIAL_DC 0

/* define chars */
#define DOT '.'
#define QUOTE_MARK '"'
#define SEVEN_CHAR '7'
#define CODE_FILLER '?' /* filler for uncodable words in first transition */
#define NEGATIVE_SIGN '-'
#define POSITIVE_SIGN '+'
#define HASHTAG '#'
#define ZERO_CHAR '0'
#define AMPERSAND '&'
#define COMMENT_CHAR ';'

/* define the maximum line length (including \n or \0) */
#define MAX_LINE_LENGTH 81

/* define the type symbols */
#define TYPE_DATA "da"
#define TYPE_EXTERNAL "ex"
#define TYPE_CODE "co"

/* define the directives */
#define DATA_DIRECTIVE ".data"
#define STRING_DIRECTIVE ".string"
#define ENTRY_DIRECTIVE ".entry"
#define EXTERN_DIRECTIVE ".extern"

/* define cases for the isDataOrString function (returns the number of data inserted) */
#define DATA_FOUND 1
#define STRING_FOUND 2

/* define some error prints */
#define SYMBOL_ALREADY_DEFINED "Symbol '%s' was already defined. Try a different name"
#define INVALID_INSTRUCTION_NAME "Found an invalid instruction name. Ensure your'e using a valid one"
#define INVALID_DIRECTIVE_NAME "'%.*s' is not a valid directive. Ensure your'e using a valid one (like .data, .string, .entry or .extern)"
#define LABEL_TOO_LONG_ERROR "The found label is too long. use a lable with %d characters or less"
#define LABEL_DOESNT_EXIST_ERROR "Found label '%s' that doesn't exist. Ensure to define it"
#define SYMBOL_DOESNT_EXIST_ERROR "Found symbol '%.*s' that doesn't exist. Ensure to define it"
#define LABEL_NOT_START_WITH_LETTER_ERROR "Found invalid label: '%.*s'. Ensure the label starts with a letter"
#define INVALID_CHARACTER_IN_LABEL_ERROR "Found invalid label: '%.*s'. Ensure the label only contains digits or numbers"
#define EMPTY_LABEL_ERROR "Found an empty label. Ensure to add the content of it before the ':'"
#define EXTRA_TEXT_AFTER_LABEL_ERROR "Found extra text after label declaration. Avoid adding an uneccessary characters"
#define SYMBOL_ALREADY_EXISTS_ERROR "Symbol name '%s' already exists. Try a different name"
#define LABEL_IS_INSTRUCTION_ERROR "Instruction '%.*s' can't be used as a label name. Try a different name"
#define LABEL_IS_REGISTER_ERROR "Found label '%.*s'. register names can't be used as labels. Try a different name"
#define EXTERNAL_AND_ENTRY_ERROR "Found label: '%s' defined as both external and entry. Ensure to define it as only one"
#define MACRO_NAME_IS_LABEL_ERROR "Found macro name '%.*s' that is also a label. Ensure to use a different name"
#define MEMORY_OVERFLOW_ERROR "Memory overflow. Ensure the program is not too big"

/* define the position of each part of a word (from the right) */
#define OPCODE_POS 18
#define SOURCE_ADDRESSING_POS 16
#define SOURCE_REGISTER_POS 13
#define DEST_ADDRESSING_POS 11
#define DEST_REGISTER_POS 8
#define FUNCT_POS 3
#define A_POS 2
#define R_POS 1
#define E_POS 0
#define ARE_LENGTH 3

/* define a mask to see if a word uses direct register addressing method */
#define REGISTER_ADDRESSING_MASK 0x3

/* define a mask to ensure numbers would be printed in 24 bits */
#define MASK_24BIT 0xFFFFFF

/* define a macro that checks if a pointer to a char and its length is a register */
#define IS_REGISTER(str, length) (!(strncmp(str, "r0", length) && strncmp(str, "r1", length) && strncmp(str, "r2", length) && strncmp(str, "r3", length) && strncmp(str, "r4", length) && strncmp(str, "r5", length) && strncmp(str, "r6", length) && strncmp(str, "r7", length)))

/* define the maximum memory size */
#define MAX_MEMORY_SIZE 2 << 21