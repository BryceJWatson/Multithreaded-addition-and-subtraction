COMPILER = gcc
CFLAGS = -Wall -pedantic -g -pthread
EXES = a2

all: ${EXES}

a2: a2.c
	${COMPILER} ${CFLAGS} a2.c -o a2

clean:
	rm -f *~ *.o ${EXES}

run:
	./a2