#ifndef MULTIMAP_H
#define MULTIMAP_H

template <typename K, typename V>
class multimap
{
public:
	multimap(){}
	
	~multimap(){}

	bool insert(const K& key, const V& value);

	bool find(const K& key);

	bool find(const K& key, std::vector<V>& values);

	int remove(const K& key);

private:
	struct node{
		K key;
		std::vector<V> values;
	};

	std::vector<node> data;
};

#include "multimap.cpp"

#endif
