
program: main.o 
	g++ -std=c++11 main.o -o program

multimap.o: multimap.cpp multimap.h
	g++ -std=c++11 -c multimap.cpp

main.o: main.cpp multimap.h
	g++ -std=c++11 -c main.cpp

clean:
	rm *.o program
