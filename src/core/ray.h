#ifndef __RAY_H_INCLUDED_CA4668D0_F683_4D3D_B7EB_63AC9E7F7633
#define __RAY_H_INCLUDED_CA4668D0_F683_4D3D_B7EB_63AC9E7F7633
#ifdef _MSC_VER
	#pragma once
#endif

#include "algebra.h"

//A ray class
struct Ray
{
	Point o; //origin
	Vector d; //direction

	Ray() {}
	Ray(const Point &_o, const Vector &_d)
		: o(_o), d(_d) {}

	Point getPoint(float _distance)
	{
		return o + _distance * d;
	}
};




#endif //__RAY_H_INCLUDED_CA4668D0_F683_4D3D_B7EB_63AC9E7F7633
