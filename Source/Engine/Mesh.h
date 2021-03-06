#pragma once

#include "Containers/Containers.h"
#include "Math/REMath.h"
#include "Math/UVector.h"
#include "Material.h"
#include "Bounds.h"

struct Vertex
{
	UVector3 position;
	UVector3 normal;
	UVector4 tangent;
	UVector2 texCoords;

	const static GLint positionIdx	= 0;
	const static GLint normalIdx	= 1;
	const static GLint tangentIdx	= 2;
	const static GLint texCoordsIdx	= 3;

	Vertex(UVector3 inPosition,
		UVector3 inNormal,
		UVector4 inTangent,
		UVector2 inTexCoords)
	{
		position = inPosition;
		normal = inNormal;
		tangent = inTangent;
		texCoords = inTexCoords;
	}

	void YUpToZUp()
	{
		float tmp = position.z;
		position.z = position.y;
		position.y = -tmp;
		tmp = normal.z;
		normal.z = normal.y;
		normal.y = -tmp;
		tmp = tangent.z;
		tangent.z = tangent.y;
		tangent.y = -tmp;
	}
};

class MeshData
{
public:
	static REArray<MeshData*> gMeshDataContainer;

	static MeshData* Create()
	{
		MeshData* md = new MeshData();
		gMeshDataContainer.push_back(md);
		return md;
	}

	MeshData() :
	VAO(0)
	, VBO(0)
	, EBO(0)
	, bHasResource(0)
	{}

	void CacheCount()
	{
		vertCount = (GLsizei)vertices.size();
		idxCount = (GLsizei)indices.size();
	}

	void InitResource();

	REArray<Vertex> vertices;
	REArray<GLuint> indices;
	GLsizei vertCount;
	GLsizei idxCount;

	BoxBounds bounds;

	bool bHasResource;
	GLuint VAO, VBO, EBO;
};

class Mesh
{
public:

	static REArray<Mesh*> gMeshContainer;

	static Mesh* Create()
	{
		Mesh* mesh = new Mesh();
		gMeshContainer.push_back(mesh);
		return mesh;
	}

	static Mesh* Create(MeshData* inMeshData, Material* inMaterial = 0)
	{
		Mesh* mesh = Create();
		mesh->Init(inMeshData, inMaterial);
		return mesh;
	}

	MeshData* meshData;
	Material* material;

	void Init(MeshData* inMeshData, Material* inMaterial);
	void Draw(struct RenderContext& renderContext, Material* overrideMaterial = 0) const;
};

struct MeshRenderData
{
public:
	Matrix4 prevModelMat;	// 16 x 4
	Matrix4 modelMat;		// 16 x 4
	Material* material;		// 8
	GLuint VAO;				// 4
	GLsizei idxCount;		// 4
	float distToCamera;		// 4

	static bool CompareAlphaBlend(const MeshRenderData& a, const MeshRenderData& b)
	{
		return a.distToCamera > b.distToCamera;
	}
	static bool CompareOpaque(const MeshRenderData& a, const MeshRenderData& b)
	{
		return a.distToCamera < b.distToCamera;
	}

	void Draw(struct RenderContext& renderContext, Material* overrideMaterial = 0) const;
};

