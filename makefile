assembler: assembler.o errorHandler.o fileHandler.o preAssembler.o assemble.o firstTransitionHelper.o instructionsHandler.o secondTransitionHelper.o assembleHelper.o writeFinalFiles.o
	gcc -ansi -Wall -pedantic -g assembler.o errorHandler.o fileHandler.o preAssembler.o assemble.o firstTransitionHelper.o instructionsHandler.o secondTransitionHelper.o assembleHelper.o writeFinalFiles.o -o assembler

assembler.o: assembler.c header.h
	gcc -c -ansi -Wall -pedantic assembler.c -o assembler.o

errorHandler.o: errorHandler.c header.h
	gcc -c -ansi -Wall -pedantic errorHandler.c -o errorHandler.o

fileHandler.o: fileHandler.c header.h fileHandler.h
	gcc -c -ansi -Wall -pedantic fileHandler.c -o fileHandler.o

preAssembler.o: preAssembler.c header.h preAssembler.h
	gcc -c -ansi -Wall -pedantic preAssembler.c -o preAssembler.o

assemble.o: assemble.c header.h assemble.h
	gcc -c -ansi -Wall -pedantic assemble.c -o assemble.o

instructionsHandler.o: instructionsHandler.c header.h assemble.h instructionsHandler.h
	gcc -c -ansi -Wall -pedantic instructionsHandler.c -o instructionsHandler.o

firstTransitionHelper.o: firstTransitionHelper.c header.h assemble.h firstTransitionHeader.h
	gcc -c -ansi -Wall -pedantic firstTransitionHelper.c -o firstTransitionHelper.o

secondTransitionHelper.o: secondTransitionHelper.c header.h assemble.h
	gcc -c -ansi -Wall -pedantic secondTransitionHelper.c -o secondTransitionHelper.o

assembleHelper.o: assembleHelper.c header.h assemble.h
	gcc -c -ansi -Wall -pedantic assembleHelper.c -o assembleHelper.o

writeFinalFiles.o: writeFinalFiles.c header.h assemble.h
	gcc -c -ansi -Wall -pedantic writeFinalFiles.c -o writeFinalFiles.o

clean:
	del /Q assembler.exe *.o
