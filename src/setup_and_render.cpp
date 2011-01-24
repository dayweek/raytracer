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
#include "rt/noise_textures.h"

// for rotating camera
#define PI 3.14159265

// creates area light sourse in a square. coordinates of the lights have the same z coordinate.
// density - how many lights. total number = density^2
void areaLightSource(IntegratorImpl &_int, float intensity, int density, Point corner, float width)
{
	//Build area light source
	PointLightSource pls;
	float x1 = corner[0];
	float x2 = corner[0] + width; 
	float y1 = corner[1]; 
	float y2 = corner[1] + width;
	float xstep = x2 - x1 / density;
	float xstart = x1 + xstep/2;
	float ystep = y2 - y1 / density;
	float ystart = y1 + ystep/2;
	//we distribute the original intensity
	float4 disintensity = float4::rep(intensity/(density * density));
	for(int y = 1; y < density + 1; y++) {
		for(int x = 1; x < density + 1; x++) {
			pls.falloff = float4(1, 0, 0, 0);
			pls.intensity  = disintensity;
			pls.position = Point(xstart + x*xstep, ystart + y*ystep, corner[2]);
			_int.lightSources.push_back(pls);
		}
			
	}	
}

//for camera synchronization with a modeling program
Vector forwardForCamera(float angleX)
{
	Vector vectors[3];
	vectors[0] = Vector(1,0,0);
	vectors[1] = Vector(0,cos(angleX),sin(angleX));
	vectors[2] = Vector(0,-sin(angleX),cos(angleX));
	Matrix rotationXMatrix(vectors);
	return (rotationXMatrix) * Vector(0,-1,0);
}

void setup_and_render()
{
	Image img(800, 600);
	img.addRef();

	//Set up the scene
	GeometryGroup scene;

	// load scene
	LWObject objects;
	objects.read("models/cube.obj", true);
	objects.addReferencesToScene(scene.primitives);	
	scene.rebuildIndex();
	
	//apply custom shaders
	DefaultPhongShader as;
	as.addRef();
	as.diffuseCoef = float4(0.2f, 0.2f, 0.4f, 0);
	as.ambientCoef = as.diffuseCoef;
	as.specularCoef = float4::rep(0);
	as.specularExponent = 10000.f;
	as.transparency = float4::rep(0.9);
	FractalLandscape f(Point(-5000,-11500,-540), Point(5000,-1500,-540),9, 0.1);
	f.shader = &as;
	f.addReferencesToScene(scene.primitives);
	scene.rebuildIndex();
	
	// my phong
	RRPhongShader glass;
	glass.n1 = 1.0f;
	glass.n2 = 1.5f;
	glass.diffuseCoef = float4(0.1, 0.1, 0.1, 0);
	glass.ambientCoef = glass.diffuseCoef;
	glass.specularCoef = float4::rep(0.9);
	glass.specularExponent = 10000;
	glass.transparency = float4::rep(0.9);
	glass.addRef();
// 	Sphere sphere(Point(-2,4,3), 1, &sh);;
// 	scene.primitives.push_back(&sphere);
// 	scene.rebuildIndex();	
	objects.materials[objects.materialMap["Glass"]].shader = &glass;

	
	//sample shader for noise
	ProceduralPhongShader skyShader;
	CloudTexture nt;
	nt.addRef();
	skyShader.amibientNoiseTexture = &nt;
	skyShader.diffNoiseTexture = &nt;
	skyShader.specularCoef = float4::rep(1.0f);
	skyShader.specularExponent = 10000.f;
	skyShader.addRef();
// 	float w = skyShader.amibientNoiseTexture->perlin->width;
 objects.materials[objects.materialMap["Sky"]].shader = &skyShader;
	
 	//Enable bi-linear filtering on the walls
//	((BumpTexturePhongShader*)cow.materials[cow.materialMap["Stones"]].shader.data())->diffTexture->filterMode = Texture::TFM_Point;


	//Set up the cameras
	PerspectiveCamera cam1(Point(-34, 1660,15 ), forwardForCamera((-6.0)*PI/180.0), Vector(0, 0, 1), 45,
		std::make_pair(img.width(), img.height()));
	
	cam1.addRef();

	//Set up the integrator
	IntegratorImpl integrator;
	integrator.addRef();
	integrator.scene = &scene;


	PointLightSource pls3;

	pls3.falloff = float4(1, 0, 0, 0);

	pls3.intensity  = float4::rep(0.9f);
	pls3.position = Point(-234, 586,395);
	integrator.lightSources.push_back(pls3);

	areaLightSource(integrator, 0.9, 2, Point(-4300,-9900, 7200), 1500);
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
	
}

