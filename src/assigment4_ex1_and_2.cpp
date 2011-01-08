#include "stdafx.h"

#include "core/image.h"
#include "rt/basic_definitions.h"
#include "rt/geometry_group.h"

#include "impl/lwobject.h"
#include "impl/phong_shaders.h"
#include "impl/basic_primitives.h"
#include "impl/perspective_camera.h"
#include "impl/integrator.h"
#include "rt/renderer.h"
#include "impl/samplers.h"
#include "impl/fractallandscape.h"

class BumpMirrorPhongShader : public DefaultPhongShader
{
protected:
	float2 m_texCoord; 
	Point m_position;
public:
	float reflCoef;

	virtual void setPosition(const Point& _point) { m_position = _point; }

	//Set the tangent to (0, 1, 0)
	virtual void setNormal(const Vector& _normal) { DefaultPhongShader::setNormal(_normal); }
	virtual void setTextureCoord(const float2& _texCoord) { m_texCoord = _texCoord;}

	virtual Vector getNormal() const 
	{ 
		Vector ret = m_normal;
		float x = m_texCoord.x - floor(m_texCoord.x) - 0.5f;
		float y = m_texCoord.y - floor(m_texCoord.y) - 0.5f;
		if(sqrt(pow(x,2) + pow(y,2)) <= 0.25f) {
			float centerz = sqrt(pow(2.5f,2) -pow(0.25f,2));
			return (~(Vector(x,y,centerz)));
		}
		return ret;
	}

	virtual float4 getIndirectRadiance(const Vector &_out, Integrator *_integrator) const
	{ 
		Ray r;
		r.o = m_position;

		Vector n = getNormal();
		Vector v = n * fabs(n * ~_out);
		r.d = _out + 2 * (v - _out);

		return _integrator->getRadiance(r) * float4::rep(reflCoef);
	}


	_IMPLEMENT_CLONE(BumpMirrorPhongShader);

};




void assigment4_1_and_2()
{
	
	Image img(800, 600);
	img.addRef();

	//Set up the scene
	GeometryGroup scene;
// 	LWObject cow;
// 	cow.read("models/cow.obj", true);
// 	cow.addReferencesToScene(scene.primitives);
// 	LWObject sc;
// 	sc.read("models/concrete.obj", true);
// 	sc.addReferencesToScene(scene.primitives);
	
	DefaultAmbientShader as;
	as.addRef();
	as.ambientCoefficient = float4(1.0, 0.5, 0.5, 0.0f);
	FractalLandscape f(Point(1,0,1), Point(0,1,1), (uint)2);
	f.shader = &as;
	f.addReferencesToScene(scene.primitives);
	
	// my phong
	RRPhongShader sh;
	sh.n1 = 1.0f;
	sh.n2 = 1.5f;
	sh.diffuseCoef = float4(0.1, 0.1, 0.1, 0);
	sh.ambientCoef = sh.diffuseCoef;
	sh.specularCoef = float4::rep(1.0);
	sh.specularExponent = 100;
	sh.addRef();
// 	Sphere sphere(Point(-2,4,3), 1, &sh);;
// 	scene.primitives.push_back(&sphere);
	scene.rebuildIndex();

	BumpMirrorPhongShader sh4;
	sh4.diffuseCoef = float4(0.2f, 0.2f, 0, 0);
	sh4.ambientCoef = sh4.diffuseCoef;
	sh4.specularCoef = float4::rep(0.8f);
	sh4.specularExponent = 10000.f;
	sh4.reflCoef = 0.4f;
	sh4.setNormal(Vector(0,1,0));
	sh4.addRef();
	//cow.materials[cow.materialMap["Floor"]].shader = &sh4;
	
 	//Enable bi-linear filtering on the walls
// 	((BumpTexturePhongShader*)cow.materials[cow.materialMap["Stones"]].shader.data())->diffTexture->filterMode = Texture::TFM_Point;
//  	((BumpTexturePhongShader*)cow.materials[cow.materialMap["Stones"]].shader.data())->amibientTexture->filterMode = Texture::TFM_Point;
// 	((BumpTexturePhongShader*)cow.materials[cow.materialMap["Stones"]].shader.data())->bumpTexture->filterMode = Texture::TFM_Point;
// 	((BumpTexturePhongShader*)sc.materials[sc.materialMap["Mat"]].shader.data())->diffTexture->addressModeX = Texture::TAM_Wrap;
// 	((BumpTexturePhongShader*)sc.materials[sc.materialMap["Mat"]].shader.data())->diffTexture->addressModeY = Texture::TAM_Wrap;


	//Set up the cameras
	PerspectiveCamera cam1(Point(-9.398149f, -6.266083f, 5.348377f), Point(-6.324413f, -2.961229f, 4.203216f), Vector(0, 0, 1), 30,
		std::make_pair(img.width(), img.height()));

	PerspectiveCamera cam2(Point(2.699700f, 6.437226f, 0.878297f), Point(4.337114f, 8.457443f,- 0.019007f), Vector(0, 0, 1), 30,
		std::make_pair(img.width(), img.height()));
	
	cam1.addRef();
	cam2.addRef();

	//Set up the integrator
	IntegratorImpl integrator;
	integrator.addRef();
	integrator.scene = &scene;
	PointLightSource pls;

	pls.falloff = float4(0, 0, 1, 0);

	pls.intensity  = float4::rep(0.9f);
	pls.position = Point(-2.473637f, 3.119330f,4.571486f);
	integrator.lightSources.push_back(pls);

	integrator.ambientLight = float4::rep(0.1f);

	DefaultSampler samp;
	samp.addRef();
// 	samp.sampleCount = 16;

	//Render
	Renderer r;
	r.integrator = &integrator;
	r.target = &img;
	r.sampler = &samp;

	r.camera = &cam1;
	r.render();
	img.writePNG("result_cam1.png");

	//For seeing the difference in texture filtering
// 	r.camera = &cam2;
// 	r.render();
// 	img.writePNG("result_cam2.png");
}

