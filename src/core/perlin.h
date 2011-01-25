#ifndef __PERLIN_INCLUDED
#define __PERLIN_INCLUDED

#include "../core/array2.h"
#include "memory.h"

// 2D smoothed noise
// to 2 mimic pseudo generating functions we save random values in an 2d array
// to mimic frequency we adjust distance between samples
// to mimic period we multiply the sample with a float
class SmoothNoise
{
protected:
	Array2<float> noise;

	uint width; //how many samples in both directions
	
	void generateRandoms()
	{
		for(int y = 0; y < width + 4; y++)
			for(int x = 0; x < width + 4; x++) {
				noise(x, y) = normalizedRand();
			}
	}
	
	// generate float between -1 and 1
	float normalizedRand() 
	{
		return (((rand() / (float)RAND_MAX)) * 2) - 1.0;	
	}
public:	
	// constructor also generates random values
	SmoothNoise(uint _width)
	{
		// we need 2 more samples on each side for smoothing
		noise = Array2<float>(_width + 4);
		width = _width;
		generateRandoms();
	}
	
	// we get smooth sample decreasing difference between neighbours 
	float smoothedSample(uint x, uint y) const
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


class Perlin: public RefCntBase
{
private:
	float persistence;
	std::vector<SmoothNoise> noises;
	int n_octaves;
public:
	uint width;

	// how many samplas we want - it grows quadratically
	// persistence determines frequency and amplitude
	// _n_octaves - how many functions we want to add
	Perlin(uint _width, float _persistence, int _n_octaves) 
	{
		width = _width;
		n_octaves = _n_octaves;
		persistence = _persistence;
		uint w = width;
		// we generate n_octaves noises
		for(int i = 1; i <= n_octaves; i++) {
			noises.push_back(SmoothNoise(w));
			w *= 2;
		}
		
	}
	
	// main method, for taking samples. we add functions
	// of different frequencies and amplitudes
	float sample(float x, float y) const
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

protected:

	// just a wrapper, children could override it
	virtual float noise(float x, float y, float frequency, float amplitude, int noise_i) const
	{
		return (interpolatedNoise(x * frequency, y * frequency, noise_i)) * amplitude;
	}
	
	// just a wrapper for changing noise before we return it, children could override it
	virtual	float customize(float x, float y, float total) const
	{
		return (total + 1.0) / 2.0;
	}
	
	float linearInterpolation(float fx, float fy, float fraction) const
	{
		return (1 - fraction) * fx + fraction * fy;
	}
	
	// we take 4 samples around fx, fy and do bilinear interpolation
	float interpolatedNoise(float fx, float fy, int noise_i) const
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
};

#endif //__PERLIN_INCLUDED