#pragma once
#include "d3dUtil.h"
#include "xnacollision.h"

struct OctreeNode;

class Octree
{
public:
	Octree();
	~Octree();

	void Build(const std::vector<XMFLOAT3>& vertices, const std::vector<UINT>& indices);
	bool RayOctreeIntersect(FXMVECTOR rayPos, FXMVECTOR rayDir);
private:
	XNA::AxisAlignedBox BuildAABB();
	void BuildOctree(OctreeNode* parent, const std::vector<UINT>& indices);
	bool RayOctreeIntersect(OctreeNode* parent, FXMVECTOR rayPos, FXMVECTOR rayDir);
private:
	OctreeNode* mRoot;

	std::vector<XMFLOAT3> mVertices;
};

struct OctreeNode
{
	XNA::AxisAlignedBox Bounds;

	//This will be empth except for leaf nodes.
	std::vector<UINT> indices;

	OctreeNode* Children[8];

	bool IsLeaf;

	OctreeNode()
	{
		for (UINT i = 0; i < 8; ++i)
		{
			Children[i] = NULL;
		}

		Bounds.Center = XMFLOAT3(0.f, 0.f, 0.f);
		Bounds.Extents = XMFLOAT3(0.f, 0.f, 0.f);

		IsLeaf = false;
	}

	~OctreeNode()
	{
		for (UINT i = 0; i < 8; ++i)
		{
			SafeDelete(Children[i]);
		}
	}

	///<summary>
	/// Subdivides the bounding box of this node into eight subboxes (vMin[i], vMax[i]) for i = 0:7.
	///</summary>
	void Subdivide(XNA::AxisAlignedBox box[8])
	{
		XMFLOAT3 halfExtent(
			0.5f*Bounds.Extents.x,
			0.5f*Bounds.Extents.y,
			0.5f*Bounds.Extents.z);

		// "Top" four quadrants.
		box[0].Center = XMFLOAT3(
			Bounds.Center.x + halfExtent.x,
			Bounds.Center.y + halfExtent.y,
			Bounds.Center.z + halfExtent.z);
		box[0].Extents = halfExtent;

		box[1].Center = XMFLOAT3(
			Bounds.Center.x - halfExtent.x,
			Bounds.Center.y + halfExtent.y,
			Bounds.Center.z + halfExtent.z);
		box[1].Extents = halfExtent;

		box[2].Center = XMFLOAT3(
			Bounds.Center.x - halfExtent.x,
			Bounds.Center.y + halfExtent.y,
			Bounds.Center.z - halfExtent.z);
		box[2].Extents = halfExtent;

		box[3].Center = XMFLOAT3(
			Bounds.Center.x + halfExtent.x,
			Bounds.Center.y + halfExtent.y,
			Bounds.Center.z - halfExtent.z);
		box[3].Extents = halfExtent;

		// "Bottom" four quadrants.
		box[4].Center = XMFLOAT3(
			Bounds.Center.x + halfExtent.x,
			Bounds.Center.y - halfExtent.y,
			Bounds.Center.z + halfExtent.z);
		box[4].Extents = halfExtent;

		box[5].Center = XMFLOAT3(
			Bounds.Center.x - halfExtent.x,
			Bounds.Center.y - halfExtent.y,
			Bounds.Center.z + halfExtent.z);
		box[5].Extents = halfExtent;

		box[6].Center = XMFLOAT3(
			Bounds.Center.x - halfExtent.x,
			Bounds.Center.y - halfExtent.y,
			Bounds.Center.z - halfExtent.z);
		box[6].Extents = halfExtent;

		box[7].Center = XMFLOAT3(
			Bounds.Center.x + halfExtent.x,
			Bounds.Center.y - halfExtent.y,
			Bounds.Center.z - halfExtent.z);
		box[7].Extents = halfExtent;
	}
};