static void MakeCube(MeshData& meshData)
{
	REArray<Vertex>& vertList = meshData.vertices;
	REArray<GLuint>& idxList = meshData.indices;
	Vector4_2 uv[4] =
	{
		Vector4_2(0, 0),
		Vector4_2(0, 1),
		Vector4_2(1, 1),
		Vector4_2(1, 0)
	};
	Vector4_3 normal[6] =
	{
		Vector4_3(1, 0, 0),
		Vector4_3(0, 1, 0),
		Vector4_3(0, 0, 1),
		Vector4_3(-1, 0, 0),
		Vector4_3(0, -1, 0),
		Vector4_3(0, 0, -1),
	};
	Vector4_3 up[6] =
	{
		Vector4_3(0, 1, 0),
		Vector4_3(0, 0, -1),
		Vector4_3(0, 1, 0),
		Vector4_3(0, 1, 0),
		Vector4_3(0, 0, 1),
		Vector4_3(0, 1, 0),
	};
	vertList.reserve(24);
	idxList.reserve(36);
	// +x -> +y -> +z -> -x -> -y -> -z
	for (int i = 0; i < 6; ++i)
	{
		Vector4_3 right = up[i].Cross3(normal[i]);
		Vector4 tangent(right, 1.f);
		for (int j = 0; j < 4; ++j)
		{
			Vector4_2 ext = uv[j] * 2.f - 1.f;;
			vertList.push_back(Vertex(normal[i] + right * ext.x - up[i] * ext.y, normal[i], tangent, uv[j]));
		}
		idxList.push_back(i * 4 + 0);
		idxList.push_back(i * 4 + 1);
		idxList.push_back(i * 4 + 2);
		idxList.push_back(i * 4 + 0);
		idxList.push_back(i * 4 + 2);
		idxList.push_back(i * 4 + 3);
	}

#if RE_UP_AXIS == RE_Z_UP
	for (int i = 0, ni = (int)vertList.size(); i < ni; ++i)
		vertList[i].YUpToZUp();
#endif

	meshData.CacheCount();
}

static void MakeSphere(MeshData& meshData, int div)
{
	REArray<Vertex>& vertList = meshData.vertices;
	REArray<GLuint>& idxList = meshData.indices;

	int latDiv = div / 2 + 1;

	int vertCount = 2 + (div + 1) * (latDiv - 2);
	int idxCount = div * ((latDiv - 3) * 2 + 2) * 3;

	vertList.reserve(vertCount);
	idxList.reserve(idxCount);

	// add first vert
	vertList.push_back(Vertex(Vector4_3(0, 1, 0), Vector4_3(0, 1, 0), Vector4(1, 0, 0, 1), Vector4_2(0, 0)));
	int prevRingStart = 0;
	int curRingStart = 1;
	// add rings
	for (int iLat = 1; iLat < latDiv - 1; ++iLat)
	{
		// (-1, 1)
		float latRatio = (float)iLat / (float)(latDiv - 1);
		float latAngle = (1 - latRatio * 2) * 0.5f * PI;
		float cosLat = Cos(latAngle);
		float sinLat = Sin(latAngle);
		// add vert on a ring
		for (int iLon = 0; iLon < div + 1; ++iLon)
		{
			float lonRatio = (float)iLon / (float)div;
			float lonAngle = lonRatio * 2 * PI;
			float cosLon = Cos(lonAngle);
			float sinLon = Sin(lonAngle);
			Vector4_3 pos(cosLon * cosLat, sinLat, sinLon * cosLat);
			Vector4 tangent(sinLon, 0, -cosLon, 1);
			vertList.push_back(Vertex(pos, pos, tangent, Vector4_2(lonRatio, latRatio)));
		}
		// add idx on a ring with prev ring
		for (int iLon = 0; iLon < div; ++iLon)
		{
			int next = iLon + 1;
			if (iLat == 1)
			{
				// first ring
				idxList.push_back(prevRingStart);
				idxList.push_back(curRingStart + next);
				idxList.push_back(curRingStart + iLon);
			}
			else
			{
				idxList.push_back(prevRingStart + iLon);
				idxList.push_back(curRingStart + next);
				idxList.push_back(curRingStart + iLon);
				idxList.push_back(prevRingStart + iLon);
				idxList.push_back(prevRingStart + next);
				idxList.push_back(curRingStart + next);
			}
		}
		prevRingStart = curRingStart;
		curRingStart += (div + 1);
	}
	// add last vert
	vertList.push_back(Vertex(Vector4_3(0, -1, 0), Vector4_3(0, -1, 0), Vector4(1, 0, 0, 1), Vector4_2(1, 1)));
	// add idx for last ring
	for (int iLon = 0; iLon < div; ++iLon)
	{
		int next = (iLon + 1);
		idxList.push_back(prevRingStart + iLon);
		idxList.push_back(prevRingStart + next);
		idxList.push_back(curRingStart);
	}

#if RE_UP_AXIS == RE_Z_UP
	for (int i = 0, ni = (int)vertList.size(); i < ni; ++i)
		vertList[i].YUpToZUp();
#endif

	assert(vertList.size() == vertCount);
	assert(idxList.size() == idxCount);
	meshData.CacheCount();
}

