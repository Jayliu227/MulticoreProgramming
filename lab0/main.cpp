#include <iostream>
#include <vector>
#include <random>
#include "multimap.h"

bool test(std::default_random_engine gen, std::uniform_int_distribution<int> distr, int numOfPairs) {
	bool success = true;

	multimap<int, int> mp;
	std::vector< std::pair<int, int> > pairs;

	std::cout << "Generating " << numOfPairs << " pairs of key-value..." << std::endl;
	for(int i = 0; i < numOfPairs; i++) {
		pairs.emplace_back(distr(gen), distr(gen));
		//std::cout << pairs[i].first << " " << pairs[i].second << std::endl;
	}

	std::cout << "Starting to insert elements into the map..." << std::endl;
	for(auto i : pairs) {
		mp.insert(i.first, i.second);
	}

	std::cout << "Testing if all elements are stored in the map..." << std::endl;
	for(auto i : pairs) {
		int key = i.first;
		int value = i.second;
		if (!mp.find(key)) {
			std::cout << "Found a miss: key-> " << key << " value->" << value << std::endl;
			success &= false;
			break;
		}
	}

	std::cout << "Testing if for every key every value is stored in the map..." << std::endl;
	for(auto i : pairs) {
		int key = i.first;
		int value = i.second;

		std::vector<int> storedValues;
		if (mp.find(key, storedValues)) {
			if (std::find(storedValues.begin(), storedValues.end(), value) == storedValues.end()) {
				std::cout << "Found a miss value: key-> " << key << " value->" << value << std::endl;
				success &= false;
				break;
			}
		} else {
			std::cout << "Found a miss key: key-> " << key << " value->" << value << std::endl;
			success &= false;
			break;
		}
	}	

	return success;
}

int main(){
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(0, 200);

	const unsigned int testCases = 10;
	const unsigned int numOfPairs = 100;

	for(int i = 0; i < testCases; i++){
		std::cout << "Start test case " << (i + 1) << std::endl;
		if (test(generator, distribution, numOfPairs)) {
			std::cout << "Test case " << (i + 1) << " succeeded!" << std::endl;
		} else {
			std::cout << "Test case " << (i + 1) << " failed!" << std::endl;
		}
		std::cout << std::endl;
	}

	return 0;
}

#include "multimap.cpp"
