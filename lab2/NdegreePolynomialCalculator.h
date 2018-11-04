#ifndef NDEGREE_POLYNOMIAL_CALCULATOR_H
#define NDEGREE_POLYNOMIAL_CALCULATOR_H

#include <random>
#include "threadSafeListenerQueue.h"

class NdegreePolynomialCalculator{

public:
	NdegreePolynomialCalculator(int32_t degree,int32_t numOfThreads, float accuracy) {
		this->degree = (degree >= 1 ? degree : 1);
		this->numOfThreads = (numOfThreads >= 1 ? numOfThreads : 1);
		this->accuracy = accuracy;
		this->hasFound = false;

		this->jobQueue = new ThreadSafeListenerQueue<task_t>();
		this->resultQueue = new ThreadSafeListenerQueue<task_t>();

		this->generator.seed(std::chrono::system_clock::now().time_since_epoch().count());		
	}

	~NdegreePolynomialCalculator() {
		std::cout << "Cleaned up.." << std::endl;
		delete(jobQueue);
		delete(resultQueue);
	}

	bool calculate(std::vector<float>& coeff);

	bool calculate(std::vector<float>& coeff, std::vector< std::pair<float, float> >& points);

private:
	struct task_t {
		std::vector<float> coeff;
		float fitness;
	};
	
	// thread working method
	static void* threadWork(void *);

	// generate n random points and store them into points
	bool genRandomPoints();

	// given the coeff and a set of points, compute the square sum of difference
	float fitnessFunc(const std::vector<float>& coeff) const;

	// degree of the polynomial
	int32_t degree;

	// num of threads we use to simultaneously calculate better fitness
	int32_t numOfThreads;

	// 0-1, which determines when we can terminate our program
	float accuracy;

	// use to help threads stop working when a good result is found
	bool hasFound;

	clock_t startTime;
	
	clock_t completeTime;

	ThreadSafeListenerQueue<task_t>* jobQueue;

	ThreadSafeListenerQueue<task_t>* resultQueue;

	std::vector< std::pair<float, float> > points;

	std::default_random_engine generator;
};

#endif