static Vector4_3 GetTangent(Vector4_3 normal)
{
	Vector4_3 up(0, 1, 0);
	Vector4_3 tangent = up.Cross3(normal);
	if (tangent.SizeSqr3() < KINDA_SMALL_NUMBER)
		return Vector4_3(1, 0, 0);
	return tangent.GetNormalized3();
}

static void MakeCone(MeshData& meshData, int firstRingVertCount, int level)
{
	REArray<Vertex>& vertList = meshData.vertices;
	REArray<GLuint>& idxList = meshData.indices;

	Vector4_3 mainAxis(0, 0, -1);
	Vector4_3 secAxis(0, 1, 0);
	Vector4_3 thrAxis = mainAxis.Cross3(secAxis);

	static const float normScaler = 1.f / Sqrt(2.f);

	// first vert = level 0
	vertList.push_back(Vertex(Vector4_3(0, 0, 0), -mainAxis, Vector4(thrAxis, 1), Vector4_2(0, 0)));
	int prevRingStart = 0;
	int curRingStart = 1;

	int ringVertCount = firstRingVertCount;
	for (int levelIdx = 1; levelIdx <= level; ++levelIdx)
	{
		float mainAxisValue = (float)levelIdx / (float)(level);
		// vert
		for (int ringIdx = 0; ringIdx <= ringVertCount; ++ringIdx)
		{
			float ratio = (float)ringIdx / (float)ringVertCount;
			float angle = ratio * 2 * PI;
			float cosA = Cos(angle);
			float sinA = Sin(angle);
			Vector4_3 pos = (mainAxis + secAxis * cosA + thrAxis * sinA) * mainAxisValue;
			Vector4_3 normal = (-mainAxis + secAxis * cosA + thrAxis * sinA) * normScaler;
			Vector4_3 tangent = secAxis * sinA + thrAxis * -cosA;

			vertList.push_back(Vertex(pos, normal, Vector4(tangent, 1), Vector4_2(ratio, mainAxisValue)));
		}

		// idx
		if (levelIdx == 1)
		{
			for (int ringIdx = 1; ringIdx <= ringVertCount; ++ringIdx)
			{
				idxList.push_back(prevRingStart);
				idxList.push_back(curRingStart + ringIdx);
				idxList.push_back(curRingStart + ringIdx - 1);
			}
		}
		else
		{
			for (int ringIdx = 2; ringIdx <= ringVertCount; ringIdx += 2)
			{
				int prevRingIdx = ringIdx / 2;
				idxList.push_back(prevRingStart + prevRingIdx - 1);
				idxList.push_back(curRingStart + ringIdx - 1);
				idxList.push_back(curRingStart + ringIdx - 2);
				idxList.push_back(prevRingStart + prevRingIdx - 1);
				idxList.push_back(prevRingStart + prevRingIdx);
				idxList.push_back(curRingStart + ringIdx - 1);
				idxList.push_back(prevRingStart + prevRingIdx);
				idxList.push_back(curRingStart + ringIdx);
				idxList.push_back(curRingStart + ringIdx - 1);
			}
		}

		prevRingStart = curRingStart;
		curRingStart += (ringVertCount + 1);

		if (levelIdx < level)
			ringVertCount *= 2;
	}

	// bottom
	for (int ringIdx = 0; ringIdx <= ringVertCount; ++ringIdx)
	{
		float ratio = (float)ringIdx / (float)ringVertCount;
		float angle = ratio * 2 * PI;
		float cosA = Cos(angle);
		float sinA = Sin(angle);
		Vector4_3 pos = (mainAxis + secAxis * cosA + thrAxis * sinA);
		Vector4_3 tangent = secAxis * sinA + thrAxis * -cosA;

		vertList.push_back(Vertex(pos, mainAxis, Vector4(tangent, 1), Vector4_2(ratio, 1)));
	}

	prevRingStart = curRingStart;
	curRingStart += (ringVertCount + 1);

	// final vert
	vertList.push_back(Vertex(mainAxis, mainAxis, Vector4(thrAxis, 1), Vector4_2(0, 0)));

	// idx
	for (int ringIdx = 1; ringIdx <= ringVertCount; ++ringIdx)
	{
		idxList.push_back(prevRingStart + ringIdx - 1);
		idxList.push_back(prevRingStart + ringIdx);
		idxList.push_back(curRingStart);
	}

#if RE_UP_AXIS == RE_Z_UP
	for (int i = 0, ni = (int)vertList.size(); i < ni; ++i)
		vertList[i].YUpToZUp();
#endif

	meshData.CacheCount();
}

