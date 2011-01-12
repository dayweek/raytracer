#ifndef __PERLIN_INCLUDED
#define __PERLIN_INCLUDED

#include "../core/array2.h"

// 2D smoothed noise
class SmoothNoise
{
public:
	Array2<float> noise;

	uint width;
	SmoothNoise(uint _width)
	{
		// widht + 2
		noise = Array2<float>(width);
		width = _width;
		generateRandoms();
	}
	void generateRandoms()
	{
		for(int y = 0; y < width; y++)
			for(int x = 0; x < width; x++)
				noise(x, y) = normalizedRand();
	}

	float normalizedRand() 
	{
		return rand() / (float)RAND_MAX;	
	}
	
	float smoothedSample(uint x, uint y)
	{
		
		float corners = (noise(x-1,y-1) + noise(x+1,y+1) 
			+ noise(x - 1,y + 1) + noise(x + 1, y- 1)) / 16.0f;
		float sides = (noise(x-1,y) + noise(x,y-1) 
			+ noise(x+1,y) + noise(x,y+1)) / 8.0f;
		float center = noise(x,y) / 4.0f;
		return corners + sides + center;
	}

};


class Perlin
{
public:
	float persistence;
	std::vector<SmoothNoise> noises;
	int n_octaves;
	//constructor
	float linearInterpolation(float fx, float fy, float fraction) 
	{
		return (1 - fraction) * fx + fraction * fy;
	}
	float interpolatedNoise(float fx, float fy, int noise_i) 
	{
		int x = (int)fx;
		int y = (int)fy;
		float fractionx = fx - x;
		float fractiony = fy - y;
		float l1 = linearInterpolation(noises[noise_i].smoothedSample(x, y), noises[noise_i].smoothedSample(x + 1, y), fractionx);
		float l2 = linearInterpolation(noises[noise_i].smoothedSample(x, y + 1), noises[noise_i].smoothedSample(x + 1, y + 1), fractionx);
		return linearInterpolation(l1, l2, fractiony);	
	}
	float sample(float x, float y) 
	{
		float frequency = 1;
		float amplitude = persistence;
		float total = 0.0f;
		for(int i = 0; i < n_octaves; i ++) {
			total += interpolatedNoise(x * frequency ,y * frequency, i) * amplitude;
			frequency *= 2;
			persistence *= persistence;
		}
	}
};
#endif __PERLIN_INCLUDED