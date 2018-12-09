program: main.o concurrentmultimap.o maze.o concurrentqueue.o
	g++ -std=c++11 main.o concurrentmultimap.o concurrentqueue.o maze.o -g -o program

concurrentmultimap.o: concurrentmultimap.cpp
	g++ -std=c++11 -c -g concurrentmultimap.cpp -pthread

concurrentqueue.o: concurrentqueue.cpp
	g++ -std=c++11 -c -g concurrentqueue.cpp -pthread

main.o: main.cpp concurrentmultimap.cpp
	g++ -std=c++11 -c -g main.cpp -pthread

maze.o: maze.cpp
	g++ -std=c++11 -Wall -c -g maze.cpp

clean:
	rm *.o program
