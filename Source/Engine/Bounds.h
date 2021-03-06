#pragma once

#include <float.h>
#include "Math/REMath.h"

class BoxBounds
{
public:
	Vector4_3 min;
	Vector4_3 max;

	BoxBounds()
	: min(Vector4_3(FLT_MAX))
	, max(Vector4_3(-FLT_MAX))
	{}

	inline Vector4_3 GetCenter()
	{
		return (min + max) * 0.5f;
	}

	inline Vector4_3 GetExtent()
	{
		return (max - min) * 0.5f;
	}

	inline void SetCenterAndExtent(const Vector4_3& center, const Vector4_3& extent)
	{
		min = center - extent;
		max = center + extent;
	}

	inline bool IsInBounds(const Vector4_3& point)
	{
		Vec128 t0 = VecCmpLE(point.m128, max.m128);
		Vec128 t1 = VecCmpGE(point.m128, min.m128);
		int r = VecMoveMask(VecAnd(t0, t1));
		return (r & 0x7) == 0x7; // ignore w component
		//return point.x <= max.x && point.x >= min.x &&
		//	point.y <= max.y && point.y >= min.y &&
		//	point.z <= max.z && point.z >= min.z;
	}

	BoxBounds GetTransformedBounds(const Matrix4& inMat)
	{
		BoxBounds outBounds;
		//Vector4_3 boundsPoints[8];
		//GetPoints(boundsPoints);
		//for (int pIdx = 0; pIdx < 8; ++pIdx)
		//{
		//	outBounds += inMat.TransformPoint(boundsPoints[pIdx]);
		//}
		outBounds += inMat.TransformPoint(min);
		outBounds += inMat.TransformPoint(max);
		outBounds += inMat.TransformPoint(VecBlend(min.m128, max.m128, 0,0,1,0));
		outBounds += inMat.TransformPoint(VecBlend(min.m128, max.m128, 0,1,0,0));
		outBounds += inMat.TransformPoint(VecBlend(min.m128, max.m128, 1,0,0,0));
		outBounds += inMat.TransformPoint(VecBlend(min.m128, max.m128, 0,1,1,0));
		outBounds += inMat.TransformPoint(VecBlend(min.m128, max.m128, 1,0,1,0));
		outBounds += inMat.TransformPoint(VecBlend(min.m128, max.m128, 1,1,0,0));

		return outBounds;
	}

	// we don't do check here, expect an array of at least 8 elements
	void GetPoints(Vector4_3* outPoints)
	{
		outPoints[0] = min;
		outPoints[1] = max;
		outPoints[2] = VecBlend(min.m128, max.m128, 0,0,1,0);
		outPoints[3] = VecBlend(min.m128, max.m128, 0,1,0,0);
		outPoints[4] = VecBlend(min.m128, max.m128, 1,0,0,0);
		outPoints[5] = VecBlend(min.m128, max.m128, 0,1,1,0);
		outPoints[6] = VecBlend(min.m128, max.m128, 1,0,1,0);
		outPoints[7] = VecBlend(min.m128, max.m128, 1,1,0,0);
	}

	BoxBounds& operator+= (const Vector4_3& point)
	{
		min.m128 = VecMin(min.m128, point.m128);
		max.m128 = VecMax(max.m128, point.m128);
		//if (point.x < min.x) min.x = point.x;
		//if (point.y < min.y) min.y = point.y;
		//if (point.z < min.z) min.z = point.z;
		//if (point.x > max.x) max.x = point.x;
		//if (point.y > max.y) max.y = point.y;
		//if (point.z > max.z) max.z = point.z;
		return *this;
	}

	BoxBounds& operator+= (const BoxBounds& otherBounds)
	{
		min.m128 = VecMin(min.m128, otherBounds.min.m128);
		max.m128 = VecMax(max.m128, otherBounds.max.m128);
		//if (otherBounds.min.x < min.x) min.x = otherBounds.min.x;
		//if (otherBounds.min.y < min.y) min.y = otherBounds.min.y;
		//if (otherBounds.min.z < min.z) min.z = otherBounds.min.z;
		//if (otherBounds.max.x > max.x) max.x = otherBounds.max.x;
		//if (otherBounds.max.y > max.y) max.y = otherBounds.max.y;
		//if (otherBounds.max.z > max.z) max.z = otherBounds.max.z;
		return *this;
	}
};

class OrientedBoxBounds
{
public:
	// (X.x, Y.x, Z.x, C.x), (X.y, Y.y, Z.y, C.y), (X.z, Y.z, Z.z, C.z)
	Vector4 permutedAxisCenter[3];

	Vector4_3 center; // used for sphere intersect
	Vector4_3 extent; // used for sphere intersect

	// pass in scale for convenience
	void SetBounds(const BoxBounds& AABB, const Matrix4& modelMat, const Vector4_3& scale)
	{
		Vector4_3 centerAABB = (AABB.max + AABB.min) * 0.5f;
		Vector4_3 extentAABB = (AABB.max - AABB.min) * 0.5f;

		extent = extentAABB * scale;
		center = modelMat.TransformPoint(centerAABB);

		Matrix4 tmp = modelMat;
		tmp.SetTranslation(center);
		Vector4_3 rScale = Vector4(VecConst::Vec_One) / scale;
		tmp.ApplyScale(rScale);
		tmp = tmp.GetTransposed43();
		permutedAxisCenter[0] = tmp.mLine[0];
		permutedAxisCenter[1] = tmp.mLine[1];
		permutedAxisCenter[2] = tmp.mLine[2];

	}
};

class SphereBounds
{
public:
	Vector4 centerRadius; // radius in w

	void SetBounds(const Vector4_3& center, float radius)
	{
		centerRadius = Vector4(center, radius);
	}
};