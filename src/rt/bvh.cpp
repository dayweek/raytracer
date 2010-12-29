#include "stdafx.h"
#include "bvh.h"

namespace bvh_build_internal
{
	struct BuildStateStruct
	{
		size_t segmentStart, segmentEnd;
		size_t nodeIndex;
		BBox centroidBBox;
	};

	struct CentroidWithID
	{
		Point centroid;
		size_t origIndex;
	};
}

using namespace bvh_build_internal;

//An iterative split in the middle build for BVHs
void BVH::build(const std::vector<Primitive*> &_objects)
{
	std::vector<BBox> objectBBoxes(_objects.size());

	BuildStateStruct curState;
	curState.centroidBBox = BBox::empty();

	std::vector<CentroidWithID> centroids(objectBBoxes.size());

	for(size_t i = 0; i < objectBBoxes.size(); i++)
	{
		objectBBoxes[i] = _objects[i]->getBBox();
		
	    centroids[i].centroid = objectBBoxes[i].min.lerp(objectBBoxes[i].max, 0.5f);
	    centroids[i].origIndex = i;
	    curState.centroidBBox.extend(centroids[i].centroid);
	}

	curState.segmentStart = 0;
	curState.segmentEnd = objectBBoxes.size();
	curState.nodeIndex = 0;
	m_nodes.resize(1);

	std::stack<BuildStateStruct> buildStack;

	const float _EPS = 0.0000001f;

	for(;;)
	{
		
	    size_t objCnt = curState.segmentEnd - curState.segmentStart;
		Vector boxDiag = curState.centroidBBox.diagonal();
		
		int splitDim = boxDiag.x > boxDiag.y ? (boxDiag.x > boxDiag.z ? 0 : 2) : (boxDiag.y > boxDiag.z ? 1 : 2);

		if(fabs(boxDiag[splitDim]) < _EPS || objCnt < 3)
		{
			//Create a leaf
			const size_t NODE_TYPE_MASK = ((size_t)1 << Node::LEAF_FLAG_BIT);
			m_nodes[curState.nodeIndex].bbox = BBox::empty();
			m_nodes[curState.nodeIndex].dataIndex = m_leafData.size() | NODE_TYPE_MASK;

			for(size_t i = curState.segmentStart; i < curState.segmentEnd; i++)
			{
				m_nodes[curState.nodeIndex].bbox.extend(objectBBoxes[centroids[i].origIndex]);
				m_leafData.push_back(_objects[centroids[i].origIndex]);
			}

			m_leafData.push_back(NULL);

			if(buildStack.empty())
				break;

			curState = buildStack.top();
			buildStack.pop();

			continue;
		}

		float splitVal = (curState.centroidBBox.min[splitDim] + curState.centroidBBox.max[splitDim]) / 2.f;
		size_t leftPtr = curState.segmentStart, rightPtr = curState.segmentEnd - 1;

		BBox nodeBBox = BBox::empty();
		BuildStateStruct rightState;
		curState.centroidBBox = BBox::empty();
		rightState.centroidBBox = BBox::empty();

		while(leftPtr < rightPtr)
		{
			while(leftPtr < curState.segmentEnd && centroids[leftPtr].centroid[splitDim] <= splitVal)
			{
				curState.centroidBBox.extend(centroids[leftPtr].centroid);
				nodeBBox.extend(objectBBoxes[centroids[leftPtr].origIndex]);
				leftPtr++;
			}

			while(rightPtr >= curState.segmentStart && centroids[rightPtr].centroid[splitDim] > splitVal)
			{
				rightState.centroidBBox.extend(centroids[rightPtr].centroid);
				nodeBBox.extend(objectBBoxes[centroids[rightPtr].origIndex]);
				rightPtr--;
			}

			if(leftPtr >= rightPtr)
				break;

			std::swap(centroids[leftPtr], centroids[rightPtr]);
		}

		m_nodes[curState.nodeIndex].bbox = nodeBBox;
		m_nodes[curState.nodeIndex].dataIndex = m_nodes.size();

		_ASSERT(leftPtr > curState.segmentStart && leftPtr < curState.segmentEnd);
		_ASSERT(leftPtr == rightPtr + 1);

		rightState.segmentStart = leftPtr;
		rightState.segmentEnd = curState.segmentEnd;
		curState.segmentEnd = leftPtr;
		curState.nodeIndex = m_nodes.size();
		rightState.nodeIndex = curState.nodeIndex + 1;
		
		buildStack.push(rightState);

		m_nodes.resize(rightState.nodeIndex + 1);
	}
}

//Recursive intersection
BVH::IntersectionReturn BVH::intersect(const Ray &_ray, float _previousBestDistance) const
{
	Primitive::IntRet bestHit;
	bestHit.distance = _previousBestDistance;

	Primitive *bestPrimitive = NULL;

	std::stack<size_t> traverseStack;

	size_t curNode = 0;
	size_t closestFace = (size_t)-1;

	for(;;)
	{
		const BVH::Node& node = m_nodes[curNode];
		if(node.isLeaf())
		{
			size_t idx = node.getLeftChildOrLeaf();
			while(m_leafData[idx] != NULL)
			{
				Primitive::IntRet curRet = m_leafData[idx]->intersect(_ray, bestHit.distance);

				if(curRet.distance > Primitive::INTEPS() && curRet.distance < bestHit.distance)
				{
					bestHit = curRet;
					bestPrimitive = m_leafData[idx];
				}

				idx++;
			}

			if(traverseStack.empty())
				break;

			curNode = traverseStack.top();
			traverseStack.pop();
		}
		else
		{
			const BVH::Node &leftNode = m_nodes[node.getLeftChildOrLeaf()];
			const BVH::Node &rightNode = m_nodes[node.getLeftChildOrLeaf() + 1];

			std::pair<float, float> intLeft = leftNode.bbox.intersect(_ray);
			std::pair<float, float> intRight = rightNode.bbox.intersect(_ray);
			intLeft.first = std::max(Primitive::INTEPS(), intLeft.first);
			intRight.first = std::max(Primitive::INTEPS(), intRight.first);
			intLeft.second = std::min(intLeft.second, bestHit.distance);
			intRight.second = std::min(intRight.second, bestHit.distance);

			bool descendLeft = intLeft.first < intLeft.second + Primitive::INTEPS();
			bool descendRight = intRight.first < intRight.second + Primitive::INTEPS();

			if(descendLeft && !descendRight)
				curNode = node.getLeftChildOrLeaf();
			else if(descendRight && !descendLeft)
				curNode = node.getLeftChildOrLeaf() + 1;
			else if(descendLeft && descendRight)
			{
				curNode = node.getLeftChildOrLeaf();
				size_t farNode = curNode + 1;
				if(intLeft.first > intRight.first)
					std::swap(curNode, farNode);
				traverseStack.push(farNode);
			}
			else
			{
				if(traverseStack.empty())
					break;

				curNode = traverseStack.top();
				traverseStack.pop();
			}
		}
	}

	BVH::IntersectionReturn ret;
	ret.ret = bestHit;
	ret.primitive = bestPrimitive;
	return ret;
}

