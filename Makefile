all: server.o server.h
	g++ -g server.o -pthread -o server.exe
.phony: clean run

clean:
	rm *.o *.exe

#run: all
#	./server.exe 10000
