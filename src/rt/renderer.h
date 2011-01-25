#ifndef __INCLUDE_GUARD_0FEF5793_A843_48DE_8050_1137AEAD3804
#define __INCLUDE_GUARD_0FEF5793_A843_48DE_8050_1137AEAD3804
#ifdef _MSC_VER
	#pragma once
#endif

#include "../core/image.h"
#include "basic_definitions.h"

//A sampler telling how to sample a pixel
struct Sampler : public RefCntBase
{
	//A sample is a poisition [0..1]x[0..1] within the pixel
	//	as well as a weight, telling how much this sample
	//	contributes to the final value of the pixel
	struct Sample
	{
		float2 position;
		float weight;
	};

	//Pushes all samples to _result
	virtual void getSamples(uint _x, uint _y, std::vector<Sample> &_result) = 0;
};

//A renderer class
class Renderer
{
public:
	SmartPtr<Sampler> sampler;
	SmartPtr<Camera> camera;
	SmartPtr<Integrator> integrator;
	SmartPtr<Image> target;

	void render()
	{

		//Loop through all pixels in the scene and determine their color
		//	from the integrator
#pragma omp parallel 
		{
			std::vector<Sampler::Sample> samples;
#pragma omp for schedule(dynamic, 10)
			for(int y = 0; y < (int)target->height(); y++) {
// 				std::cout << y << std::endl;
				for(int x = 0; x < (int)target->width(); x++)
				{
					float4 color = float4::rep(0.f);

					samples.clear();
					sampler->getSamples((uint)x, (uint)y, samples);

					//Accumulate the samples
					for(size_t i = 0; i < samples.size(); i++)
					{
						Ray r = camera->getPrimaryRay(samples[i].position.x + x, samples[i].position.y + y);
						color += integrator->getRadiance(r) * float4::rep(samples[i].weight);
					}

					(*target)(x, y) = color;
				}
			}

		}
	}
};


#endif //__INCLUDE_GUARD_0FEF5793_A843_48DE_8050_1137AEAD3804