static int FindNewVert(const REArray<Vector4_3>& newVertTable, int i0, int i1)
{
	for (int i = 0; i < newVertTable.size(); ++i)
	{
		if ((newVertTable[i].x == i0 && newVertTable[i].y == i1) ||
			(newVertTable[i].x == i1 && newVertTable[i].y == i0))
			return (int)newVertTable[i].z;
	}
	return -1;
}

static void MakeIcosahedron(MeshData& meshData, int tesLevel)
{
	REArray<Vertex>& vertList = meshData.vertices;
	REArray<GLuint>& idxList = meshData.indices;

	vertList.empty();
	idxList.empty();

	int tesPower = 1 << (2 * tesLevel); // = 4 ^ tes
	vertList.reserve(10 * tesPower + 2); // 12 + 30 + 30 * 4 + ... + 30 * 4 ^ (tes-1) = 12 + 30 * (4 ^ tes - 1) / (4 - 1) = 10 * 4 ^ tes + 2
	idxList.reserve(60 * tesPower);

	static const float t = (1.f + Sqrt(5.f)) * 0.5f;
	static const float s = 1.f / Sqrt(1.f + t*t);

	static const Vector4_3 originVert[12] =
	{
		Vector4_3(1, t, 0) * s,
		Vector4_3(-1, t, 0) * s,
		Vector4_3(-1, -t, 0) * s,
		Vector4_3(1, -t, 0) * s,
		Vector4_3(t, 0, 1) * s,
		Vector4_3(t, 0, -1) * s,
		Vector4_3(-t, 0, -1) * s,
		Vector4_3(-t, 0, 1) * s,
		Vector4_3(0, 1, t) * s,
		Vector4_3(0, -1, t) * s,
		Vector4_3(0, -1, -t) * s,
		Vector4_3(0, 1, -t) * s,
	};

	static const int originIdx[60]
	{
		0, 8, 4,
		0, 4, 5,
		0, 5, 11,
		0, 11, 1,
		0, 1, 8,
		1, 11, 6,
		1, 6, 7,
		1, 7, 8,
		2, 9, 7,
		2, 7, 6,
		2, 6, 10,
		2, 10, 3,
		2, 3, 9,
		3, 10, 5,
		3, 5, 4,
		3, 4, 9,
		4, 8, 9,
		5, 10, 11,
		6, 11, 10,
		7, 9, 8,
	};

	for (int i = 0; i < 12; ++i)
	{
		vertList.push_back(Vertex(originVert[i], originVert[i], Vector4(GetTangent(originVert[i]), 1), Vector4_2(0, 0)));
	}

	for (int i = 0; i < 60; ++i)
		idxList.push_back(originIdx[i]);

	for (int tesIdx = 0; tesIdx < tesLevel; ++tesIdx)
	{
		int idxCount = (int)idxList.size();
		int newVertStartIdx = (int)vertList.size();

		int newVertCount = idxCount / 2;
		REArray<Vector4_3> newVertTable;
		newVertTable.reserve(newVertCount);

		for (int faceIdx = 0; faceIdx < idxCount; faceIdx += 3)
		{
			int i0 = idxList[faceIdx + 0];
			int i1 = idxList[faceIdx + 1];
			int i2 = idxList[faceIdx + 2];
			Vector4_3 v0 = vertList[i0].position.ToVector4();
			Vector4_3 v1 = vertList[i1].position.ToVector4();
			Vector4_3 v2 = vertList[i2].position.ToVector4();
			// new idx
			int i3 = FindNewVert(newVertTable, i0, i1);
			int i4 = FindNewVert(newVertTable, i1, i2);
			int i5 = FindNewVert(newVertTable, i2, i0);
			// new vert
			if (i3 < 0)
			{
				Vector4_3 v3 = ((v0 + v1) * 0.5f).GetNormalized3();
				vertList.push_back(Vertex(v3, v3, Vector4(GetTangent(v3), 1), Vector4_2(0, 0)));
				i3 = newVertStartIdx;
				++newVertStartIdx;
				newVertTable.push_back(Vector4_3((float)i0, (float)i1, (float)i3));
			}
			if (i4 < 0)
			{
				Vector4_3 v4 = ((v1 + v2) * 0.5f).GetNormalized3();
				vertList.push_back(Vertex(v4, v4, Vector4(GetTangent(v4), 1), Vector4_2(0, 0)));
				i4 = newVertStartIdx;
				++newVertStartIdx;
				newVertTable.push_back(Vector4_3((float)i1, (float)i2, (float)i4));
			}
			if (i5 < 0)
			{
				Vector4_3 v5 = ((v2 + v0) * 0.5f).GetNormalized3();
				vertList.push_back(Vertex(v5, v5, Vector4(GetTangent(v5), 1), Vector4_2(0, 0)));
				i5 = newVertStartIdx;
				++newVertStartIdx;
				newVertTable.push_back(Vector4_3((float)i2, (float)i0, (float)i5));
			}

			// change origin idx
			idxList[faceIdx + 0] = i0;
			idxList[faceIdx + 1] = i3;
			idxList[faceIdx + 2] = i5;
			// add idx
			idxList.push_back(i3);
			idxList.push_back(i1);
			idxList.push_back(i4);
			idxList.push_back(i3);
			idxList.push_back(i4);
			idxList.push_back(i5);
			idxList.push_back(i5);
			idxList.push_back(i4);
			idxList.push_back(i2);
		}
	}

#if RE_UP_AXIS == RE_Z_UP
	for (int i = 0, ni = (int)vertList.size(); i < ni; ++i)
		vertList[i].YUpToZUp();
#endif

	meshData.CacheCount();
}

// view space quad
static void MakeQuadVS(MeshData& meshData)
{
	REArray<Vertex>& vertList = meshData.vertices;
	REArray<GLuint>& idxList = meshData.indices;

	vertList.reserve(4);
	idxList.reserve(6);

	vertList.push_back(Vertex(Vector4_3(-1, 1, 0),	Vector4_3(0, 0, -1), Vector4(1, 0, 0, 1), Vector4_2(0, 1)));
	vertList.push_back(Vertex(Vector4_3(-1, -1, 0), Vector4_3(0, 0, -1), Vector4(1, 0, 0, 1), Vector4_2(0, 0)));
	vertList.push_back(Vertex(Vector4_3(1, -1, 0),	Vector4_3(0, 0, -1), Vector4(1, 0, 0, 1), Vector4_2(1, 0)));
	vertList.push_back(Vertex(Vector4_3(1, 1, 0),	Vector4_3(0, 0, -1), Vector4(1, 0, 0, 1), Vector4_2(1, 1)));

	idxList.push_back(0);
	idxList.push_back(1);
	idxList.push_back(2);
	idxList.push_back(0);
	idxList.push_back(2);
	idxList.push_back(3);

	meshData.CacheCount();
}
