#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <chrono>
#include "multimap.h"

struct errorHandler{
	bool success = true;
	int round;
	std::string error = "";
	std::vector<int> noninsertedKeys;
	std::vector<int> missingKeys;
	std::vector< std::pair<int, int> > missingPairs;
	void handleError(){
		if(success){
			std::cout << "Test case " << round << " succeeded!" << std::endl;
		}else{
			std::cout << "Test case " << round << " failed!" << std::endl;
			std::cout << "Error message: \n" << error << "Details: " << std::endl;

			std::cout << "\tNoninserted keys: " << std::endl;
			for(auto i : noninsertedKeys) {
				std::cout << "\t" << i << " ";
			}
			std::cout << std::endl;

			std::cout << "\tMissed keys: " << std::endl;
			for(auto i : missingKeys) {
				std::cout << "\t" << i << " ";
			}
			std::cout << std::endl;

			std::cout << "\tMissed pairs: " << std::endl;
			for(auto i : missingPairs) {
				std::cout << "\t" << i.first << "-" << i.second << " ";
			}
			std::cout << std::endl;
		}	
		std::cout << std::endl;
	}
};

void test(int numOfPairs, errorHandler& handler) {
	multimap<int, int> mp;

	std::default_random_engine gen;
	gen.seed(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<int> distr(0, 200);
	
	std::vector< std::pair<int, int> > pairs;

	std::cout << "Generating " << numOfPairs << " pairs of key-value..." << std::endl;
	for(int i = 0; i < numOfPairs; i++) {
		pairs.emplace_back(distr(gen), distr(gen));
	}

	std::cout << "Starting to insert elements into the map..." << std::endl;
	for(auto i : pairs) {
		if(!mp.insert(i.first, i.second)) {
			handler.error += "Insertion error.\n";
			handler.noninsertedKeys.emplace_back(i.first);
			handler.success &= false;
		}
	}

	std::cout << "Testing if all elements are stored in the map..." << std::endl;
	for(auto i : pairs) {
		int key = i.first;
		int value = i.second;
		if (!mp.find(key)) {
			handler.error += "Found a miss key miss.\n";
			handler.missingKeys.emplace_back(key);
			handler.success &= false;
		}
	}

	std::cout << "Testing if for every key every value is stored in the map..." << std::endl;
	for(auto i : pairs) {
		int key = i.first;
		int value = i.second;

		std::vector<int> storedValues;
		if (mp.find(key, storedValues)) {
			if (std::find(storedValues.begin(), storedValues.end(), value) == storedValues.end()) {
				handler.error += "Found a missing pair.\n";
				handler.missingPairs.emplace_back(key, value);
				handler.success &= false;
			}
		} else {
			handler.error += "Found a missing pair.\n";
			handler.missingPairs.emplace_back(key, value);
			handler.success &= false;
		}
	}	
}

int main(){
	const unsigned int testCases = 10;
	const unsigned int numOfPairs = 100;

	for(int i = 0; i < testCases; i++){
		std::cout << "Started test case " << (i + 1) << std::endl;
		errorHandler handler;
		handler.round = i + 1;
		test(numOfPairs, handler);
		handler.handleError();
	}

	return 0;
}
