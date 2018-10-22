#include <iostream>
#include <vector>

#include "NdegreePolynomialCalculator.h"

int main(int argc, char* argv[]) {	
	NdegreePolynomialCalculator* cal = new NdegreePolynomialCalculator(3, 4, 1);
	
	std::vector<float> v;
	cal->calculate(v);

	delete(cal);
	return 0;
}