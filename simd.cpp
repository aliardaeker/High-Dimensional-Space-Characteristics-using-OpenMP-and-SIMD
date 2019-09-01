#include <immintrin.h>
#include <cmath>
#include <functional>
#include <chrono>
#include <random>
#include <iostream>
#include <cassert>

// Number of points, 1 point has 4 dimensions
const int N = 4000 * 4000; // Must be divisible by 4

double
time (const std::function <void ()> &f) 
{
	f(); // Run once to warm the cache. 
    
    	auto start = std::chrono::system_clock::now();
    	f(); // Measure now.
    	auto stop = std::chrono::system_clock::now();

    	return std::chrono::duration <double> (stop - start).count();
}

int 
main ()
{
	// Use memory addresses which are multiple of 32, reserve 32 bytes for each 4 byte float value
	alignas(32) static float x[N], y[N], z[N], w[N];

	std::default_random_engine eng;
    	std::uniform_real_distribution <float> dist(0, 1);
    	for (int i = 0; i < N; i++) 
    	{
        	x[i] = dist(eng);
        	y[i] = dist(eng);
        	z[i] = dist(eng);
        	w[i] = dist(eng);
    	}	

    	static float length_seq[N / 2];
    	float tot;

    	auto seq = [&]() 
    	{
        	for (int i = 0; i < N / 2; i++) 
        	{
        		tot = std::pow(x[i * 2] - x[i * 2 + 1], 2);
        		tot = tot + std::pow(y[i * 2] - y[i * 2 + 1], 2);
        		tot = tot + std::pow(z[i * 2] - z[i * 2 + 1], 2);
        		tot = tot + std::pow(w[i * 2] - w[i * 2 + 1], 2);
     	       		length_seq[i] = std::sqrt(tot);
        	}
    	};

	double t = time(seq);
    std::cout << "\nSequential: \n"; 
	std::cout << "	Time: " << t << "  Performance: " << (N / t) / 1000000 << " Mops/s" << std::endl;

    alignas(32) static float data_T[N / 2][8];
   	bool even = true;

    	for (int i = 0; i < N / 2; i++)
    	{		
    		if (even)
    		{
    			data_T[i][0] = x[i * 2];
    			data_T[i][1] = y[i * 2];
    			data_T[i][2] = z[i * 2];
   			    data_T[i][3] = w[i * 2];
    			data_T[i][4] = x[i * 2 + 2];
    			data_T[i][5] = y[i * 2 + 2];
    			data_T[i][6] = z[i * 2 + 2];
    			data_T[i][7] = w[i * 2 + 2];
   		}
   		else
   		{
   			    data_T[i][0] = x[i * 2 - 1];
   			    data_T[i][1] = y[i * 2 - 1];
    			data_T[i][2] = z[i * 2 - 1];
    			data_T[i][3] = w[i * 2 - 1];
    			data_T[i][4] = x[i * 2 + 1];
  			    data_T[i][5] = y[i * 2 + 1];
   			    data_T[i][6] = z[i * 2 + 1];
   			    data_T[i][7] = w[i * 2 + 1];
   		}

    		even = !even; 
    	}

	/*
	std::cout << std::endl;
    	for (int i = 0; i < N; i++) std::cout << x[i] << " ";
    	std::cout << std::endl;
    	for (int i = 0; i < N; i++) std::cout << y[i] << " ";
    	std::cout << std::endl;
    	for (int i = 0; i < N; i++) std::cout << z[i] << " ";
    	std::cout << std::endl;
    	for (int i = 0; i < N; i++) std::cout << w[i] << " ";
	
	std::cout << std::endl << std::endl;
	for (int i = 0; i < N / 2; i++)
	{
		for (int j = 0; j < 8; j++) std::cout << data_T[i][j] << " ";
		std::cout << std::endl;
	}
	*/

    alignas(32) static float length_m[N / 4][8];
	static float length_vec[N / 2];	

    auto vec = [&]() 
    {
    	for (int i = 0; i < N / 4; i++) 
    	    {
        		__m256 data_1 = _mm256_load_ps(data_T[i * 2]);
        		__m256 data_2 = _mm256_load_ps(data_T[i * 2 + 1]);
        		__m256 res_sub = _mm256_sub_ps(data_1, data_2);
        		__m256 res_mul = _mm256_mul_ps(res_sub, res_sub);

        		__m256 one = _mm256_set1_ps(1);
        		__m256 res_4 = _mm256_hadd_ps(res_mul, one);
        		__m256 res_2 = _mm256_hadd_ps(res_4, one);
        		__m256 final_res = _mm256_sqrt_ps(res_2);
	
        		_mm256_store_ps(length_m[i], final_res);
        	}
	};
	
	t = time(vec);
	std::cout << "Vector: \n";
 	std::cout << "	Time: " << t << "  Performance: " << (N / t) / 1000000 << " Mops/s\n\n";	

	for (int i = 0; i < N / 4; i++)
	{
		length_vec[i * 2] = length_m[i][0];
		length_vec[i * 2 + 1] = length_m[i][4];	
	}
	
	for (int i = 0; i < N / 2; i++) 
	{
		//std::cout << "Seq: " << length_seq[i] << "  Vec: " << length_vec[i] << std::endl;
		if ((length_seq[i] - length_vec[i]) > 0.0001) assert(false);
	}
}
