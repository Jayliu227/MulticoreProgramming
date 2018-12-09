#include <iostream>
#include <atomic>
#include <random>
#include <time.h>
#include <stdlib.h>
#include <cmath>

#include "maze.h"
#include "concurrentmultimap.cpp"
#include "concurrentqueue.cpp"

using namespace std;

/* global state for the program */
size_t num_of_threads = 4;
size_t threshold = 1000;
size_t rows = 5;
size_t columns = 5;
size_t genome_length = 20;
atomic<uint64_t> futility_counter(0);

/* thread-safe containers shared by all threads */
ConcurrentMultimap<float, string> population;
ConcurrentQueue<string> offsprings;

/* mixer threads */
void* mixer(void* arg) {

}

/* mutator threads */
void* mutator(void* arg) {

}

/* init a new random genome */
string init_genome(size_t length, bool all_zeros) {
	string result = "";
	for (int i = 0; i < length; i++) {
		/* 0 represents no move, 1 to 4 represents four directions */
		if (all_zeros) {	
			result += '0';
		} else {
			result += '0' + rand() % 5;            
		}
	}

	return result;
}

float fitness_metrics(string& genome, Maze& maze) {
	int run_into_wall_counter = 0;
	/* initial coordinate */
	int cur_x = maze.getStart().row;
	int cur_y = maze.getStart().col;

	/* try each step */
	for (int i = 0; i < genome.size(); i++) {
		int attempt_move = genome[i] - '0';
		
		int attempt_x = cur_x;
		int attempt_y = cur_y;
		switch(attempt_move) {
			case 1:
				attempt_x++;                /* right */	
				break;
			case 2:
				attempt_y++;                /* up */
				break;
			case 3:
				attempt_x--;				/* left */
				break;
			case 4:
				attempt_y--;				/* down */
				break;
		}

		/* sanity check */
		if (attempt_x >= 0 && attempt_x < rows && attempt_y >= 0 && attempt_y < columns) {
			if (maze.get(attempt_x, attempt_y)) {
				run_into_wall_counter++;
			} else {
				cur_x = attempt_x;
				cur_y = attempt_y;
			}
		} else {
			run_into_wall_counter++;
		}
	}

	/* taxicab distance */
	int end_x = maze.getFinish().row;
	int end_y = maze.getFinish().col;

	float dist = sqrt((cur_x - end_x) * (cur_x - end_x) + (cur_y - end_y) * (cur_y - end_y));

	return 2.0 * dist + (float) run_into_wall_counter;
}

void print_maze(Maze& maze) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			cout << (maze.get(i, j) ? 1 : 0) << " ";
		}
		cout << endl;
	}
}

int main() {
	/* set random seed */
	srand(time(NULL));

	/* maze object */
	Maze maze(rows, columns);

	/* generate initial 4 * num_of_threads genomes */
	for (int i = 0; i < 4 * num_of_threads; i++) {
		string new_genome = init_genome(genome_length, false);
		float fitness = fitness_metrics(new_genome, maze);
		
		//cout << new_genome << " : " << fitness << endl;
		population.insert(fitness, new_genome);
	}	

	return 0;
}
