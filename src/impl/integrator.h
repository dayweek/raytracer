#ifndef __INCLUDE_GUARD_D2CAE848_1C1E_4362_8DFE_163B2EA2A97D
#define __INCLUDE_GUARD_D2CAE848_1C1E_4362_8DFE_163B2EA2A97D
#ifdef _MSC_VER
	#pragma once
#endif

struct PointLightSource
{
	Point position;
	float4 intensity, falloff;
	//falloff formula: (.x  / dist^2 + .y / dist + .z) * intensity;
};


class IntegratorImpl : public Integrator
{
public:
	GeometryGroup *scene;
	std::vector<PointLightSource> lightSources;
	float4 ambientLight;

	virtual float4 getRadiance(const Ray &_ray)
	{
		float4 col = float4::rep(0);
		if(!terminate || count != 0) {
			count = count - 1;
			Primitive::IntRet ret = scene->intersect(_ray, FLT_MAX);
			if(ret.distance < FLT_MAX && ret.distance >= Primitive::INTEPS())
			{
				SmartPtr<Shader> shader = scene->getShader(ret);
				if(shader.data() != NULL)
				{
					col += shader->getAmbientCoefficient() * ambientLight;

					Point intPt = _ray.o + ret.distance * _ray.d;

					for(std::vector<PointLightSource>::const_iterator it = lightSources.begin(); it != lightSources.end(); it++)
						if(visibleLS(intPt, it->position))
						{
							Vector lightD = it->position - intPt;
							float4 refl = shader->getReflectance(-_ray.d, lightD);
							float dist = lightD.len();
							float fallOff = it->falloff.x / (dist * dist) + it->falloff.y / dist + it->falloff.z;
							col += refl * float4::rep(fallOff) * it->intensity;
						}

					col += shader->getIndirectRadiance(-_ray.d, this);
				}
			}
		} else {
			terminate = false;
		}

		return col;
	}
private:

	bool visibleLS(const Point& _pt, const Point& _pls)
	{
		Ray r;
		r.o = _pt;
		r.d = _pls - _pt;

		Primitive::IntRet ret = scene->intersect(r, 1.1f);
		return ret.distance <= Primitive::INTEPS() || ret.distance >= 1 - Primitive::INTEPS();
	}

};


#endif //__INCLUDE_GUARD_D2CAE848_1C1E_4362_8DFE_163B2EA2A97D
