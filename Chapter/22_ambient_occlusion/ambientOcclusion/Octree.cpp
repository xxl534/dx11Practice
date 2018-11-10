#include "Octree.h"

Octree::Octree()
	:mRoot(NULL)
{

}

Octree::~Octree()
{
	SafeDelete(mRoot);
}

void Octree::Build(const std::vector<XMFLOAT3>& vertices, const std::vector<UINT>& indices)
{
	mVertices = vertices;

	XNA::AxisAlignedBox sceneBounds = BuildAABB();

	mRoot = new OctreeNode();
	mRoot->Bounds = sceneBounds;

	BuildOctree(mRoot, indices);
}

XNA::AxisAlignedBox Octree::BuildAABB()
{
	XMVECTOR vmin = XMVectorReplicate(+MathHelper::Infinity);
	XMVECTOR vmax = XMVectorReplicate(-MathHelper::Infinity);

	for (UINT i = 0; i < mVertices.size(); ++i)
	{
		XMVECTOR p = XMLoadFloat3(&mVertices[i]);

		vmin = XMVectorMin(vmin, p);
		vmax = XMVectorMax(vmax, p);
	}

	XNA::AxisAlignedBox bounts;
	XMVECTOR center = 0.5f*(vmin + vmax);
	XMVECTOR extent = 0.5f*(vmax - vmin);

	XMStoreFloat3(&bounts.Center, center);
	XMStoreFloat3(&bounts.Extents, extent);

	return bounts;
}

void Octree::BuildOctree(OctreeNode* parent, const std::vector<UINT>& indices)
{
	UINT triCount = indices.size() / 3;

	if (triCount < 60)
	{
		parent->IsLeaf = true;
		parent->indices = indices;
	}
	else
	{
		parent->IsLeaf = false;

		XNA::AxisAlignedBox subBox[8];

		parent->Subdivide(subBox);

		for (UINT i = 0; i < 8; ++i)
		{
			parent->Children[i] = new OctreeNode();
			parent->Children[i]->Bounds = subBox[i];

			std::vector<UINT> intersectedTriangleIndices;
			for (UINT j = 0; j < triCount; ++j)
			{
				UINT i0 = indices[j * 3 + 0];
				UINT i1 = indices[j * 3 + 1];
				UINT i2 = indices[j * 3 + 2];

				XMVECTOR v0 = XMLoadFloat3(&mVertices[i0]);
				XMVECTOR v1 = XMLoadFloat3(&mVertices[i1]);
				XMVECTOR v2 = XMLoadFloat3(&mVertices[i2]);

				if (XNA::IntersectTriangleAxisAlignedBox(v0, v1, v2, &subBox[i]))
				{
					intersectedTriangleIndices.push_back(i0);
					intersectedTriangleIndices.push_back(i1);
					intersectedTriangleIndices.push_back(i2);
				}
			}

			BuildOctree(parent->Children[i], intersectedTriangleIndices);
		}
	}
}

bool Octree::RayOctreeIntersect(FXMVECTOR rayPos, FXMVECTOR rayDir)
{
	return RayOctreeIntersect(mRoot, rayPos, rayDir);
}

bool Octree::RayOctreeIntersect(OctreeNode* parent, FXMVECTOR rayPos, FXMVECTOR rayDir)
{
	if (!parent->IsLeaf)
	{
		for (int i = 0; i < 8; ++i)
		{
			float t;
			if (XNA::IntersectRayAxisAlignedBox(rayPos, rayDir, &parent->Children[i]->Bounds, &t))
			{
				if (RayOctreeIntersect(parent->Children[i], rayPos, rayDir))
				{
					return true;
				}
			}
		}
		return false;
	}
	else
	{
		UINT triCount = parent->indices.size() / 3;

		for (UINT i = 0; i < triCount; ++i)
		{
			UINT i0 = parent->indices[i * 3 + 0];
			UINT i1 = parent->indices[i * 3 + 1];
			UINT i2 = parent->indices[i * 3 + 2];

			XMVECTOR v0 = XMLoadFloat3(&mVertices[i0]);
			XMVECTOR v1 = XMLoadFloat3(&mVertices[i1]);
			XMVECTOR v2 = XMLoadFloat3(&mVertices[i2]);

			float t;
			if (XNA::IntersectRayTriangle(rayPos, rayDir, v0, v1, v2, &t))
				return true;
		}

		return false;
	}
}

