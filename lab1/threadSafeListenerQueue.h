#ifndef THREAD_SAFE_LISTENER_QUEUE_H
#define THREAD_SAFE_LISTENER_QUEUE_H

using namespace std;

template<typename T>
class ThreadSafeListenerQueue {

public:
	ThreadSafeListenerQueue() {
		head = end = 0;
		isEmpty = true;

		pthread_mutex_init(&lock, nullptr);
		pthread_cond_init(&cond, nullptr);
	}

	~ThreadSafeListenerQueue() {
		pthread_mutex_destroy(&lock);
		pthread_cond_destroy(&cond);
	}

	/*
		Should push the element onto the front of the list, 
		so that it will be the last of the items currently on the queue to be removed.
	*/
	bool Push(const T element);

	/*
		Pop the least-recently inserted element from the queue and 
		fill in the passed-by-reference variable element with its contents, 
		if the queue was not empty. Return true if this was successful; 
		return false if the queue was empty. 
	*/
	bool Pop(T& element);

	/*
		Similar to pop(), but block until there is an element to be popped. 
		Return true if an element was returned.
	*/
	bool Listen(T& element);

private:

	vector<T> data;

	int head;

	int end;

	bool isEmpty;
	
	pthread_mutex_t lock;

	pthread_cond_t cond;
};

#include "ThreadSafeListenerQueue.cpp"

#endif