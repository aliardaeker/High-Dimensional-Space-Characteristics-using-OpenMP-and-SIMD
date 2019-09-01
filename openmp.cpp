#include <random>
#include <cmath>
#include <chrono>
#include <omp.h>
#include <iostream>
#include <fstream>

const int P = 10000;			// Number of points
const int D = 10;	 		// Number of dimension to test with
const int num_t = 8;

typedef struct point
{
	double data[16];
} point;

int 
main ()
{
	std::uniform_real_distribution <double> distribution(-1, 1);
	static point points[P];
	static int fractions[100] = {0};
	int sub_loop = P / num_t;
	//std::ofstream file;

	// Create points
	auto start_c = std::chrono::system_clock::now();
	#pragma omp parallel num_threads(num_t)
    {
    	int id = omp_get_thread_num();
    	std::default_random_engine eng(id);
		int offset = id * sub_loop;

		for (int i = 0; i < sub_loop; i++)
		{
			double dist = 0.0;
			point p;

			for (int j = 0; j < D; j++) p.data[j] = distribution(eng);
			for (int j = 0; j < D; j++) dist = dist + p.data[j] * p.data[j];

			if (std::sqrt(dist) > 1.0) i--;
			else points[i + offset] = p;
		}
	}
	auto stop_c = std::chrono::system_clock::now();

	// Check if points in the circle
	for (int i = 0; i < P; i++)
	{
		double dist = 0.0;
		for (int j = 0; j < D; j++) dist = dist + std::pow(points[i].data[j], 2);
		if (std::sqrt(dist) > 1.0) printf("Error\n");
	}

	// Traverse points and compute distance to the center
	auto start_t = std::chrono::system_clock::now();
	#pragma omp parallel for num_threads(num_t)
	for (int i = 0; i < P; i++)
	{
		double cmp = 0.01;
		double dist = 0.0;
		
		for (int k = 0; k < D; k++) dist = dist + std::pow(points[i].data[k], 2);
		dist = std::sqrt(dist);

		for (int j = 0; j < 100; j++)
		{
			if (dist <= cmp) 
			{
				#pragma omp atomic
				fractions[j]++;
				break;
			}
			else cmp += 0.01;
		}
	}
	auto stop_t = std::chrono::system_clock::now();

	// Print fractions and time
  	//file.open("data_16.txt");
  	//double total = 0;
	for (int i = 0; i < 100; i++) 
	{
		double f = (fractions[i] * 100) / (double) P;
		printf(" (%.2f - %.2f) -> %.2f%%\n", i/100.0, (i/100.0) + 0.01, f);
		//total = total + f;
		//file << f << "\n";
	}
	//printf("\nFraction: %f\n", total);
	//file.close();

	printf("\nTime to create points:      %.4f s\n", std::chrono::duration <double> (stop_c - start_c).count());
	printf("Time to compute fractions:  %.4f s\n\n", std::chrono::duration <double> (stop_t - start_t).count());
}
