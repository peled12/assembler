#include "header.h"

/*
    This is the assembler project.
    It can accept assembly files and create object, entry and external for each. as well as a pre-assembler file.
    The assembler is divided into 2 main parts:
    1. Pre-assembler - this part is responsible for palnting the macro definitions where there called, and skipping comment and empty lines.
    2. Assembler - this part is responsible for compiling the assembly file a well as creating the output files.
*/

int main(int argc, char *argv[])
{
    int i;                  /* intilize the index that iterates between the files */
    int foundError = FALSE; /* initialize the error flag */

    /* ensure at least 1 file was inserted */
    if (argc == 1)
    {
        printf("No files were inserted.");
        return 1;
    }

    /* iterate between every file */
    for (i = 1; i < argc; i++)
    {
        char *filename = getFileName(argv[i]); /* get the final filename */
        FILE *file;                            /* open the file */
        FILE *preAssemblerFile;                /* initialize the preAssembler file */

        /* allocate memory for the new filename with the extension (+ 1 for null-terminator) */
        char *preAssemblerFileName = malloc(strlen(filename) + strlen(PRE_ASSEMBLER_FILE_EXTENTION) + 1);
        if (!preAssemblerFileName)
        {
            handleMemoryError();
        }

        /* copy the file name and concatenate the extension */
        strcpy(preAssemblerFileName, filename);
        strcat(preAssemblerFileName, PRE_ASSEMBLER_FILE_EXTENTION);

        file = openAssemblyFile(filename); /* open the file */

        if (file == NULL)
        {
            printf("An error occured opening the file '%s'.\n", filename);
            /* an error occured opening the file. was already printed */
            continue; /* continue to the next file */
        }

        /* print a message that indicates the start of file scanning */
        printf("Scanning file '%s'...\n", filename);

        if ((preAssemblerFile = openPreAssemblerFile(preAssemblerFileName)) == NULL)
        {
            printf("An error occured opening the pre-assembler file '%s'\n.", filename);
            /* an error occured opening the pre-assembler file. was already printed */
            continue; /* continue to the next file */
        }

        /* pre-assemble the file */
        if ((foundError = preAssembler(file, preAssemblerFile)) == FALSE)
        {
            foundError = assemble(preAssemblerFile, filename); /* assemble the file */

            fclose(preAssemblerFile); /* close the pre-assembler file */
        }
        else
        {
            fclose(preAssemblerFile); /* close the pre-assembler file */

            /* an error occured - remove the pre-assembler file */

            printf("An error occured pre-assembling the file.\n");

            if (remove(preAssemblerFileName) == 0)
            {
                printf("Pre-assembled file was successfully removed.\n");
            }
            else
            {
                printf("Failed to remove the pre-assembled file.\n");
            }
        }

        free(preAssemblerFileName); /* free the pre-assembler file name */
        fclose(file);               /* close the source file */
    }

    /* print a concluding message */
    if (foundError)
    {
        printf("Compilation process failed.\n");
    }
    else
    {
        printf("Compilation process completed successfully.\n");
    }

    return 0;
}
