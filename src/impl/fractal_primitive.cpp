#include "stdafx.h"
#include "fractallandscape.h"
#include "../core/util.h"

SmartPtr<Shader> FractalLandscape::Face::getShader(IntRet _intData) const
{
	SmartPtr<ExtHitPoint> hit = _intData.hitInfo;

	SmartPtr<PluggableShader> shader = m_fractal->shader->clone();

	shader->setPosition(Point::lerp(m_fractal->vertices(vert1x, vert1y), m_fractal->vertices(vert2x, vert2y), 
		m_fractal->vertices(vert3x, vert3y), hit->intResult.x, hit->intResult.y));

	
	Vector norm = 
		m_fractal->vertexNormals(vert1x, vert1y) * hit->intResult.x + 
		m_fractal->vertexNormals(vert2x, vert2y) * hit->intResult.y + 
		m_fractal->vertexNormals(vert3x, vert3y) * hit->intResult.z;
	
 	shader->setNormal(norm);


	float2 texPos = 
		m_fractal->textCoords(vert1x, vert1y) * hit->intResult.x +  
		m_fractal->textCoords(vert2x, vert2y) * hit->intResult.y +  
		m_fractal->textCoords(vert3x, vert3y) * hit->intResult.z;

	shader->setTextureCoord(texPos);


	return shader;

}


Primitive::IntRet FractalLandscape::Face::intersect(const Ray& _ray, float _previousBestDistance) const
{
	IntRet ret;

	float4 inter = 
		intersectTriangle(
			m_fractal->vertices(vert1x, vert1y), m_fractal->vertices(vert2x, vert2y), m_fractal->vertices(vert3x, vert3y), _ray
		);

	ret.distance = inter.w;

	if(inter.w < _previousBestDistance)
	{
		SmartPtr<ExtHitPoint> hit = new ExtHitPoint;
		ret.hitInfo = hit;
		hit->intResult = inter;
	}

	return ret;
}


BBox FractalLandscape::Face::getBBox() const
{

	BBox ret = BBox::empty();
	ret.extend(m_fractal->vertices(vert1x, vert1y));
	ret.extend(m_fractal->vertices(vert2x, vert2y));
	ret.extend(m_fractal->vertices(vert3x, vert3y));
	return ret;
}