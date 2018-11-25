#include <iostream>
#include <vector>
#include <pthread.h>

#include "threadSafeListenerQueue.h"

template<typename T>
bool ThreadSafeListenerQueue<T>::push(const T element) {

	// push simply push_back the element to the end of the vector
	pthread_mutex_lock(&lock);
	data.push_back(element);
	tail++;
	isEmpty = false;
	// everytime a new element is pushed, we signify the listener in the queue
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&lock);
	
	return true;
}

template<typename T>
bool ThreadSafeListenerQueue<T>::pop(T& element) {
	
	pthread_mutex_lock(&lock);
	// if the queue is empty, simply return false
	if (isEmpty) {
		pthread_mutex_unlock(&lock);
		return false;
	} else {
		// otherwise, we first increment the head and pop it out
		element = data[++head];		
		// update isEmpty if necessary
		if (head == tail) {
			isEmpty = true;
		}
		pthread_mutex_unlock(&lock);
		return true;
	}
}

template<typename T>
bool ThreadSafeListenerQueue<T>::listen(T& element) {

	pthread_mutex_lock(&lock);
	// we use while loop to check if it is not empty
	while(isEmpty) {
		pthread_cond_wait(&cond, &lock);
	}
	// if is not empty, we increment head and pop it out
	element = data[++head];
	if (head == tail) {
		isEmpty = true;
	}
	pthread_mutex_unlock(&lock);
	
	return true;
}


