all: project1 
project1: main.c semun.h
	gcc -o project1 main.c semun.h -lm -g
clean:
	-rm -f project1