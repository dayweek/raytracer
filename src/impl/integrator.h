#ifndef __INCLUDE_GUARD_D2CAE848_1C1E_4362_8DFE_163B2EA2A97D
#define __INCLUDE_GUARD_D2CAE848_1C1E_4362_8DFE_163B2EA2A97D
#ifdef _MSC_VER
	#pragma once
#endif
#include "../impl/phong_shaders.h"
#include <limits>

struct PointLightSource
{
	Point position;
	float4 intensity, falloff;
	//falloff formula: (.x  / dist^2 + .y / dist + .z) * intensity;
};


struct DepthStateKey
{
	typedef int t_data;
};

class IntegratorImpl : public Integrator
{
public:
	enum {_MAX_BOUNCES = 7};
	GeometryGroup *scene;
	std::vector<PointLightSource> lightSources;
	float4 ambientLight;

	IntegratorImpl()
	{
		state.value<DepthStateKey>() = 0;
		end_contribution = 1.0f;
	}
	
	virtual float4 getRadiance(const Ray &_ray)
	{
		//state.value<DepthStateKey>()++;

		float4 col = float4::rep(0);

		//if(state.value<DepthStateKey>() < _MAX_BOUNCES)
		if(end_contribution > 0.05f)
		{
			Primitive::IntRet ret = scene->intersect(_ray, FLT_MAX);
			if(ret.distance < FLT_MAX && ret.distance >= Primitive::INTEPS())
			{
				SmartPtr<PluggableShader> shader = scene->getShader(ret);
				if(shader.data() != NULL)
				{
					col += shader->getAmbientCoefficient() * ambientLight;

					Point intPt = _ray.o + ret.distance * _ray.d;

					for(std::vector<PointLightSource>::const_iterator it = lightSources.begin(); it != lightSources.end(); it++)
					{
// 						float4 trans = getTotalTransparency(intPt, it->position, shader->transparency);

						Vector lightD = it->position - intPt;
						float4 refl = shader->getReflectance(-_ray.d, lightD);
						float dist = lightD.len();
						float fallOff = it->falloff.x / (dist * dist) + it->falloff.y / dist + it->falloff.z;
						col +=  refl * float4::rep(fallOff) * it->intensity;
					}

					col += shader->getIndirectRadiance(-_ray.d, this);
				}
			}
		}

		//state.value<DepthStateKey>()--;

		return col;
	}

	virtual float4 getTotalTransparency(const Point& _pt, const Point& _pls, float4 initial_transparency)
	{
		Ray r;
		float4 t = initial_transparency;
		r.d = ~(_pls - _pt);
		float total_distance = (_pt - _pls).len();
		float distance = Primitive::INTEPS();
		Point pt = _pt;
		Primitive::IntRet ret;
		while (distance < total_distance)
			{			
				
				ret = scene->intersect(r, FLT_MAX);
				if(ret.distance < std::numeric_limits<float>::max() && ret.distance > Primitive::INTEPS()) {
					distance += ret.distance + Primitive::INTEPS();
					if(distance < total_distance) {
						SmartPtr<PluggableShader> p = scene->getShader(ret);
						t *= p->transparency;
					}
					else
						distance = total_distance;
				}
				else
					distance = total_distance;
				r.o = _pt + distance * r.d;
				
			}
		return t;
	}


};


#endif //__INCLUDE_GUARD_D2CAE848_1C1E_4362_8DFE_163B2EA2A97D
