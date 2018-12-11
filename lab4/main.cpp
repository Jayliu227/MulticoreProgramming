#include <iostream>
#include <atomic>
#include <random>
#include <time.h>
#include <stdlib.h>
#include <cmath>
#include <cassert>
#include <unistd.h>

#include "maze.h"
#include "concurrentmultimap.cpp"
#include "concurrentqueue.cpp"

using namespace std;

/* global state for the program */
size_t num_of_threads = 4;                   /* has to be greater than two */
size_t threshold = 100000;
size_t rows = 15;
size_t columns = 15;
size_t genome_length = 30;
atomic<uint64_t> futility_counter(0);

bool has_terminated = false;

/* thread-safe containers shared by all threads */
ConcurrentMultimap<float, string> population;
ConcurrentQueue<string> offsprings;

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

float fitness_metrics(const string& genome, Maze& maze) {
	assert(genome.size() == genome_length);

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
				attempt_y++;                /* down */
				break;
			case 3:
				attempt_x--;				/* left */
				break;
			case 4:
				attempt_y--;				/* up */
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
	printf("Print maze:\n");
	int start_x = maze.getStart().row;
	int start_y = maze.getStart().col;
	int end_x = maze.getFinish().row;
	int end_y = maze.getFinish().col;

	for (int j = 0; j < columns; j++) {
		for (int i = 0; i < rows; i++) {
			if (i == start_x && j == start_y) {
				printf("S ");
			} else if (i == end_x && j == end_y) {
				printf("E ");
			} else {
				printf("%c ", (maze.get(i, j) ? '1' : ' '));
			}
		}
		printf("\n");
	}
}


/* mixer threads */
void* mixer(void* arg) {
	while (!has_terminated) {
		/* choose two random rows from the generation map */
		size_t row_index1 = rand() % (4 * num_of_threads);
		size_t row_index2 = rand() % (4 * num_of_threads);

		string parent1 = population[row_index1].second;
		string parent2 = population[row_index2].second;

		/* randomly choose a splice point and join them together */
		size_t splice_point;
		/* make sure splice_point is not trivial */
		while ((splice_point = rand() % genome_length + 1) == 0 || splice_point == genome_length - 1);
	
		/* mix two parent genome to produce offspring genome */
		string mixed_genome = "";
		for (int i = 0; i < splice_point; i++) mixed_genome += parent1[i];
		for (int i = splice_point; i < genome_length; i++) mixed_genome += parent2[i];

		offsprings.push(mixed_genome);
	}
	pthread_exit(nullptr);
}

/* mutator threads */
void* mutator(void* arg) {
	Maze maze = *(Maze*)arg;

	while (!has_terminated) {
		/* get the first place in population and record the fitness */
		auto first_row = population[0];
		float best_fitness_so_far = first_row.first;

		/* fetch a offspring */
		string offspring_genome;
		offsprings.listen(offspring_genome);

		/* modifying one random element of the offspring with probability 40 */
		if (rand() % 100 <= 40) {
			size_t mutate_position = rand() % genome_length;
			offspring_genome[mutate_position] = rand() % 4 + '0';
		}

		/* compute the new fitness and insert it into the population, truncate the population */
		float offspring_fitness = fitness_metrics(offspring_genome, maze);
		population.insert(offspring_fitness, offspring_genome);
		population.truncate(4 * num_of_threads);

		first_row = population[0];
		float new_best_fitness = first_row.first;
		/* we check if the best fitness has changed */
		if (new_best_fitness < best_fitness_so_far) {
			futility_counter = 0;
		} else {
			futility_counter++;
			/* if we have not made any progress for <threshold> that many iteration, terminate */
			if (futility_counter > threshold) {
				has_terminated = true;
			}
		}
	} 
	pthread_exit(nullptr);
}

void input_handle(int argc, char* argv[]) {
	int token;
	while((token = getopt(argc, argv, "n:t:r:c:l:")) != -1) {
		switch(token) {
			case 'n':
				num_of_threads = atoi(optarg);
				break;
			case 't':
				threshold = atoi(optarg);
				break;
			case 'r':
				rows = atoi(optarg);
				break;
			case 'c':
				columns = atoi(optarg);
				break;
			case 'l':
				genome_length = atoi(optarg);
				break;
			case '?':
				if (optopt == 'n') {
					std::cout << "flag -n requires an integer to specify number of threads (greater than 1)." << std::endl;
				} else if (optopt == 't') {
					std::cout << "flag -t requires an integer to specify threshold (greater than 0)." << std::endl;
				} else if (optopt == 'r') {
					std::cout << "flag -r requires an integer to specify the rows of the maze (greater than 0)." << std::endl;
				} else if (optopt == 'c') {
					std::cout << "flag -c requires an integer to specify the columns of the maze (greater than 0)." << std::endl;					
				} else if (optopt == 'l') {
					std::cout << "flag -l requires an integer to specify the length of the genome (greater than 0)." << std::endl;					
				} else if (isprint(optopt)) {
					std::cout << "unknown command" << std::endl;
				} else {
					std::cout << "unknown character." << std::endl;
				}
				abort();
			default:
				abort();
		}
	}

	/* sanity check */
	if (num_of_threads < 2 ||
		threshold < 1 ||
		rows < 1 ||
		columns < 1 ||
		genome_length < 1
	) abort();
}

int main(int argc, char* argv[]) {
	/* set random seed */
	srand(time(NULL));

	/* handle input */
	input_handle(argc, argv);

	/* maze object */
	Maze maze(rows, columns);

	/* generate initial 4 * num_of_threads genomes */
	for (int i = 0; i < 4 * num_of_threads; i++) {
		string new_genome = init_genome(genome_length, false);
		float fitness = fitness_metrics(new_genome, maze);
		population.insert(fitness, new_genome);
	}	

	size_t num_of_mixers = num_of_threads / 2;
	size_t num_of_mutators = num_of_threads - num_of_mixers;

	pthread_t mixers[num_of_mixers];
	pthread_t mutators[num_of_mutators];

	/* spawn mixers and mutators */
	for (int i = 0; i < num_of_mixers; i++) { pthread_create(&mixers[i], nullptr, &mixer, nullptr); }
	for (int i = 0; i < num_of_mutators; i++) { pthread_create(&mutators[i], nullptr, &mutator, (void*) &maze); }

	/* join all threads */
	for (int i = 0; i < num_of_mixers; i++) { pthread_join(mixers[i], nullptr); }
	for (int i = 0; i < num_of_mutators; i++) { pthread_join(mutators[i], nullptr); }

	/* print the result */
	print_maze(maze);
	auto best_result = population[0];
	float fitness = best_result.first;
	string genome = best_result.second;

	printf("Number of threads: %zu\nThreshold: %zu\nGenome Length: %zu\n", num_of_threads, threshold, genome_length);
	printf("Best result:\n  fitness is %f\n  genome is %s", fitness, genome.c_str());

	return 0;
}
