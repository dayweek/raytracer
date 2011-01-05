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
		float4 t = float4(m_position) * scale;

		int x = (int)floor(t.x), y = (int)floor(t.y), z = (int)floor(t.z);
		x %= 2; y %= 2; z %= 2; 

		float col = (float)((6 + x + 0 + z) % 2);
		return float4::rep(col);
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

	RegularSampler regSamp;
	regSamp.addRef();
	regSamp.samplesX = 4;
	regSamp.samplesY = 4;

	RandomSampler randSamp;
	randSamp.addRef();
	randSamp.sampleCount = 16;

	StratifiedSampler stratSamp;
	stratSamp.addRef();
	stratSamp.samplesX = 4;
	stratSamp.samplesY = 4;

	HaltonSampleGenerator haltonSamp;
	haltonSamp.addRef();
	haltonSamp.sampleCount = 16;

	SmartPtr<Sampler> samplers[] = {&sampDef, &regSamp, &randSamp, &stratSamp};

	Renderer r;
	r.camera = &cam1;
	r.integrator = &integrator;
	r.target = &img;

	for(int s = 0; s < 4; s++)
	{
		r.sampler = samplers[s];
		r.render();
		std::stringstream ssm;
		ssm << "result_ex3_" << s << ".png" << std::flush;
		img.writePNG(ssm.str());

	}

}
