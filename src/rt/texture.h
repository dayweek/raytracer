#ifndef __INCLUDE_GUARD_6A0F8987_914D_41B8_8E51_53B29CF1A045
#define __INCLUDE_GUARD_6A0F8987_914D_41B8_8E51_53B29CF1A045
#ifdef _MSC_VER
	#pragma once
#endif

#include "../rt/texture_basics.h"
#include "../core/image.h"

//A texture class
class Texture : public TextureBase
{

public:
	SmartPtr<Image> image;
private:
	float4 lookupTexel(float _x, float _y) const
	{
		float realX = _x, realY = _y;
		fixAddress(realX, width(), addressModeX);
		fixAddress(realY, height(), addressModeY);

		uint x = (uint)floor(realX);
		uint y = (uint)floor(realY);
		
		return (*image)(x, y);
	}
	float width() const
	{
		return (float)image->width();
	}
	float height() const
	{
		return (float)image->height();
	}
};


#endif //__INCLUDE_GUARD_6A0F8987_914D_41B8_8E51_53B29CF1A045
