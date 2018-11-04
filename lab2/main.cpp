#include <iostream>
#include <csignal>
#include <unistd.h>
#include <vector>

#include "NdegreePolynomialCalculator.h"

NdegreePolynomialCalculator* cal;

void signal_handler(int signalNum) { 
   std::cout << "Interrupted..." << std::endl;

   delete(cal);

   // terminate program
   exit(signalNum);   
}

float calculatePolynomial(const float x, const std::vector<float>& coeff) {
	float result = 0;
	float X = 1;
	for (int i = coeff.size() - 1; i >= 0; i--) {
		float currentCo = coeff[i];
		result += currentCo * X;
		X *= x;
	}
	return result;
}

int main(int argc, char* argv[]) {	
	std::signal(SIGINT, signal_handler);

	int inputNumOfThreads = 4;
	int inputDegree = 1;
	float inputAccuracy = 1.0f;

	int token;
	while((token = getopt(argc, argv, "n:d:f:")) != -1) {
		switch(token) {
			case 'n':
				inputNumOfThreads = atoi(optarg);
				break;
			case 'd':
				inputDegree = atoi(optarg);
				break;
			case 'f':
				inputAccuracy = atof(optarg);
				break;
			case '?':
				if (optopt == 'n') {
					std::cout << "flag -n requires an integer to specify number of threads (greater than 0)." << std::endl;
				} else if (optopt == 'd') {
					std::cout << "flag -d requires an integer to specify degree of the polynomial (greater or equal to 0)." << std::endl;
				} else if (optopt == 'f') {
					std::cout << "flag -f requires a float to specify the fitness accuracy (greater than 0)." << std::endl;
				} else if (isprint(optopt)) {
					std::cout << "unknown command" << std::endl;
				} else {
					std::cout << "unknown character." << std::endl;
				}
				return 0;
			default:
				abort();
		}
	}

	// degree, num of threads, accuracy
	cal = new NdegreePolynomialCalculator(inputDegree, inputNumOfThreads, inputAccuracy);
	
	std::vector<float> coeff;
	std::vector< std::pair<float, float> > sameplePoints;

	if (cal->calculate(coeff, sameplePoints)) {
		std::cout << "Real y Values Calculated Directly: " << std::endl;

		for (auto& point : sameplePoints) {
			float out = calculatePolynomial(point.first, coeff);
			std::cout << "x: " << point.first << " y:" << calculatePolynomial(point.first, coeff) << "." << std::endl;
		}		
	} else {
		std::cout << "Error in Calculating Coefficients." << std::endl;
	}

	delete(cal);

	return 0;
}