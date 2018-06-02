default: notebook

notebook.o: leNotebook.c
	gcc -c leNotebook.c -o notebook.o

notebook: notebook.o
	gcc notebook.o -o notebook

clean:
	-rm -f notebook.o
	-rm -f notebook

cleanNotebookFiles:
	-rm -f *.nb

cleanAll: clean cleanNotebookFiles