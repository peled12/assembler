/* define some error prints */
#define FOUND_ILLEGAL_COMMA "An illegal comma was found. Ensure valid syntax"
#define FOUND_ILLEGAL_NUMBER "Found an illegal character after data directive. Ensure your'e using only numbers"
#define MISSING_COMMA_ERROR "Found a missing comma in a directive line. Ensure to add a comma"
#define NO_DATA_ARGS_ERROR "No argument found after directive .data. Ensure to add at least 1 argument"
#define CHAR_OUT_QUOTES "Found character outside of quotation marks in .string directive. Ensure all the characters after the .string directive are wrapped between \"s"
#define MISSING_ENDING_QUOTE "Expected the ending quotation mark in a .string directive. Ensure to add it"
#define NUMBER_TOO_BIG "The inserted number is to big. Use number that uses 24 bits or less"
#define MISSING_WHITE_SPACE_AFTER_COLON "Found a missing whitespace after colon. Ensure to seperate the label name and its definition with a whitespace"

/* define a macro that checks if its too large (24 bits signed) */
#define IS_LARGER_THAN_24_BITS(num) ((num) > 8388607 || (num) < -8388608)

/* define a function that parses a number in data directive */
int parseNumberInData(char **, int *);

/* define a function that handles the result of adding to the memory table */
int handleAddToMemoryTable(MemoryQueue *, int, int);
