/* define a macro struct */
typedef struct MacroNode
{
   char name[MAX_MACRO_NAME_LENGTH + 1]; /* including the null-terminator */
   char *definition;
   struct MacroNode *next;
} MacroNode;

/* function to add a macro in the table
   (takes the head of the list, name and definition) */
int addMacro(MacroNode **, char *, char *);

/* function that checks if a macro starts (takes a line) */
int isMacroStart(char *);

/* function that checks if a macro ends (takes a pointer to a line) */
int isMacroEnd(char *);

/* define a function that checks if a string is a valid macro name (takes the string and a line number) */
int isValidMacroName(char *, int);

/* define a function that handles memory error */
void customHandleMemoryError(FILE *, MacroNode **);

/* define a function that frees the allocated memory for the macro list */
void freeMacroList(MacroNode **);

/* define the length of a register (with null terminator) */
#define REGISTER_LENGTH 3

/* macro to see if a string is a directive name */
#define IS_DIRECTIVE(str) (strcmp(str, "data") == 0 || strcmp(str, "string") == 0 || strcmp(str, "entry") == 0 || strcmp(str, "extern") == 0)

/* max line length (including \n or \0) */
#define MAX_LINE_LENGTH 81

/* define macro start and end */
#define MACRO_START "mcro"
#define MACRO_END "mcroend"

/* define the lengths of MACRO_START and MACRO_END */
#define MACRO_START_LENGTH 4
#define MACRO_END_LENGTH 7

/* define the inital macro definition size */
#define INITIAL_DEF_SIZE 0

/* define the white spaces in 1 word */
#define WHITE_SPACES " \t\n"

/* define error messages */
#define EXTRA_MACRO_START_CHARS_ERROR "Extra characters in macro assignment line; try removing those"
#define EXTRA_MACRO_END_CHARS_ERROR "Extra characters in macro end line; try removing those"
#define INVALID_MACRO_NAME_ERROR "Invalid macro name; the name you provided is invalid, try a different name"
#define MACRO_NAME_TOO_BIG_ERROR "The macro name you provided is too big. ensure using a name with %d characters or less"
#define LINE_TOO_BIG_ERROR "Line is too long; the maximum line length is %d characters"
#define MACRO_IS_INSTRUCTION_ERROR "Found macro name: %s; a macro name cannot be an instruction name. try a different name"
#define MACRO_IS_REGISTER_ERROR "Found macro name: %s; a macro name cannot be a register name. try a different name"
#define MACRO_IS_DIRECTIVE_ERROR "Found macro name: %s; a macro name cannot be a directive name. try a different name"
#define MACRO_INVALID_NAME_START "Found macro name: %s; a macro name must start with a letter. try a different name"
#define MACRO_CONTAINS_INVALID_CHAR "Found macro name: %s; a macro name can only contain letters, digits and underscore. try a different name"
#define MACRO_NAME_IS_LABEL_ERROR "Found macro name '%.*s' that is also a label. Ensure to use a different name"

/* define an error code */
#define EXTRANOUS_CHARACTERS -1
#define NON_MACRO_RELATED -2

/* define a macro that checks if a word is an instruction */
#define IS_INSTRUCTION(word) (strcmp(word, "mov") == 0 || strcmp(word, "cmp") == 0 || strcmp(word, "add") == 0 || strcmp(word, "sub") == 0 || strcmp(word, "not") == 0 || strcmp(word, "clr") == 0 || strcmp(word, "lea") == 0 || strcmp(word, "inc") == 0 || strcmp(word, "dec") == 0 || strcmp(word, "jmp") == 0 || strcmp(word, "bne") == 0 || strcmp(word, "red") == 0 || strcmp(word, "prn") == 0 || strcmp(word, "jsr") == 0 || strcmp(word, "rts") == 0 || strcmp(word, "stop") == 0)

/* define macros that check if a line starts with macro start */
#define STARTS_WITH_MACRO_START(line) ((strncmp(line, MACRO_START, MACRO_START_LENGTH) == 0) && (isspace(*(line + MACRO_START_LENGTH)) || *(line + MACRO_START_LENGTH) == NULL_TERMINATOR))

/* define macros that check if a line starts with macro end */
#define STARTS_WITH_MACRO_END(line) ((strncmp(line, MACRO_END, MACRO_END_LENGTH) == 0) && (isspace(*(line + MACRO_END_LENGTH)) || *(line + MACRO_END_LENGTH) == NULL_TERMINATOR))

/* define the underscore char */
#define UNDERSCORE '_'

/* define the comment character */
#define COMMENT_CHAR ';'