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
		// widht + 4
		noise = Array2<float>(_width + 4);
		width = _width;
		generateRandoms();
	}
	void generateRandoms()
	{
		for(int y = 0; y < width + 4; y++)
			for(int x = 0; x < width + 4; x++) {
				noise(x, y) = normalizedRand();
			}
	}

	float normalizedRand() 
	{
		return (((rand() / (float)RAND_MAX)) * 2) - 1.0;	
	}
	
	float smoothedSample(uint x, uint y)
	{
		x += 2;
		y += 2;
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
	uint width;
	float c;
	//constructor
	Perlin(uint _width, float _persistence, int _n_octaves) 
	{
		width = _width;
		n_octaves = _n_octaves;
		persistence = _persistence;
		c = 5;
		uint w = width;
		for(int i = 1; i <= n_octaves; i++) {
			noises.push_back(SmoothNoise(w));
			w *= 2;
		}
		
	}
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
		float li = linearInterpolation(l1, l2, fractiony);	
		return li;
	}
	float noise(float x, float y, float frequency, float amplitude, int noise_i) 
	{
		return (interpolatedNoise(x * frequency, y * frequency, noise_i)) * amplitude;
	}
	float customize(float x, float y, float total) 
	{
		//return 0.5 * (1.0 + sin(x + c*total));
		//return sin(x +  total);
		return (total + 1.0) / 2.0;
	}
	float sample(float x, float y) 
	{
		float frequency = 1;
		float amplitude = 1;
		float total = 0.0f;
		for(int i = 0; i < n_octaves; i ++) {
			total += noise(x, y, frequency, amplitude, i);
			frequency *= 2;
			amplitude *= persistence;
		}
		return customize(x, y, total);
	}
};
#endif //__PERLIN_INCLUDED