#pragma once
#include "JMath.h"

namespace Jogo
{
	struct MeshVertex
	{
		Vector3 Pos;
		Vector3 Normal;
		float u;
		float v;
	};

	struct RenderVertex
	{
		Vector3 ViewPos;
		Vector3 ViewNormal;
		Vector4 ScreenPos;
		u32 color;
		float u;
		float v;
		u8	bIsList;
		u8	OutCode;
	};

	struct Mesh
	{
		u32 NumVerts;
		MeshVertex* Verts;
		u32 NumTris;

		// if NumVerts < 64K, then SmallIndices, else BigIndices
		union
		{
			u16* SmallIndices;
			u32* BigIndices;
		};

		Vector3 MinAABB;
		Vector3 MaxAABB;

		u32 AABBOutCode;
	};

	void RenderMesh(Mesh& mesh);
}