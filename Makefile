all: simpler-tetris extended-tetris

simpler-tetris: posix-support.c tetris.c
	gcc posix-support.c tetris.c -o simpler-tetris

extended-tetris: posix-support.c tetris-ext.c
	gcc posix-support.c tetris-ext.c -o extended-tetris

clean:
	rm -f simpler-tetris extended-tetris
