#ifndef NDEGREE_POLYNOMIAL_CALCULATOR_H
#define NDEGREE_POLYNOMIAL_CALCULATOR_H

#include "threadSafeListenerQueue.h"

class NdegreePolynomialCalculator{

public:
	NdegreePolynomialCalculator(int32_t degree,int32_t numOfThreads, float accuracy) {
		this->degree = (degree >= 1 ? degree: 1);
		this->numOfThreads = (numOfThreads >= 1 ? numOfThreads : 1);
		this->accuracy = (accuracy > 0 && accuracy <= 1 ? accuracy : 0.01);
		this->hasFound = false;

		this->jobQueue = new ThreadSafeListenerQueue<task_t>();
		this->resultQueue = new ThreadSafeListenerQueue<task_t>();
	}

	~NdegreePolynomialCalculator() {
		delete(jobQueue);
		delete(resultQueue);
	}

	bool calculate(std::vector<int32_t>& coeff);

private:
	struct task_t
	{
		std::vector<int32_t> coeff;
		float fitness;
	};

	// thread working method
	void* threadWork(void* arg);

	// generate n random points and store them into points
	bool genRandomPoints(int32_t num, std::vector< std::pair<int32_t, int32_t> >& points) const;

	// given the coeff and a set of points, compute the square sum of difference
	float fitnessFunc(const std::vector<float> coeff, const std::vector< std::pair<int32_t, int32_t> > points) const;

	// degree of the polynomial
	int32_t degree;

	// num of threads we use to simultaneously calculate better fitness
	int32_t numOfThreads;

	// 0-1, which determines when we can terminate our program
	float accuracy;

	// use to help threads stop working when a good result is found
	bool hasFound;

	ThreadSafeListenerQueue<task_t>* jobQueue;

	ThreadSafeListenerQueue<task_t>* resultQueue;

	std::vector< std::pair<int32_t, int32_t> > points;
};

#endif