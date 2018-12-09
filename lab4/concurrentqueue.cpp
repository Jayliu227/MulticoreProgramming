#include <iostream>
#include <vector>

#include <pthread.h>

template<typename T>
class ConcurrentQueue {

public:
	ConcurrentQueue();

	~ConcurrentQueue();

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
	std::vector<T> _data;

	// indicates where the next pop should be
	int32_t _head;

	// indicates the position of the most recent push
	int32_t _tail;

	// used with _cond variable to signify listeners
	bool _is_empty;

	pthread_mutex_t _lock;

	pthread_cond_t _cond;
};

template<typename T>
ConcurrentQueue<T>::ConcurrentQueue() {
	// initially set both _head and _tail to be -1
	_head = _tail = -1;
	// and the queue is empty
	_is_empty = true;

	pthread_mutex_init(&_lock, nullptr);
	pthread_cond_init(&_cond, nullptr);
}

template<typename T>
ConcurrentQueue<T>::~ConcurrentQueue() {
	pthread_mutex_destroy(&_lock);
	pthread_cond_destroy(&_cond);
}

template<typename T>
bool ConcurrentQueue<T>::push(const T element) {

	// push simply push_back the element to the end of the vector
	pthread_mutex_lock(&_lock);
	_data.push_back(element);
	_tail++;
	_is_empty = false;
	// everytime a new element is pushed, we signify the listener in the queue
	pthread_cond_signal(&_cond);
	pthread_mutex_unlock(&_lock);
	
	return true;
}

template<typename T>
bool ConcurrentQueue<T>::pop(T& element) {
	
	pthread_mutex_lock(&_lock);
	// if the queue is empty, simply return false
	if (_is_empty) {
		pthread_mutex_unlock(&_lock);
		return false;
	} else {
		// otherwise, we first increment the _head and pop it out
		element = _data[++_head];		
		// update _is_empty if necessary
		if (_head == _tail) {
			_is_empty = true;
		}
		pthread_mutex_unlock(&_lock);
		return true;
	}
}

template<typename T>
bool ConcurrentQueue<T>::listen(T& element) {

	pthread_mutex_lock(&_lock);
	// we use while loop to check if it is not empty
	while(_is_empty) {
		pthread_cond_wait(&_cond, &_lock);
	}
	// if is not empty, we increment _head and pop it out
	element = _data[++_head];
	if (_head == _tail) {
		_is_empty = true;
	}
	pthread_mutex_unlock(&_lock);
	
	return true;
}
