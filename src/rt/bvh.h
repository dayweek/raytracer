#ifndef __INCLUDE_GUARD_8D5E74D9_FBD2_4B91_88E1_716ECFC377C4
#define __INCLUDE_GUARD_8D5E74D9_FBD2_4B91_88E1_716ECFC377C4
#ifdef _MSC_VER
	#pragma once
#endif

#include "../core/memory.h"
#include "basic_definitions.h"

//A bounding volume hierarchy
class BVH
{

	struct Node
	{
		enum {LEAF_FLAG_BIT = sizeof(size_t) * 8 - 1};

		BBox bbox;
		size_t dataIndex;

		bool isLeaf() const
		{
			const size_t NODE_TYPE_MASK = ((size_t)1 << LEAF_FLAG_BIT);
			return (dataIndex & NODE_TYPE_MASK) != 0;
		}

		size_t getLeftChildOrLeaf() const
		{
			const size_t INDEX_MASK = ((size_t)1 << LEAF_FLAG_BIT) - 1;
			return dataIndex & INDEX_MASK;
		}

	};

	std::vector<Node> m_nodes;
	std::vector<Primitive*> m_leafData;

public:
	struct IntersectionReturn
	{
		Primitive *primitive;
		Primitive::IntRet ret;
	};

	//Builds the hierarchy over a set of bounded primitives
	void build(const std::vector<Primitive*> &_objects);

	//Intersects a ray with the BVH.
	IntersectionReturn intersect(const Ray &_ray, float _previousBestDistance) const;

	BBox getSceneBBox() const { return m_nodes[0].bbox; };
};

#endif //__INCLUDE_GUARD_8D5E74D9_FBD2_4B91_88E1_716ECFC377C4
