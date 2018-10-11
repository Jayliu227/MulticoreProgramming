#include <iostream>
#include <unordered_map>
#include <pthread.h>

#include "threadSafeKVStore.h"

using namespace std;

template<typename K, typename V>
bool ThreadSafeKVStore<K, V>::Insert(const K key, const V value){

	pthread_mutex_lock(&lock);
	data[key] = value;
	pthread_mutex_unlock(&lock);

	return true;
}

template<typename K, typename V>
bool ThreadSafeKVStore<K, V>::Accumulate(const K key, const V value){

	pthread_mutex_lock(&lock);
	if (data.find(key) == data.end()) {
		data[key] = value;
	} else {
		data[key] += value;
	}
	pthread_mutex_unlock(&lock);

	return true;
}

template<typename K, typename V>
bool ThreadSafeKVStore<K, V>::Lookup(const K key, V& value){
	bool found = false;

	pthread_mutex_lock(&lock);
	auto it = data.find(key);
	if (it != data.end()) {
		found = true;
		value = it->second;
	}
	pthread_mutex_unlock(&lock);

	return found;
}

template<typename K, typename V>
bool ThreadSafeKVStore<K, V>::Remove(const K key){

	pthread_mutex_lock(&lock);
	auto it = data.find(key);
	if (it != data.end()) {
		data.erase(it);
	}
	pthread_mutex_unlock(&lock);

	return false;
}
