#ifndef __NOISE_TEXTURES_INCLUDED
#define __NOISE_TEXTURES_INCLUDED

#include "../core/perlin.h"
#include "../core/algebra.h"



//A texture class
class CloudTexture : public TextureBase
{

public:
	
	class CloudPerlin: public Perlin
	{
	public:
		CloudPerlin(uint _width, float _persistence, int _n_octaves): Perlin(_width, _persistence, _n_octaves)
		{

		}
		virtual	float customize(float x, float y, float total) const
		{
			//return 0.5 * (1.0 + sin(x + c*total));
			//return sin(x +  total);
			float density = 1.1f;
			float coverage = -0.2f;
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
		perlin = new CloudPerlin(10, 0.5f, 6); 
	}
private:

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
		return perlin->getWidth();
	}
	virtual float height() const
	{
		return perlin->getWidth();
	}
};



#endif //__NOISE_TEXTURES_INCLUDED