#ifndef THREAD_SAFE_LISTENER_QUEUE_H
#define THREAD_SAFE_LISTENER_QUEUE_H

template<typename T>
class ThreadSafeListenerQueue {

public:
	ThreadSafeListenerQueue() {
		// initially set both head and tail to be -1
		head = tail = -1;
		// and the queue is empty
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
	bool push(const T element);

	/*
		Pop the least-recently inserted element from the queue and 
		fill in the passed-by-reference variable element with its contents, 
		if the queue was not empty. Return true if this was successful; 
		return false if the queue was empty. 
	*/
	bool pop(T& element);

	/*
		Similar to pop(), but block until there is an element to be popped. 
		Return true if an element was returned.
	*/
	bool listen(T& element);

private:

	// underlying store DS which is a std::vector
	std::vector<T> data;

	// indicates where the next pop should be
	int32_t head;

	// indicates the position of the most recent push
	int32_t tail;

	// used with cond variable to signify listeners
	bool isEmpty;

	pthread_mutex_t lock;

	pthread_cond_t cond;
};

#include "threadSafeListenerQueue.cpp"

#endif