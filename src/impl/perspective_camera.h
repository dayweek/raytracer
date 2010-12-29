#ifndef __INCLUDE_GUARD_8B7841BB_FFCF_42B9_BF24_2CD457380EAB
#define __INCLUDE_GUARD_8B7841BB_FFCF_42B9_BF24_2CD457380EAB
#ifdef _MSC_VER
	#pragma once
#endif

#include "core/algebra.h"
#include "rt/basic_definitions.h"

//A perspective camera implementation
class PerspectiveCamera : public Camera
{
	Point m_center;
	Vector m_topLeft;
	Vector m_stepX, m_stepY;
	
	void init(const Point &_center, const Vector &_forward, const Vector &_up, 
		float _vertOpeningAngInGrad, std::pair<uint, uint> _resolution)
	{
		m_center = _center;

		float aspect_ratio = (float)_resolution.first / (float)_resolution.second;

		Vector forward_axis = ~_forward;
		Vector right_axis = ~(forward_axis % _up);
		Vector up_axis = -~(right_axis % forward_axis);

		float angleInRad = _vertOpeningAngInGrad * (float)M_PI / 180.f;
		Vector row_vector = 2.f * right_axis * tanf(angleInRad / 2.f) * aspect_ratio;
		Vector col_vector = 2.f * up_axis * tanf(angleInRad / 2.f);

		m_stepX = row_vector / (float)_resolution.first;
		m_stepY = col_vector / (float)_resolution.second;
		m_topLeft = forward_axis - row_vector / 2.f - col_vector / 2.f;

	}

public:
	PerspectiveCamera(const Point &_center, const Vector &_forward, const Vector &_up, 
		float _vertOpeningAngInGrad, std::pair<uint, uint> _resolution)
	{
		init(_center, _forward, _up, _vertOpeningAngInGrad, _resolution);
	}

	PerspectiveCamera(const Point &_center, const Point &_lookAt, const Vector &_up, 
		float _vertOpeningAngInGrad, std::pair<uint, uint> _resolution)
	{
		init(_center, _lookAt - _center, _up, _vertOpeningAngInGrad, _resolution);
	}

	virtual Ray getPrimaryRay(float _x, float _y)
	{
		Ray ret;
		ret.o = m_center;
		ret.d = m_topLeft + _x * m_stepX + _y * m_stepY;

		return ret;
	}
};


#endif //__INCLUDE_GUARD_8B7841BB_FFCF_42B9_BF24_2CD457380EAB
