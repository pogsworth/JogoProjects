#pragma once

#include "JMath.h"
#include "Bitmap.h"

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
		float uw;
		float vw;
		u8	bIsLit;
		u8	OutCode;

		static RenderVertex Lerp(const RenderVertex& a, const RenderVertex& b, float t)
		{
			RenderVertex result;
			result.color = Bitmap::LerpRGB(a.color, b.color, t);
			result.ViewPos = Vector3::Lerp(a.ViewPos, b.ViewPos, t);
			result.ScreenPos = Vector4::Lerp(a.ScreenPos, b.ScreenPos, t);

			return result;
		}

		Bitmap::VertexTexLit GetTexLitVertex()
		{
			Bitmap::VertexTexLit vtl = { ScreenPos, color, uw, vw };

			return vtl;
		}
	};

	struct MeshMaterial
	{
		Bitmap* Texture;
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

	Mesh CreateCube(float size);

	// Plane bits returned in the outcodes
	const u32 LEFT_PLANE	= 1;
	const u32 RIGHT_PLANE	= 2;
	const u32 TOP_PLANE		= 4;
	const u32 BOTTOM_PLANE	= 8;
	const u32 NEAR_PLANE	= 16;
	const u32 FAR_PLANE		= 32;

	struct Frustum
	{
		Plane planes[6];	// left, right, top, bottom, near, far

		u32 ClipCode(Vector3 pt, u32 MeshCode = 0x3f) const {
			u32 code = 0;
			u32 planeBit = LEFT_PLANE;

			for (u32 i = 0; i < 6; i++)
			{
				if (planeBit & MeshCode)
					if (Vector3::Dot(pt, planes[i].Normal) + planes[i].Distance < 0)
						code |= planeBit;
				planeBit <<= 1;
			}
			return code;
		}
		u64 ClipPoly(u32 numVerts, Vector3* pIn, Vector3* pOut);
	};

	struct Camera : public Matrix4
	{
		float FOV = 90.0f * D2R;
		float CotFOV = 1.0f;
		float HCotFOV = 1.0f;
		float AspectRatio = 1.0f;
		u32 TargetWidth = 1024;
		u32 TargetHeight = 1024;
		float NearZ = 1.0f;
		float FarZ = 1000.0f;
		float HalfWidth;
		float HalfHeight;
		float ProjectZ = 1;

		void SetProjection(float FOVdegrees, u32 Width, u32 Height, float _NearZ, float _FarZ)
		{
			FOV = FOVdegrees * D2R;
			CotFOV = 1.0f / tangent(FOV * 0.5f);
			TargetWidth = Width;
			TargetHeight = Height;
			AspectRatio = (float)Width / Height;
			HCotFOV = CotFOV / AspectRatio;
			NearZ = _NearZ;
			FarZ = _FarZ;
			HalfWidth = TargetWidth / 2.0f;
			HalfHeight = TargetHeight / 2.0f;
			ProjectZ = FarZ / (FarZ - NearZ);
		}

		Vector4 Project(Vector3& v) const;
		Frustum GetViewFrustum() const;
		u32 ClipCode(RenderVertex& v, u32 MeshCode = 0x3f) const;
		u32 ClipTriangle(const RenderVertex* pIn, RenderVertex*& pNewVerts, u16* TriIndices, u16* OutIndices, u32 TriOutCode) const;
	};

	void RenderMesh(const Mesh& mesh, const Matrix4&, const Camera&, const Bitmap&, const Bitmap&, Arena&);
};