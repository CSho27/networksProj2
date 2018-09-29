all: proj2.o
	gcc -o proj2 proj2.o

proj2.o: proj2.c
	gcc -Wall -Werror -g -c proj2.c
	
clean:
	rm -f *.o
	rm -f proj2