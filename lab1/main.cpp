#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include <pthread.h>
#include <string>

#include "threadSafeKVStore.h"

using namespace std;

const int THREAD_NUM = 10;
pthread_t threads[THREAD_NUM];

ThreadSafeKVStore<string, int>* threadSafeMap;

void* threadFunction(void* arg) {
	long threadId = (long) arg;

	string key = "hello";
	threadSafeMap->Accumulate(key, threadId);

	printf("Thread %ld has ended.\n", threadId);

	pthread_exit(nullptr);
}

int main(){

	threadSafeMap = new ThreadSafeKVStore<string, int>();

	for(int i = 0; i < THREAD_NUM; i++) {
		if (pthread_create(&(threads[i]), NULL, &threadFunction, (void*) (i + 1L))) {
			cout << "Can not create thread." << endl;
		}
	}

	for (int i = 0; i < THREAD_NUM; i++) {
		pthread_join(threads[i], NULL);
	}

	for (int i = 0; i < THREAD_NUM; i++) {
		string key = "hello";
		int v = 0;
		if (threadSafeMap->Lookup(key, v)) {
			cout << v << endl;
		} else {
			cout << "error occured" << endl;
		}
	}

	delete(threadSafeMap);

	return 0;
}