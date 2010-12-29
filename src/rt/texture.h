#ifndef __INCLUDE_GUARD_6A0F8987_914D_41B8_8E51_53B29CF1A045
#define __INCLUDE_GUARD_6A0F8987_914D_41B8_8E51_53B29CF1A045
#ifdef _MSC_VER
	#pragma once
#endif


#include "../core/image.h"


//Specifies where the center of the texel is.
//Currently the value 0.5 means that (0.5, 0.5)
//	in normalized texture coordinates will correspond
//	to the center of the the pixel (0,0) in the image
//	This is quite visible with bilinear filtering, since
//	texel (1, 1) will actually be 
//	1/4 (pixel(0, 0) + pixel(0, 1) + pixel(1, 0) + pixel(1, 1)),
//	whereas texel(0.5, 0.5) will be = pixel(0, 0) of the imagge
#define _TEXEL_CENTER_OFFS 0.5f

//A texture class
class Texture : public RefCntBase
{

public:
	//What to do if the texture coordinates are outside
	//	of the texture
	enum TextureAddressMode
	{
		TAM_Wrap, //Wrap around 0. Results in a repeated texture
		TAM_Border, //Clamp the coordinate to the border. 
			//Results in the border pixels repeated
	};

	//The texture filtering mode. It affects magnifaction only
	enum TextureFilterMode
	{
		TFM_Point,
		TFM_Bilinear
	};

	SmartPtr<Image> image;

	TextureAddressMode addressModeX, addressModeY;
	TextureFilterMode filterMode;

	Texture()
	{
		addressModeX = TAM_Wrap;
		addressModeY = TAM_Wrap;
		filterMode = TFM_Point;
	}
	uint maybewrapx(uint x) const {
	  if(addressModeX == TAM_Wrap) {
	    return wrap(x, image->width());
	  }
	  return x;
	}
	uint maybewrapy(uint x) const {
	  if(addressModeY == TAM_Wrap) {
	    return wrap(x, image->height());
	  }
	  return x;
	}
	uint wrap(uint x, uint max) const {
	 return x % max; 
	}

	//Sample the texture. Coordinates are normalized:
	//	(0, 0) corresponds to pixel (0, 0) in the image and
	//	(1, 1) - to pixel (width - 1, height - 1) of the image,
	//	if doing point sampling
	float4 sample(const float2& _pos) const
	{
		//Denormalize the texture coordinates and offset the center
		//	of the texel
		float2 pos = 
			_pos * float2((float)image->width(), (float)image->height())
			+ float2(_TEXEL_CENTER_OFFS, _TEXEL_CENTER_OFFS);
		uint x = pos.x;
		uint y = pos.y;
		//TODO: Implement point and bilinear texture sampling

		if(filterMode == TFM_Point) {
			//return float4::rep(0.3f);
			x = maybewrapx(x);
			y = maybewrapy(y);
			

			float4 c = (*image)(x, y);
			
			return c;
		}
		else if(filterMode == TFM_Bilinear) {
			float s = pos.x - floor(pos.x);
			float t = pos.y - floor(pos.y);
			
			float4 c0 = ((*image)(maybewrapx((uint)pos.x), maybewrapy((uint)pos.y)));
			float4 c1 = ((*image)(maybewrapx((uint)(pos.x + 0.5)), maybewrapy((uint)pos.y)));
			float4 c2 = ((*image)(maybewrapx((uint)pos.x), maybewrapy((uint)(pos.y + 0.5))));
			float4 c3 = ((*image)(maybewrapx((uint)(pos.x + 0.5)), maybewrapy((uint)(pos.y + 0.5))));
			return c0.multi((1-s)*(1-t)) + 
				c1.multi((s)*(1-t)) + 
				c2.multi((1-s)*(t)) + 
				c3.multi((s)*(t));
		}
		else
			return float4::rep(0.f);
	}
};


#endif //__INCLUDE_GUARD_6A0F8987_914D_41B8_8E51_53B29CF1A045
