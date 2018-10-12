#include <iostream>
#include <vector>
#include <pthread.h>

#include "threadSafeListenerQueue.h"

using namespace std;

template<typename T>
bool ThreadSafeListenerQueue::Push(const T element) {
	return false;
}

template<typename T>
bool ThreadSafeListenerQueue::Pop(T& element) {
	return false;
}

template<typename T>
bool ThreadSafeListenerQueue::Listen(T& element) {
	return false;
}