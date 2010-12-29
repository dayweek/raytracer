#include "stdafx.h"

#include "core/image.h"
#include "rt/basic_definitions.h"
#include "impl/basic_primitives.h"
#include "impl/perspective_camera.h"
#include "impl/samplers.h"


struct EyeLightIntegrator : public Integrator
{
public:
	Primitive *scene;

	virtual float4 getRadiance(const Ray &_ray)
	{
		float4 col = float4::rep(0);

		Primitive::IntRet ret = scene->intersect(_ray, FLT_MAX);
		if(ret.distance < FLT_MAX && ret.distance >= Primitive::INTEPS())
		{
			SmartPtr<Shader> shader = scene->getShader(ret);
			if(shader.data() != NULL)
				col += shader->getReflectance(-_ray.d, -_ray.d);
		}

		return col;
	}
};

//A check board shader in 3D, dependent only of the position in space
class CheckBoard3DShader : public PluggableShader
{
	Point m_position;
public:
	//Only the x, y, and z are taken into account
	float4 scale;
	virtual void setPosition(const Point& _point) {m_position = _point;}
	virtual float4 getReflectance(const Vector &_outDir, const Vector &_inDir) const
	{
		int steps = (int)((m_position[0] * scale[0])) + (int)((m_position[1] * scale[1])) + (uint)((m_position[2] * scale[2]));
		if(m_position[0] < 0)
			steps += 1;
		if(m_position[1] < 0)
			steps += 1;
		if(m_position[2] < 0)
			steps += 1;
		if(steps % 2 == 0) {
			return float4::rep(.0f);
		} else {
			return float4::rep(1.0f);
		}
	}

	_IMPLEMENT_CLONE(CheckBoard3DShader);
};




void assigment4_ex3()
{
	Image img(800, 600);
	img.addRef();

	CheckBoard3DShader sh;
	sh.scale = float4::rep(4);
	sh.addRef();

	InfinitePlane p1(Point(0, 0.1f, 0), Vector(0, 1, 0), &sh);

	PerspectiveCamera cam1(Point(0, 2, 10), Vector(0, 0, -1), Vector(0, 1, 0), 60,
		std::make_pair(img.width(), img.height()));
	cam1.addRef();

	EyeLightIntegrator integrator;
	integrator.addRef();
	integrator.scene = &p1;

	DefaultSampler sampDef;
	sampDef.addRef();	
	RegularSampler sampReg;
	sampReg.addRef();sampReg.m = 3;
	RandomSampler sampRand;
	sampRand.addRef();sampRand.n = 9;
	StratifiedSampler sampStrat;
	sampStrat.addRef();sampStrat.m = 3;
	
	SmartPtr<Sampler> samplers[] = {&sampDef, &sampReg, &sampRand, &sampStrat};

	Renderer r;
	r.camera = &cam1;
	r.integrator = &integrator;
	r.target = &img;

	for(int s = 0; s < 4; s++)
	{
		r.sampler = samplers[s];
		r.render();
		std::stringstream ssm;
		ssm << "result_ex3_" << s + 1 << ".png" << std::flush;
		img.writePNG(ssm.str());

	}

}
