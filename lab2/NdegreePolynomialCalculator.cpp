#include <iostream>
#include <vector>
#include <chrono>
#include <utility>
#include <pthread.h>
#include <time.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include "NdegreePolynomialCalculator.h"

bool NdegreePolynomialCalculator::calculate(std::vector<float>& resultCoeff, bool useAnnealing){
	startTime = clock();

	// we need to generate n + 1 random points
	genRandomPoints();
	if (this->points.size() == 0) {
		return false;
	}

	// we also need to give some initial values to coefficients
	std::uniform_real_distribution<float> coeffDis(-20.0f, 20.0f);
	// initial coefficients, randomly generated
	std::vector<float> coefficients;
	for (int i = 1; i <= degree + 1; i++) {
		float new_co = coeffDis(generator);
		coefficients.push_back(new_co);
	}
	float initialFitness = fitnessFunc(coefficients);

	// we need to construct some threads and assign them with some tasks to do
	pthread_t threads[numOfThreads];

	// we first put numOfThreads jobs to the job queue	
	for (int i = 0; i < numOfThreads; i++) {
		task_t newTask;
		newTask.coeff = coefficients;
		newTask.fitness = initialFitness;
		this->jobQueue->push(newTask);
	}

	arg_t arguments[numOfThreads];

	// we spawn numOfThreads that many threads and let them do works
	for (int i = 0; i < numOfThreads; i++) {		
		arguments[i].self = this;
		arguments[i].threadNum = (i + 1);
		// we check if we are using annealing version
		if (pthread_create(&(threads[i]), nullptr, 
			useAnnealing ? &NdegreePolynomialCalculator::threadWorkAnnealing : &NdegreePolynomialCalculator::threadWork, 
			(void*)(&arguments[i]))) 
		{
			std::cout << "Can not create thread." << std::endl;
		}
	}

	// we listen on the queue, keep looking at if it has already found a good enough answer
	task_t bestResult;
	bestResult.coeff = coefficients;
	bestResult.fitness = initialFitness;

	// keep looping until we found a result that satisfy our requirement 'accuracy'
	while (true) {
		task_t result;
		this->resultQueue->listen(result);

		// if the new result is better, we record it and push it to the job queue
		if (result.fitness < bestResult.fitness) {
			bestResult.fitness = result.fitness;
			bestResult.coeff = result.coeff;
			if (bestResult.fitness < accuracy) {
				hasFound = true;
				break;
			}

			//std::cout << "New fitness: " << bestResult.fitness << "!" << std::endl;
		}	

		// else we simply push the old job to the queue
		this->jobQueue->push(bestResult);
	}

	// we send another round of jobs to the threads so that they know they need to stop
	for (int i = 0; i < numOfThreads; i++) {
		task_t meaninglessTask;
		meaninglessTask.fitness = -1;
		this->jobQueue->push(meaninglessTask);
	}

	for (int i = 0; i < numOfThreads; i++) {
		pthread_join(threads[i], NULL);
	}

	resultCoeff = bestResult.coeff;

	completeTime = clock();

	// print out results to the console
	std::cout << "Point Samples: ";
	for (auto& p : this->points) {
		float x = p.first;
		float y = p.second;
		std::cout << "(" << x << "," << y << ")" << " ";
	}
	std::cout << std::endl;

	std::cout << "Final Fitness: " << bestResult.fitness << "."<< std::endl;

	std::cout << "Coefficients: ";
	for (auto& i : bestResult.coeff) {
		std::cout << i << " ";
	}
	std::cout << std::endl;

	std::cout << "With " << numOfThreads << " Threads, Time Taken: " << ((completeTime - startTime) / double(CLOCKS_PER_SEC)) << "s." << std::endl;

	return true;
}

bool NdegreePolynomialCalculator::calculate(std::vector<float>& coeff, std::vector< std::pair<float, float> >& points, bool useAnnealing) {
	bool ok = calculate(coeff, useAnnealing);
	points = this->points;
	return ok;
}

