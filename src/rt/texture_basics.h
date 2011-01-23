#ifndef __INCLUDE_TEXTURE_BASICS
#define __INCLUDE_TEXTURE_BASICS
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
class TextureBase : public RefCntBase
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


	TextureAddressMode addressModeX, addressModeY;
	TextureFilterMode filterMode;

	TextureBase()
	{
		addressModeX = TAM_Wrap;
		addressModeY = TAM_Wrap;
		filterMode = TFM_Point;
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
			_pos * float2(width(), height())
			+ float2(_TEXEL_CENTER_OFFS, _TEXEL_CENTER_OFFS);

		if(filterMode == TFM_Point)
			return lookupTexel(pos.x, pos.y);
		else if(filterMode == TFM_Bilinear)
		{
			float x_lo = floor(pos.x);
			float y_lo = floor(pos.y);
			float x_hi = ceil(pos.x);
			float y_hi = ceil(pos.y);

			float4 pix[2][2];
			pix[0][0] = lookupTexel(x_lo, y_lo);
			pix[1][0] = lookupTexel(x_hi, y_lo);
			pix[0][1] = lookupTexel(x_lo, y_hi);
			pix[1][1] = lookupTexel(x_hi, y_hi);

			float4 xhw = float4::rep(pos.x - x_lo);
			float4 yhw = float4::rep(pos.y - y_lo);
			float4 xlw = float4::rep(1 - xhw.x);
			float4 ylw = float4::rep(1 - yhw.x);

			return
				ylw * (xlw * pix[0][0] + xhw * pix[1][0]) +
				yhw * (xlw * pix[0][1] + xhw * pix[1][1]);
		}
		else
			return float4::rep(0.f);
	}
	float2 derivatives(const float2& _pos) const
	{
		// bilinear transformation

		float2 pos = _pos * float2(width(), height());
		pos.x = floor(pos.x);
		pos.y = floor(pos.y);
		pos += float2(_TEXEL_CENTER_OFFS, _TEXEL_CENTER_OFFS);
		float4 c0 = lookupTexel(pos.x, pos.y);
		float4 c1 = lookupTexel(pos.x + 1.0, pos.y);
		float4 c2 = lookupTexel(pos.x, pos.y + 1.0);

		return float2(c1[0] - c0[0], c2[0] - c0[0]);
	}
	
	Vector bumpVector1(const float2& _pos) const
	{
		// bilinear transformation

		float2 pos = _pos * float2(width(), height());
		pos.x = floor(pos.x);
		pos.y = floor(pos.y);
		pos += float2(_TEXEL_CENTER_OFFS, _TEXEL_CENTER_OFFS);
		float4 c0 = lookupTexel(pos.x, pos.y);
		float4 c1 = lookupTexel(pos.x + 1.0, pos.y);
		float4 c2 = lookupTexel(pos.x, pos.y + 1.0);

		return Vector(1, 0, c1[0] - c0[0]);
	}
	
	Vector bumpVector2(const float2& _pos) const
	{
		// bilinear transformation

		float2 pos = _pos * float2(width(), height());
		pos.x = floor(pos.x);
		pos.y = floor(pos.y);
		pos += float2(_TEXEL_CENTER_OFFS, _TEXEL_CENTER_OFFS);
		float4 c0 = lookupTexel(pos.x, pos.y);
		float4 c1 = lookupTexel(pos.x + 1.0, pos.y);
		float4 c2 = lookupTexel(pos.x, pos.y + 1.0);

		return Vector(0, 1, c2[0] - c0[0]);
	}
	virtual float width() const {
		
		
	}
	virtual float height() const {}
protected:
	//Correct the sampling address to be inside the texture
	virtual void fixAddress(float &_addr, float _max, TextureAddressMode _tam) const
	{
		if(_tam == TAM_Wrap) 
			_addr = fmodf(_addr, _max);
			if(_addr < 0.0)
				_addr = _max + _addr;
		else if(_tam == TAM_Border) 
			_addr = std::min(std::max(_addr, 0.f), _max);
	}

	//Lookup a texel using poin sampling. Coordinates are
	//	denormalized and texel center is at (0, 0) of image
	//	pixel's center
	virtual float4 lookupTexel(float _x, float _y) const
	{
		
	}
};


#endif //__INCLUDE_TEXTURE_BASICS
