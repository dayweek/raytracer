#include "stdafx.h"
#include "fractallandscape.h"
#include "../core/util.h"

SmartPtr<Shader> FractalLandscape::Face::getShader(IntRet _intData) const
{
	SmartPtr<ExtHitPoint> hit = _intData.hitInfo;

	SmartPtr<PluggableShader> shader = m_fractal->shader->clone();

	shader->setPosition(Point::lerp(m_fractal->vertices(vert1x, vert1y), m_fractal->vertices(vert2x, vert2y), 
		m_fractal->vertices(vert3x, vert3y), hit->intResult.x, hit->intResult.y));

	float hh;
	hh = m_fractal->vertices(vert3x, vert3y)[2];
	
	Vector norm = 
		m_fractal->vertexNormals(norm1x, norm1y) * hit->intResult.x + 
		m_fractal->vertexNormals(norm2x, norm2y) * hit->intResult.y + 
		m_fractal->vertexNormals(norm3x, norm3y) * hit->intResult.z;
	
 	shader->setNormal(norm);

// 	if(tex1 != -1 && tex2 != -1 && tex3 != -1)
// 	{
// 		float2 texPos = 
// 			m_fractal->texCoords[tex1] * hit->intResult.x +  
// 			m_fractal->texCoords[tex2] * hit->intResult.y +  
// 			m_fractal->texCoords[tex3] * hit->intResult.z;
// 
// 		shader->setTextureCoord(texPos);
// 	}

	
	//set pu, pv for bump mapping
// 	shader->setPuPv(m_fractal->vertices(vert1x, vert1y), m_fractal->vertices(vert2x, vert2y),m_fractal->vertices(vert3x, vert3y),
// 			m_fractal->texCoords[tex1], m_fractal->texCoords[tex2], m_fractal->texCoords[tex3]);
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