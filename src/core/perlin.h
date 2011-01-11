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
	float linearInterpolation(float a, float b, float x) 
	{
		return a*(1-x) + b*x;
	}
	float normalizedRand() 
	{
		return rand() / (float)RAND_MAX;	
	}
	float sample(float x, float y)
	{
		
	}

};


class Perlin
{
public:
	Array2<float> a;

};
#endif __PERLIN_INCLUDED