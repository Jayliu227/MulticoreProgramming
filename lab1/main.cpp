#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <random>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "threadSafeKVStore.h"
#include "threadSafeListenerQueue.h"

const int32_t ITERATION_PER_THREAD = 10000;
int32_t THREAD_NUM;

clock_t programStartTime;
clock_t pthreadCompleteTime;

// thread input argument structure
struct pthreadArgs{
	int32_t threadID;
	ThreadSafeKVStore<std::string, int32_t>* mapStore;
	ThreadSafeListenerQueue<int32_t>* listenerQueue;
};

// function signitures:
// thread function, which takes in a pthreadArgs as the input which contains necessary info
void* threadFunction(void* arg);

int main(int argc, char* argv[]){

	int inputNumOfThreads = -1;
	int token;
	while((token = getopt(argc, argv, "n:")) != -1) {
		switch(token) {
			case 'n':
				inputNumOfThreads = atoi(optarg);
				break;
			case '?':
				if (optopt == 'n') {
					std::cout << "flag -n requires an integer to specify number of threads." << std::endl;
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

	// check if the specified number of threads is valid
	if (inputNumOfThreads < 1) {
		std::cout << "The number of threads specified needs to be greater or equal to 1." << std::endl;
		return 0;
	}

	THREAD_NUM = inputNumOfThreads;

	// instantiation
	auto threadSafeMapStore = new ThreadSafeKVStore<std::string, int32_t>();
	auto threadSafeListenerQueue = new ThreadSafeListenerQueue<int32_t>();

	// threads that would be spawned by main program
	pthread_t threads[THREAD_NUM];

	// arguments that would be passed to each thread
	pthreadArgs args[THREAD_NUM];

	// the clock time records the beginning of the program
	programStartTime = clock();

	// launch all threads
	for(int i = 0; i < THREAD_NUM; i++) {
		// we use the threadID to make random generator behave differently across threads
		args[i].threadID = (i + 1);
		// all threads share the same map store and listener queue
		args[i].mapStore = threadSafeMapStore;
		args[i].listenerQueue = threadSafeListenerQueue;

		if (pthread_create(&(threads[i]), NULL, &threadFunction, (void*) (&args[i]))) {
			std::cout << "Can not create thread." << std::endl;
		}
	}

	// get the sum of all values pushed to the queue
	int32_t sumFromAllThreads = 0;
	for(int i = 0; i < THREAD_NUM; i++) {
		int32_t value;
		threadSafeListenerQueue->listen(value);
		sumFromAllThreads += value * 1L;
	}

	// wait for all threads to finish
	for (int i = 0; i < THREAD_NUM; i++) {
		pthread_join(threads[i], NULL);
	}

	// compute the sum from the underlying map data structure
	int32_t sumFromUnderlyingDS = 0;
	auto begin = threadSafeMapStore->getBeginIterator();
	auto end = threadSafeMapStore->getEndIterator();
	while (begin != end) {
		sumFromUnderlyingDS += begin->second * 1L;
		begin++;
	}

	// when all threads are join, compute the time when the last thread ends
	pthreadCompleteTime = clock();
	float diff = (pthreadCompleteTime - programStartTime) / double(CLOCKS_PER_SEC);
	std::cout << "The time between launching the first thread and the final thread terminating is " << diff << "s."<< std::endl;	

	// test if these two are the same or not
	if (sumFromAllThreads == sumFromUnderlyingDS) {
		std::cout << "Passed test: sumFromAllThreads == sumFromUnderlyingDS, which equals to " << sumFromAllThreads << "." << std::endl;
	} else {
		std::cout << "Failed test: sumFromAllThreads = " << sumFromAllThreads << " sumFromUnderlyingDS = " << sumFromUnderlyingDS << "." << std::endl;
	}

	delete(threadSafeMapStore);
	delete(threadSafeListenerQueue);
	return 0;
}

void* threadFunction(void* arg) {
	// cast arg to be the argument structure
	pthreadArgs* arguments = (pthreadArgs*) arg;
	
	// create a random number generator and multple distribution
	std::default_random_engine generator;
	generator.seed(arguments->threadID);
	std::uniform_int_distribution<int32_t> probabilityDis(0, 100);
	std::uniform_int_distribution<int32_t> keyDis(0, 500);
	std::uniform_int_distribution<int32_t> valueDis(-256, 256);

	// create a vector to keep track of all keys
	std::vector<std::string> keyList;

	// the sum that would later on be pushed to the thread safe queue
	int32_t totalAccumulatedSum = 0;

	// start operations
	for (int i = 0; i < ITERATION_PER_THREAD; i++) {
		int prob = probabilityDis(generator);
		// with probability 20% we accumulate a key-value pair into the store
		if (prob <= 20) {
			std::string key = "user" + std::to_string(keyDis(generator));
			int32_t value = valueDis(generator);

			keyList.push_back(key);
			if (!arguments->mapStore->accumulate(key, value)) {
				printf("Thread %d has fatal error accumulating kv pair: %s -> %d.\n", arguments->threadID, key.c_str(), value);
			} else {
				totalAccumulatedSum += value;
			}
		} else {
		// with probability 80% we look up values we've accumulated before
			if (keyList.size() == 0) {
				continue;
			}
			// create a new distribution based on the size of keyList
			std::uniform_int_distribution<int32_t> lookUpDis(0, keyList.size() - 1);
			std::string lookUpKey = keyList[lookUpDis(generator)];

			int32_t value;
			if (!arguments->mapStore->lookup(lookUpKey, value)) {
				printf("Thread %d has fatal error finding key %s\n", arguments->threadID, lookUpKey.c_str());
			}
		}
	}

	// push the accumulated sum onto the queue
	arguments->listenerQueue->push(totalAccumulatedSum);

	// calculate the time used by this thread to complete all its work
	pthreadCompleteTime = clock();
	float diff = (pthreadCompleteTime - programStartTime) / double(CLOCKS_PER_SEC);
	printf("Thread %d completes using %fs with a total sum of %d accumulated.\n", arguments->threadID, diff, totalAccumulatedSum);
	pthread_exit(nullptr);
}

