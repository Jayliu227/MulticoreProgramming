
program: main.o
	g++ -std=c++11 main.o -g -o program

NdegreePolynomialCalculator.o: NdegreePolynomialCalculator.cpp NdegreePolynomialCalculator.h
	g++ -std=c++11 -c -g NdegreePolynomialCalculator.cpp -pthread

threadSafeListenerQueue.o: threadSafeListenerQueue.cpp threadSafeListenerQueue.h
	g++ -std=c++11 -c -g NdegreePolynomialCalculator.cpp -pthread

main.o: main.cpp NdegreePolynomialCalculator.h NdegreePolynomialCalculator.cpp threadSafeListenerQueue.h threadSafeListenerQueue.cpp
	g++ -std=c++11 -c -g main.cpp -pthread

clean:
	rm *.o program
