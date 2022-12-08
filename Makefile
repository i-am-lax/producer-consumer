CC=g++ -Wall

all: main

main: helper.o main.o
	$(CC) -pthread -o main helper.o main.o

main.o: helper.cpp main.cpp
	$(CC) -c helper.cpp main.cpp

tidy:
	rm -f *.o core

clean:
	rm -f main producer consumer *.o core
