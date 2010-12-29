#ifndef __INCLUDE_GUARD_3862487A_DF63_478D_99C2_652B7C66442E
#define __INCLUDE_GUARD_3862487A_DF63_478D_99C2_652B7C66442E
#ifdef _MSC_VER
	#pragma once
#endif

#include "../rt/basic_definitions.h"
#include "../rt/bvh.h"
#include "shading_basics.h"

//A group of geometry. Since it is a primitive, it can also
//	contain other groups, thus creating a hierarchy.
class GeometryGroup : public Primitive
{
	//Intersection info which encapsulates the intersection info
	//	of a contained primitive, toghether with a reference to the primitive
	struct GGHitPoint : public RefCntBase
	{
		Primitive::IntRet intRet;
		Primitive* innerPrimitive;
	};

	//A BVH over the bounded primitives
	BVH m_bvh;

	//The list of not bounded primitives
	std::vector<Primitive *> m_nonIdxPrimitives;

public:
	std::vector<Primitive *> primitives;

	virtual SmartPtr<Shader> getShader(IntRet _intData) const;
	virtual IntRet intersect(const Ray& _ray, float _previousBestDistance ) const;
	virtual BBox getBBox() const;

	//Rebuilds the BVH and updated m_nonIdxPrimitives
	void rebuildIndex();
};

#endif //__INCLUDE_GUARD_3862487A_DF63_478D_99C2_652B7C66442E
