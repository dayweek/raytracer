#include "stdafx.h"
#include "lwobject.h"
#include "../core/util.h"

SmartPtr<Shader> LWObject::Face::getShader(IntRet _intData) const
{
	SmartPtr<ExtHitPoint> hit = _intData.hitInfo;

	SmartPtr<PluggableShader> shader = m_lwObject->materials[material].shader->clone();

	shader->setPosition(Point::lerp(m_lwObject->vertices[vert1], m_lwObject->vertices[vert2], 
		m_lwObject->vertices[vert3], hit->intResult.x, hit->intResult.y));
	
	float4 bary = hit->intResult;

	//TODO: Implement texture and normal interpolation
	//Calculate the normal
	Vector e1 = m_lwObject->vertices[vert2] - m_lwObject->vertices[vert1];
	Vector e2 = m_lwObject->vertices[vert3] - m_lwObject->vertices[vert1];
	Vector norm = ~(~e1 % ~e2);
	norm = bary[0]*m_lwObject->normals[norm1] + bary[1]*m_lwObject->normals[norm2] + bary[2]*m_lwObject->normals[norm3];
	
	shader->setNormal(norm);
	float2 texPos = bary[0]*m_lwObject->texCoords[tex1] + bary[1]*m_lwObject->texCoords[tex2] + bary[2]*m_lwObject->texCoords[tex3];
	shader->setTextureCoord(texPos);
	//set pu, pv for bump mapping
	shader->setPuPv(m_lwObject->vertices[vert1], m_lwObject->vertices[vert2],m_lwObject->vertices[vert3],
			m_lwObject->texCoords[tex1], m_lwObject->texCoords[tex2], m_lwObject->texCoords[tex3]);

	return shader;
}


Primitive::IntRet LWObject::Face::intersect(const Ray& _ray, float _previousBestDistance) const
{
	IntRet ret;

	float4 inter = 
		intersectTriangle(
			m_lwObject->vertices[vert1], m_lwObject->vertices[vert2], m_lwObject->vertices[vert3], _ray
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


BBox LWObject::Face::getBBox() const
{

	BBox ret = BBox::empty();
	ret.extend(m_lwObject->vertices[vert1]);
	ret.extend(m_lwObject->vertices[vert2]);
	ret.extend(m_lwObject->vertices[vert3]);

	return ret;
}

void LWObject::addReferencesToScene(std::vector<Primitive*> &_scene) const
{
	for(std::vector<Face>::const_iterator it = faces.begin(); it != faces.end(); it++)
		_scene.push_back((Primitive*)&*it);
}
