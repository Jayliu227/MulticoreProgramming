#include <iostream>
#include <unordered_map>
#include <pthread.h>

#include "threadSafeKVStore.h"

template<typename K, typename V>
bool ThreadSafeKVStore<K, V>::insert(const K key, const V value){

	pthread_mutex_lock(&lock);
	// if we insert a value, we simply use [] to set its value
	data[key] = value;
	pthread_mutex_unlock(&lock);

	return true;
}

template<typename K, typename V>
bool ThreadSafeKVStore<K, V>::accumulate(const K key, const V value){

	pthread_mutex_lock(&lock);
	// we need to first check if it's inserted already
	if (data.find(key) == data.end()) {
		// if not, we use [] to set it value
		data[key] = value;
	} else {
		// otherwise, we increment it by the value, assuming + is supported
		data[key] += value;
	}
	pthread_mutex_unlock(&lock);

	return true;
}

template<typename K, typename V>
bool ThreadSafeKVStore<K, V>::lookup(const K key, V& value){
	bool found = false;

	pthread_mutex_lock(&lock);
	auto it = data.find(key);
	if (it != data.end()) {
		// if we find the key, simply return its value
		found = true;
		// set the reference
		value = it->second;
	}
	pthread_mutex_unlock(&lock);

	return found;
}

template<typename K, typename V>
bool ThreadSafeKVStore<K, V>::remove(const K key){

	pthread_mutex_lock(&lock);
	auto it = data.find(key);
	if (it != data.end()) {
		// if the data is in the map, remove it
		data.erase(it);
	}
	// otherwise, ignore this call
	pthread_mutex_unlock(&lock);

	return true;
}
