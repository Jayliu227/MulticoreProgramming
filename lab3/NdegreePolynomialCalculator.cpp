#include <iostream>
#include <vector>
#include <chrono>
#include <utility>
#include <pthread.h>
#include <time.h>

#include "NdegreePolynomialCalculator.h"

using namespace std;
using namespace std::chrono;

bool NdegreePolynomialCalculator::calculate(vector<float>& resultCoeff){
	startTime = clock();
	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	// we need to generate n + 1 random points
	genRandomPoints();
	if (this->points.size() == 0) {
		return false;
	}

	// we also need to give some initial values to coefficients
	uniform_real_distribution<float> coeffDis(-3.0f, 3.0f);
	// initial coefficients, randomly generated
	vector<float> coefficients;
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
			NdegreePolynomialCalculator::threadWork, 
			(void*)(&arguments[i]))) 
		{
			std::cout << "Can not create thread." << std::endl;
			return false;
		}
	}

	// we listen on the queue, keep looking at if it has already found a good enough answer
	task_t bestResult;
	bestResult.coeff = coefficients;
	bestResult.fitness = initialFitness;

	// might overflow
	size_t numOfBestGuesses = 0;
	size_t numOfThreadGuesses = 0;

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

			numOfBestGuesses++;
			
			std::cout << "New fitness: " << bestResult.fitness << "!" << std::endl;
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

	vector<size_t> iterationsByThreads;

	for (int i = 0; i < numOfThreads; i++) {
		pthread_join(threads[i], nullptr);	
		iterationsByThreads.push_back(arguments[i].numOfIterations);
	}

	sort(iterationsByThreads.begin(), iterationsByThreads.end());

	resultCoeff = bestResult.coeff;

	completeTime = clock();
	high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

	// print out results to the console
	cout << "Print Results:" << endl;
	cout << "====================================" << endl;

	cout << "With " << numOfThreads << " Threads" << endl; 
	cout << "CPU Clock Time Taken: " << ((completeTime - startTime) / double(CLOCKS_PER_SEC)) << "s." << endl;
	cout << "Wall Clock Time Taken: " << duration_cast<microseconds>( t2 - t1 ).count() / 1000000.0 << "s." << endl;
	cout << endl;

	cout << "Number of best coefficients returned by threads is " << numOfBestGuesses << "." << endl;
	cout << "Number of guesses performed by all threads is " << std::accumulate(iterationsByThreads.begin(), iterationsByThreads.end(), 0) << "." << endl;
	cout << "Max number of iterations is " << *(iterationsByThreads.end() - 1) << "." << endl;
	cout << "Min number of iterations is " << *iterationsByThreads.begin() << "." << endl;
	cout << "Mean of iterations is " << (std::accumulate(iterationsByThreads.begin(), iterationsByThreads.end(), 0) / numOfThreads) << "." << endl;
	cout << "Median of iterations is " << iterationsByThreads[numOfThreads / 2] << "." << endl;
	cout << endl;

	cout << "Final Fitness: " << bestResult.fitness << "."<< endl;
	cout << "Point Samples: " << endl;
	for (auto& p : this->points) {
		float x = p.first;
		float y = p.second;
		cout << "(" << x << "," << y << ")" << endl;
	}

	cout << "Coefficients: ";
	for (auto& i : bestResult.coeff) {
		cout << i << " ";
	}
	cout << endl;

	return true;
}

bool NdegreePolynomialCalculator::calculate(vector<float>& coeff, vector< pair<float, float> >& points) {
	bool ok = calculate(coeff);
	points = this->points;
	return ok;
}

void* NdegreePolynomialCalculator::threadWork(void* arg) {
	arg_t* argument = (arg_t*) arg;

	NdegreePolynomialCalculator* self = argument->self;

	default_random_engine generator;

	generator.seed(argument->threadNum * system_clock::now().time_since_epoch().count());

	// timeout try
	const float timeoutTry = 2;

	// start and end time
	clock_t start, end;

	// the variance used for mutating
	std::uniform_real_distribution<float> mutateDis(-1.0f * argument->threadNum, 1.0f * argument->threadNum);

	// total num of guesses performed by this thread
	size_t numOfGuesses = 0;

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
			numOfGuesses++;

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
	
	argument->numOfIterations = numOfGuesses;
	pthread_exit(nullptr);
}

bool NdegreePolynomialCalculator::genRandomPoints() {
	
	std::uniform_real_distribution<float> pointRangeDis(-5.0f, 5.0f);

	// n + 1 points for n degree polynomial
	for (int i = 1; i <= degree + 1; i++) {
		float new_x = pointRangeDis(generator);
		float new_y = pointRangeDis(generator);
		this->points.emplace_back(new_x, new_y);
	}

	return true;
}

// sum of squared distances of all the points
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

		fitness += (result - y) * (result - y);
	}
	return fitness;
}
