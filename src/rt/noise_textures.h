#ifndef __NOISE_TEXTURES_INCLUDED
#define __NOISE_TEXTURES_INCLUDED

#include "../core/perlin.h"
#include "../core/algebra.h"



//A texture class
class CloudTexture : public TextureBase
{

public:
	// class for generating clouds in grayscale
	class CloudPerlin: public Perlin
	{
	public:
		CloudPerlin(uint _width, float _persistence, int _n_octaves): Perlin(_width, _persistence, _n_octaves)
		{}
		
		// make clouds with certain size and density
		virtual	float customize(float x, float y, float total) const
		{
			float density = 1.2f;
			float coverage = -0.1f;
			total = (total + coverage) * density;
			if(total < 0.0)
				total = 0.0;
			if(total > 1.0)
				total = 1.0;
			return total;
		}
	};
	SmartPtr<Perlin> perlin;
	

	CloudTexture()
	{
		addressModeX = TAM_Wrap;
		addressModeY = TAM_Wrap;
		filterMode = TFM_Point;
		perlin = new CloudPerlin(100, 0.6f, 6); 
	}
private:
	// we get grayscale sample and convert it to white-blue to look like a clouds
	virtual float4 lookupTexel(float _x, float _y) const
	{
		float realX = _x, realY = _y;
		fixAddress(realX, width(), addressModeX);
		fixAddress(realY, height(), addressModeY);
		float perlinSample = perlin->sample(realX, realY);
		
		float blue = 0.99f;
		float red = perlinSample;
		float green = perlinSample / 2.0 + 0.5;
		return float4(red, green, blue, 0.0);
	}
	virtual float width() const
	{
		return (float)perlin->width;
	}
	virtual float height() const
	{
		return (float)perlin->width;
	}
};



#endif //__NOISE_TEXTURES_INCLUDED