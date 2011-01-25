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

#ifndef WIDTH
#define WIDTH 1280
#endif

#ifndef HEIGHT
#define HEIGHT 960
#endif

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
			pls.falloff = float4(0, 0, 1, 0);
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

	Image img(WIDTH, HEIGHT);
	img.addRef();

	//Set up the scene
	GeometryGroup scene;

	// load scene
	LWObject objects;
	objects.read("models/cube.obj", true);
	objects.addReferencesToScene(scene.primitives);	
	scene.rebuildIndex();
	
	//apply custom shaders
	BumpTexturePhongShader as;
	as.addRef();
	Image  grass;
	grass.addRef();
	grass.readPNG("models/mat.png");
	Texture textureGrass;
	textureGrass.addRef();
	textureGrass.image = &grass;
	as.diffTexture = &textureGrass;
	as.amibientTexture = &textureGrass;
	as.specularCoef = float4::rep(0);
	as.specularExponent = 10000.f;
	as.transparency = float4::rep(0.9);
	FractalLandscape f(Point(-4419,-8000,-569), Point(3581,0, -569),9, 0.1, &as, 5.0f);
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
	Sphere sphere(Point(-78,1318,40), 25, &glass);;
	scene.primitives.push_back(&sphere);
	scene.rebuildIndex();	
	objects.materials[objects.materialMap["Glass"]].shader = &glass;

	
	//sample shader for noise
	ProceduralPhongShader skyShader;
	skyShader.addRef();
	CloudTexture nt;
	nt.addRef();
	skyShader.amibientNoiseTexture = &nt;
	skyShader.diffuseCoef = float4::rep(0.0f);
	skyShader.specularCoef = float4::rep(0.0f);

// 	float w = skyShader.amibientNoiseTexture->perlin->width;
 objects.materials[objects.materialMap["Sky"]].shader = &skyShader;

	//Set up the cameras
	PerspectiveCamera cam1(Point(-23, 1483, 30 ), forwardForCamera((0.0)*PI/180.0), Vector(0, 0, 1), 45,
		std::make_pair(img.width(), img.height()));
	
	cam1.addRef();

	//Set up the integrator
	IntegratorImpl integrator;
	integrator.addRef();
	integrator.scene = &scene;

	PointLightSource pls3;

	pls3.falloff = float4(0, 0, 1, 0);

	pls3.intensity  = float4::rep(0.9f);
	pls3.position = Point(299.5, 99, 518);
	integrator.lightSources.push_back(pls3);

// 	PointLightSource pls4;
// 
// 	pls4.falloff = float4(0, 0, 1, 0);
// 
// 	pls4.intensity  = float4::rep(0.9f);
// 	pls4.position = Point(1289.5, 99, 518);
// 	integrator.lightSources.push_back(pls4);
	
	areaLightSource(integrator, 0.9, 2, Point(-1180, -3860, -1718), 1000);
	integrator.ambientLight = float4::rep(0.1f);

	StratifiedSampler samp;
	samp.addRef();
 	samp.samplesX = 3;
	samp.samplesY = 3;

	//Render
	Renderer r;
	r.integrator = &integrator;
	r.target = &img;
	r.sampler = &samp;

	r.camera = &cam1;
	r.render();
	img.writePNG("result.png");
	
}

