#include <iostream>
#include <vector>
#include "multimap.h"

template<typename K, typename V>
bool multimap<K, V>::insert(const K& key, const V& value){
	bool foundKey = false;
	for(int i = 0; i < data.size(); i++){
		if(data[i].key == key) {
			foundKey = true;
			data[i].values.emplace_back(value);
			break;
		}
	}
	if (!foundKey) {
		node n;
		n.key = key;
		std::vector<V> newValues;
		newValues.emplace_back(value);
		n.values = newValues;
		data.emplace_back(n);
	}
	return true;
}

template<typename K, typename V>
bool multimap<K, V>::find(const K& key){
	for(int i = 0; i < data.size(); i++) {
		if (data[i].key == key) {
			return true;
		}
	}
	return false;
}

template<typename K, typename V>
bool multimap<K, V>::find(const K& key, std::vector<V>& values){
	for(int i = 0; i < data.size(); i++) {
		if (data[i].key == key) {
			values = data[i].values;
			return true;
		}
	}
	
	return false;
}

template<typename K, typename V>
int multimap<K, V>::remove(const K& key){
	for(int i = 0; i < data.size(); i++) {
		if (data[i].key == key) {
			int removedNum = data[i].values.size();
			data.erase(data.begin() + i);
			return removedNum;
		}
	}

	return 0;
}