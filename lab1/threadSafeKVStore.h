#ifndef THREAD_SAFE_KV_STORE_H
#define THREAD_SAFE_KV_STORE_H

template<typename K, typename V>
class ThreadSafeKVStore {

public:
	ThreadSafeKVStore() {
		pthread_mutex_init(&lock, nullptr);
	}

	~ThreadSafeKVStore() {
		pthread_mutex_destroy(&lock);
	}

	/*
		Should insert the key-value pair if the key doesn’t exist in the hashmap, 
		or update the value if it does. 
	*/
	bool insert(const K key, const V value);

	/*
		Like insert(), but if the key-value pair already exists, 
		accumulate (i.e., add) the new value to the existing value. 
		This of course means that the templated V type must support the + operator.
	*/
	bool accumulate(const K key, const V value);

	/*
		Return true if the key is present, false otherwise. 
		If the key is present, fill the value variable
		(passed by reference) with the associated value.	
	*/
	bool lookup(const K key, V& value);

	/*
		Delete the key-value pair with key key from the hashmap, if it exists. 
		Do nothing if it does not exist.
	*/
	bool remove(const K key);

	// thread-UNsafe getter for the underlying unordered_map
	typename std::unordered_map<K, V>::iterator getBeginIterator() {
		return data.begin();
	}

	// thread-UNsafe getter for the underlying unordered_map
	typename std::unordered_map<K, V>::iterator getEndIterator() {
		return data.end();
	}

private:

	// use a std::unordered_map as the underlying data structure
	std::unordered_map <K, V> data; 

	pthread_mutex_t lock;

};

#include "threadSafeKVStore.cpp"

#endif