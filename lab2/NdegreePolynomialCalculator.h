#ifndef NDEGREE_POLYNOMIAL_CALCULATOR_H
#define NDEGREE_POLYNOMIAL_CALCULATOR_H

#include <random>
#include "threadSafeListenerQueue.h"

class NdegreePolynomialCalculator{

public:
	NdegreePolynomialCalculator(int32_t degree,int32_t numOfThreads, float accuracy) {
		// set up private vars
		this->degree = (degree >= 1 ? degree : 1);
		this->numOfThreads = (numOfThreads >= 1 ? numOfThreads : 1);
		this->accuracy = accuracy;
		this->hasFound = false;

		this->jobQueue = new ThreadSafeListenerQueue<task_t>();
		this->resultQueue = new ThreadSafeListenerQueue<task_t>();

		// seed the generator
		this->generator.seed(std::chrono::system_clock::now().time_since_epoch().count());		
	}

	~NdegreePolynomialCalculator() {
		std::cout << "Cleaned up.." << std::endl;
		delete(jobQueue);
		delete(resultQueue);
	}

	bool calculate(std::vector<float>& coeff, bool useAnnealing);

	bool calculate(std::vector<float>& coeff, std::vector< std::pair<float, float> >& points, bool useAnnealing);

private:
	// structure that will be passed to job queue
	struct task_t {
		std::vector<float> coeff;
		float fitness;
	};
	
	// structure that can be used for threads to access the queue
	struct arg_t {
		NdegreePolynomialCalculator* self;
		size_t threadNum;
	};

	// thread working method
	static void* threadWork(void *);

	// thread working annealing
	static void* threadWorkAnnealing(void *);

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

	// start of the computation
	clock_t startTime;
	
	// end of the computation
	clock_t completeTime;

	// used to send coefficients and fitness to threads
	ThreadSafeListenerQueue<task_t>* jobQueue;

	// used to send coefficients and fitness back from threads 
	ThreadSafeListenerQueue<task_t>* resultQueue;

	// random points sample
	std::vector< std::pair<float, float> > points;

	std::default_random_engine generator;
};

#endif