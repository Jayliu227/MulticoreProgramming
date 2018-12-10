#include <iostream>
#include <map>
#include <math.h>

#include <pthread.h>

using namespace std;

template <typename K, typename V>
class ConcurrentMultimap {

public:
	ConcurrentMultimap();
	~ConcurrentMultimap();

	bool insert(const K key, const V value);
	bool lookup(const K key, V& value);
	bool remove(const K key);
	bool truncate(const size_t n);

	pair<K, V> operator [] (const size_t n) {
		pair<K, V> result;
		
		pthread_rwlock_rdlock(&_lock);
		int index = min(n, _multimap.size() - 1);
		auto iter = _multimap.begin();
		for (int i = 0; i < index; i++) iter++;
		result = *iter;
		pthread_rwlock_unlock(&_lock);
		
		return result;
	}

private:
	std::multimap<K, V> _multimap;

	pthread_rwlock_t _lock;
};

// constructor
template<typename K, typename V>
ConcurrentMultimap<K, V>::ConcurrentMultimap() {
	pthread_rwlock_init(&_lock, nullptr);
}

// destructor
template<typename K, typename V>
ConcurrentMultimap<K, V>::~ConcurrentMultimap() {
	pthread_rwlock_destroy(&_lock);
}

template<typename K, typename V>
bool ConcurrentMultimap<K, V>::insert(const K key, const V value) {
	pthread_rwlock_wrlock(&_lock);
	_multimap.insert(make_pair(key, value));
	pthread_rwlock_unlock(&_lock);
	return true;
}

template<typename K, typename V>
bool ConcurrentMultimap<K, V>::lookup(const K key, V& value) {
	bool found = false;
	pthread_rwlock_rdlock(&_lock);
	auto iter = _multimap.find(key);
	if (iter != _multimap.end()) {
		found = true;
		value = iter->second;
	}
	pthread_rwlock_unlock(&_lock);
	return found;
}

template<typename K, typename V>
bool ConcurrentMultimap<K, V>::remove(const K key) {
	bool success = false;
	pthread_rwlock_wrlock(&_lock);
	success = _multimap.erase(key) > 0;
	pthread_rwlock_unlock(&_lock);
	return success;
}

template<typename K, typename V>
bool ConcurrentMultimap<K, V>::truncate(const size_t n) {
	pthread_rwlock_wrlock(&_lock);
	if (_multimap.size() <= n) {
		pthread_rwlock_unlock(&_lock);
		return true;
	}
	
	auto start = _multimap.begin();
	for (int i = 0; i < n; i++) start++;
	auto end = _multimap.end();
	_multimap.erase(start, end);

	pthread_rwlock_unlock(&_lock);
	return true;
}