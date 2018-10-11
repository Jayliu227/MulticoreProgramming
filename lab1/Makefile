
program: main.o
	g++ -std=c++11 main.o -g -o program

threadSafeKVStore.o: threadSafeKVStore.cpp threadSafeKVStore.h
	g++ -std=c++11 -c -g threadSafeKVStore.cpp -pthread

main.o: main.cpp threadSafeKVStore.h threadSafeKVStore.cpp
	g++ -std=c++11 -c -g main.cpp -pthread

clean:
	rm *.o program