// simulated annealing version
void* NdegreePolynomialCalculator::threadWorkAnnealing(void* arg) {
	arg_t* argument = (arg_t*) arg;

	NdegreePolynomialCalculator* self = argument->self;

	std::default_random_engine generator;

	generator.seed(argument->threadNum * std::chrono::system_clock::now().time_since_epoch().count());

	// the variance used for mutating
	std::uniform_real_distribution<float> mutateDis(-1.5f * argument->threadNum, 1.5f * argument->threadNum);

	// the max temperature and it decreases monotonically over time
	const int maxTemperature = 200;

	// if the optimal solution is not yet found
	while(!self->hasFound) {
		task_t myTask;
		// block until it receives a new task
		self->jobQueue->listen(myTask);

		// when fitness if passed as -1, it means we want to end our job
		if (myTask.fitness == -1) break;

		// our new fitness, initialized to be the same as the old fitness
		float newFitness = myTask.fitness;
		// our new coeff, intialized to be the same as the old coeff
		std::vector<float> newCoefficient = myTask.coeff;
		
		// we start from high temperature and keep decrementing
		for (int currentTemp = maxTemperature; currentTemp > 0 && !self->hasFound; currentTemp--) {
			// every round, we first take the old coefficients (not necessary the optimal)
			auto tempCo = newCoefficient;
			
			// we mutate each term
			for (int i = 0; i < tempCo.size(); i++) {
				// the reason why we multiply a factor is because higher degree tends to be more
				// stable, hence should not be altered too much (by observation)
				tempCo[i] += mutateDis(generator) * (i + 1) / (tempCo.size());
			}

			float tempFit = self->fitnessFunc(tempCo);

			// positive is good, negative is bad
			float deltaFit = newFitness - tempFit;

			// if the result is better, we record it
			if (deltaFit > 0) {
				newCoefficient = tempCo;
				newFitness = tempFit;
			// with certain probability, we will accept a bad result
			// the probability decreases as temperature goes down
			} else if (pow(M_E, deltaFit * 0.5 / currentTemp) > (rand() / double(RAND_MAX))) {
				newCoefficient = tempCo;
				newFitness = tempFit;
			}
		}	
		

		// store the result into myTask and push it to the result queue
		myTask.coeff = newCoefficient;
		myTask.fitness = newFitness;
		self->resultQueue->push(myTask);
	}
	
	pthread_exit(nullptr);
}

void* NdegreePolynomialCalculator::threadWork(void* arg) {
	arg_t* argument = (arg_t*) arg;

	NdegreePolynomialCalculator* self = argument->self;

	std::default_random_engine generator;

	generator.seed(argument->threadNum * std::chrono::system_clock::now().time_since_epoch().count());

	// timeout try
	const float timeoutTry = 2;

	// start and end time
	clock_t start, end;

	// the variance used for mutating
	std::uniform_real_distribution<float> mutateDis(-1.0f * argument->threadNum, 1.0f * argument->threadNum);

	// if the optimal solution is not yet found
	while(!self->hasFound) {
		task_t myTask;
		// block until it receives a new task
		self->jobQueue->listen(myTask);

		// when fitness if passed as -1, it means we want to end our job
		if (myTask.fitness == -1) break;

		start = clock();

		// our new fitness, initialized to be the same as the old fitness
		float newFitness = myTask.fitness;
		// our new coeff, intialized to be the same as the old coeff
		std::vector<float> newCoefficient = myTask.coeff;
		// we keep mutating until we find a better one
		while (!self->hasFound) {
			auto tempCo = myTask.coeff;
			
			for (int i = 0; i < tempCo.size(); i++) {
				tempCo[i] += mutateDis(generator) * (i + 1) / (tempCo.size());
			}

			float tempFit = self->fitnessFunc(tempCo);
			if (tempFit < myTask.fitness) {
				newCoefficient = tempCo;
				newFitness = tempFit;
				break;
			}

			end = clock();

			// if this thread is stuck at some point
			// where it can't find a better one, just break
			float timeElapsed = ((end - start) / double(CLOCKS_PER_SEC));

			if (timeElapsed > timeoutTry) {
				// after sending the non optimal result back
				// it will have a chance of getting a better result as 
				// the new input might be better
				break;
			}
		}

		// store the result into myTask and push it to the result queue
		myTask.coeff = newCoefficient;
		myTask.fitness = newFitness;
		self->resultQueue->push(myTask);
	}
	
	pthread_exit(nullptr);
}

bool NdegreePolynomialCalculator::genRandomPoints() {
	
	std::uniform_real_distribution<float> pointRangeDis(-50.0f, 50.0f);

	// n + 1 points for n degree polynomial
	for (int i = 1; i <= degree + 1; i++) {
		float new_x = pointRangeDis(generator);
		float new_y = pointRangeDis(generator);
		this->points.emplace_back(new_x, new_y);
	}

	return true;
}

float NdegreePolynomialCalculator::fitnessFunc(const std::vector<float>& coeff) const {
	float fitness = 0;

	for (auto& point : points) {
		float x = point.first;
		float y = point.second;

		float result = 0;
		float X = 1;
		// calculate the distance
		for (int i = coeff.size() - 1; i >= 0; i--) {
			float currentCo = coeff[i];
			result += currentCo * X;
			X *= x;
		}

		// the fitness is calculated the sum of
		// all sample point's y (expected value) minus calculated y value
		// over the y value 
		float ratio = (y - result) / y;
		// ratio = 0 if y = result
		// make sure it is positive
		fitness += (ratio < 0 ? -ratio : ratio);
	}

	return fitness / (this->points.size());
}
