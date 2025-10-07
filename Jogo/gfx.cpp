#include "gfx.h"
#include "Bitmap.h"
#include "Arena.h"

namespace Jogo
{

	u32 ClipAABB(Vector3 min, Vector3 max, Matrix4& MVT, Frustum& ViewFrustum, float& MinZ, u32& OrCode)
	{
		// transform mesh AABB and abort if all out
		Vector3 origin = min * MVT;
		float xaxis = max.x - min.x;
		float yaxis = max.y - min.y;
		float zaxis = max.z - min.z;

		Matrix3 VecTransform = (Matrix3)MVT;
		Vector3 axes[3] = {
			{ Vector3{ xaxis, 0.0f, 0.0f } *VecTransform},
			{ Vector3{ 0.0f, yaxis, 0.0f } *VecTransform},
			{ Vector3{ 0.0f, 0.0f, zaxis } *VecTransform}
		};

		MinZ = origin.z;

		OrCode = 0;
		u32 AndCode = 0x3f;
		for (u32 i = 0; i < 8; i++)
		{
			Vector3 corner = origin;
			if (i & 1)
				corner += axes[0];
			if (i & 2)
				corner += axes[1];
			if (i & 4)
				corner += axes[2];

			u32 code = ViewFrustum.ClipCode(corner);
			OrCode |= code;
			AndCode &= code;

			MinZ = corner.z < MinZ ? corner.z : MinZ;
		}

		return AndCode;
	}

	/*
	*
	*	From https://www.cbloom.com/3d/techdocs/pipeline.txt
	*
	*	The first thing we do is concatenate the Model->World (a parameter) and
	*	World->View (a global) transforms. Each transform is a 4x4 matrix with
	*	{0,0,0,1} in the bottom row (that is, a 3x3 matrix and a 3-vector
	*	translation), all 3d vectors have an implicit {1} in the final spot when
	*	treated as a 4d vector.
	*	1) The result is the Model->View transform (MVT).
	*
	*	2) Then we create an orthonormalized version of the MVT using Gram-Schmidt
	*	reduction.
	*
	*	3) We then transform all of the source verts in the VB
	*	(xyz,rgba,uv,normal) into View space, using the MVT, and rotate all the
	*	normals into view space using the normalized MVT (so that the normals
	*	stay normalized!), filling out the ViewSpace Position & Normal in the PV
	*	array. This is just a bunch of matrix-vertex multiplies.
	*
	*
	*/

	u32 AmbientColor = 0x202020;
	u32 SunlightColor = 0xf0f080;
	Vector3 SunDir{ -.577f, .577f, -.577f };

	void LightVertex(RenderVertex& v)
	{
		if (v.bIsLit)
			return;
		float dot = max(Vector3::Dot(v.ViewNormal, SunDir), 0.5f);
		Bitmap::FloatRGBA frgba = Bitmap::GetFloatColor(SunlightColor);
		frgba.r *= dot;
		frgba.g *= dot;
		frgba.b *= dot;
		v.color = Bitmap::GetColorFromFloatRGB(frgba);
		v.bIsLit = true;
	}

	// Maybe Camera, that has VT, Frustum, Projection
	void RenderMesh(const Mesh& mesh, const Matrix4& ModelToWorld, const Camera& camera, Bitmap& Target, const Bitmap& Texture, Arena& arena, bool fillTL)
	{
		// build MVT transform
		Matrix4 View = camera.GetInverse();;
		Matrix4 MVT = ModelToWorld * View;

		Frustum ViewFrustum = camera.GetViewFrustum();

		float ViewMinZ;
		// early out if the mesh bbox is completely out any of the frustum planes
		u32 AABBOutCode = 0;
		if (ClipAABB(mesh.MinAABB, mesh.MaxAABB, MVT, ViewFrustum, ViewMinZ, AABBOutCode))
			return;

		Matrix3 NormalMVT = (Matrix3)MVT;
		NormalMVT.Normalize();

		RenderVertex* RenderVerts = (RenderVertex*)arena.Allocate((3 * mesh.NumVerts) * sizeof(RenderVertex));
		u16* VisibleTris = (u16*)arena.Allocate(mesh.NumTris * 3 * 2 * sizeof(u16));

		RenderVertex* VertIter = RenderVerts;
		for (u32 i = 0; i < mesh.NumVerts; i++, VertIter++)
		{
			VertIter->ViewPos = mesh.Verts[i].Pos * MVT;
			VertIter->ViewNormal = mesh.Verts[i].Normal * NormalMVT;
			VertIter->ScreenPos = camera.Project(VertIter->ViewPos);
			VertIter->u = mesh.Verts[i].u;
			VertIter->v = mesh.Verts[i].v;
			VertIter->uw = mesh.Verts[i].u * VertIter->ScreenPos.w;
			VertIter->vw = mesh.Verts[i].v * VertIter->ScreenPos.w;
			VertIter->bIsLit = false;
			VertIter->OutCode = AABBOutCode ? camera.ClipCode(*VertIter, AABBOutCode) : 0;
		}

		// loop over all triangles (indices)
		// test if 3 verts are all out same 
		// OutCode & OutCode & OutCode != 0 then skip this triangle - it is clipped
		u16* ShortIndices = mesh.SmallIndices;
		u16* VisibleTriIter = VisibleTris;

		for (u32 i = 0; i < mesh.NumTris; i++, ShortIndices += 3)
		{
			RenderVertex& p = RenderVerts[ShortIndices[0]];
			RenderVertex& q = RenderVerts[ShortIndices[1]];
			RenderVertex& r = RenderVerts[ShortIndices[2]];

			if (p.OutCode & q.OutCode & r.OutCode)
				continue;

			u32 OrCode = p.OutCode | q.OutCode | r.OutCode;

			// backface check
			if (OrCode & NEAR_PLANE)
			{
				// do backface check in view space
				Vector3 Edge1 = q.ViewPos - p.ViewPos;
				Vector3 Edge2 = r.ViewPos - p.ViewPos;
				Vector3 TriNormal = Vector3::Cross(Edge1, Edge2);
				if (Vector3::Dot(p.ViewPos, TriNormal) >= 0.0f)
					continue;
			}
			else
			{
				// do backface check in screen space
				if ((r.ScreenPos.x - p.ScreenPos.x) * (q.ScreenPos.y - p.ScreenPos.y) >=
					(r.ScreenPos.y - p.ScreenPos.y) * (q.ScreenPos.x - p.ScreenPos.x))
					continue;       
			}

			// Light the vertices
			if (!p.bIsLit)
				LightVertex(p);
			if (!q.bIsLit)
				LightVertex(q);
			if (!r.bIsLit)
				LightVertex(r);

			if (!OrCode)
			{
				*VisibleTriIter++ = ShortIndices[0];
				*VisibleTriIter++ = ShortIndices[1];
				*VisibleTriIter++ = ShortIndices[2];
			}
			else
			{
				u16 ClippedIndices[10];
				u32 NumVerts = camera.ClipTriangle(RenderVerts, VertIter, ShortIndices, ClippedIndices, OrCode);
				if (NumVerts > 2)
				{
					// create a triangle fan of the resulting indices
					u32 NumTris = NumVerts - 2;
					for (u32 j = 0; j < NumTris; j++)
					{
						*VisibleTriIter++ = ClippedIndices[0];
						*VisibleTriIter++ = ClippedIndices[j + 1];
						*VisibleTriIter++ = ClippedIndices[j + 2];
					}
				}
			}
		}

		for (u16* TriIter = VisibleTris; TriIter < VisibleTriIter; TriIter += 3)
		{
			RenderVertex& p = RenderVerts[TriIter[0]];
			RenderVertex& q = RenderVerts[TriIter[1]];
			RenderVertex& r = RenderVerts[TriIter[2]];

			Bitmap::VertexTexLit tri[3] =
			{
				{ p.ScreenPos, p.color, p.uw, p.vw },
				{ q.ScreenPos, q.color, q.uw, q.vw },
				{ r.ScreenPos, r.color, r.uw, r.vw },
			};
			if (fillTL)
			{
				Target.FillTriangleTexLitInt(p.GetTexLitVertex(), q.GetTexLitVertex(), r.GetTexLitVertex(), Texture);
			}
			else
			{
				//else
				//	Target.FillTriangle(p.GetTexLitVertex(), q.GetTexLitVertex(), r.GetTexLitVertex(), Texture);
				// Bitmap::VertexLit tri[3] = {
				//	{p.ScreenPos.x, p.ScreenPos.y, p.color},
				//	{q.ScreenPos.x, q.ScreenPos.y, q.color},
				//	{r.ScreenPos.x, r.ScreenPos.y, r.color},
				//};
				//Target.FillTriangle(tri);	// tri, tri + 1, tri + 2);
				Target.DrawLine((s32)p.ScreenPos.x, (s32)p.ScreenPos.y, (s32)q.ScreenPos.x, (s32)q.ScreenPos.y, 0);
				Target.DrawLine((s32)q.ScreenPos.x, (s32)q.ScreenPos.y, (s32)r.ScreenPos.x, (s32)r.ScreenPos.y, 0);
				Target.DrawLine((s32)r.ScreenPos.x, (s32)r.ScreenPos.y, (s32)p.ScreenPos.x, (s32)p.ScreenPos.y, 0);
			}
		}
	}


	// TODO: pass in an outcode for this 
	u64 Frustum::ClipPoly(u32 numVerts, Vector3* pIn, Vector3* pOut)
	{
		u64 verts = numVerts;

		// TODO: use scratch allocation from an arena instead of alloca
		Vector3* a = (Vector3*)alloca(sizeof(Vector3) * (numVerts + 6));

		verts = planes[0].ClipPoly(verts, pIn, a);
		if (!verts) return 0;
		verts = planes[1].ClipPoly(verts, a, pOut);
		if (!verts) return 0;
		verts = planes[2].ClipPoly(verts, pOut, a);
		if (!verts) return 0;
		verts = planes[3].ClipPoly(verts, a, pOut);
		if (!verts) return 0;
		verts = planes[4].ClipPoly(verts, pOut, a);
		if (!verts) return 0;
		verts = planes[5].ClipPoly(verts, a, pOut);

		return verts;
	}

	Vector4 Camera::Project(Vector3& v) const
	{
		Vector4 result;

		float W = 1.0f / v.z;
		result.x = v.x * HCotFOV * W;
		result.y = v.y * CotFOV * W;
		result.z = (v.z - NearZ) * ProjectZ * W;
		result.w = W;

		result.x = (result.x + 1) * HalfWidth;
		result.y = (1 - result.y) * HalfHeight;
		return result;
	}

	Frustum Camera::GetViewFrustum() const
	{
		Frustum f;

		// build the planes from Projection
		f.planes[0] = Plane{ -HCotFOV,	0.0f,	1.0f };
		f.planes[1] = Plane{ HCotFOV,	0.0f,	1.0f };
		f.planes[2] = Plane{ 0.0f,    -CotFOV,	1.0f };
		f.planes[3] = Plane{ 0.0f,	    CotFOV, 1.0f };
		f.planes[4] = Plane{ 0.0f,		0.0f,	ProjectZ,	-NearZ * ProjectZ };
		f.planes[5] = Plane{ 0.0f,		0.0f,	1 - ProjectZ,	NearZ * ProjectZ };

		for (u32 i = 0; i < 6; i++)
			f.planes[i].Normalize();

		return f;
	}


	u32 Camera::ClipCode(RenderVertex& v, u32 MeshCode) const
	{
		u32 code = 0;
		if (LEFT_PLANE & MeshCode)
			code |= v.ScreenPos.x < 0 ? LEFT_PLANE : 0;
		if (RIGHT_PLANE & MeshCode)
			code |= v.ScreenPos.x > TargetWidth ? RIGHT_PLANE : 0;
		if (TOP_PLANE & MeshCode)
			code |= v.ScreenPos.y < 0 ? TOP_PLANE : 0;
		if (BOTTOM_PLANE & MeshCode)
			code |= v.ScreenPos.y > TargetHeight ? BOTTOM_PLANE : 0;
		if (NEAR_PLANE & MeshCode)
			code |= v.ViewPos.z < NearZ ? NEAR_PLANE : 0;
		if (FAR_PLANE & MeshCode)
			code |= v.ViewPos.z > FarZ ? FAR_PLANE : 0;

		return code;
	}

	u32 Inside(const RenderVertex& pos, float edge, u32 c)
	{
		switch (c)
		{
		case 0:
			return pos.ScreenPos.x >= edge;
		case 1:
			return pos.ScreenPos.x <= edge;
		case 2:
			return pos.ScreenPos.y >= edge;
		case 3:
			return pos.ScreenPos.y <= edge;
		}
		return 0;
	}

	float ClipT(const RenderVertex& p1, const RenderVertex& p2, float edge, u32 c)
	{
		switch (c)
		{
		case 0:
			return  p1.ScreenPos.x / (p1.ScreenPos.x - p2.ScreenPos.x);
		case 1:
			return (p1.ScreenPos.x - edge) / (p1.ScreenPos.x - p2.ScreenPos.x);
		case 2:
			return p1.ScreenPos.y / (p1.ScreenPos.y - p2.ScreenPos.y);
		case 3:
			return (p1.ScreenPos.y - edge) / (p1.ScreenPos.y - p2.ScreenPos.y);
		}
		return 0;
	}

	// return number of verts
	u32 Camera::ClipTriangle(const RenderVertex* pIn, RenderVertex*& pNewVerts, u16* TriIndices, u16* OutIndices, u32 TriOutCode) const
	{
		u32 FromCount = 3;
		u32 ToCount = 0;
		u16 NewIndex = (u16)(pNewVerts - pIn);
		u16 From[10] = { TriIndices[0],TriIndices[1],TriIndices[2] };
		u16 To[10];
		u16* FromBase = From;
		u16* ToBase = To;
		u16* FromIter = FromBase;
		u16* ToIter = ToBase;

		// clip against NearZ first and project
		if (TriOutCode & NEAR_PLANE)
		{
			u16 Index1 = FromIter[FromCount - 1];
			const RenderVertex* p1 = pIn + Index1;
			for (u32 i = 0; i < FromCount; i++)
			{
				if (p1->ViewPos.z >= NearZ)
				{
					*ToIter++ = Index1;
					ToCount++;
				}

				const RenderVertex* p2 = pIn + FromIter[i];

				if ((p1->OutCode & NEAR_PLANE) != (p2->OutCode & NEAR_PLANE))
				{
					float t = (p1->ViewPos.z - NearZ) / (p1->ViewPos.z - p2->ViewPos.z);
					*pNewVerts = RenderVertex::Lerp(*p1, *p2, t);
					pNewVerts->ViewPos.z = NearZ;
					pNewVerts->ScreenPos = Project(pNewVerts->ViewPos);
					pNewVerts->u = lerp(p1->u, p2->u, t);
					pNewVerts->v = lerp(p1->v, p2->v, t);
					pNewVerts->uw = pNewVerts->u * pNewVerts->ScreenPos.w;
					pNewVerts->vw = pNewVerts->v * pNewVerts->ScreenPos.w;
					pNewVerts->OutCode = ClipCode(*pNewVerts);
					pNewVerts++;
					*ToIter++ = NewIndex++;
					ToCount++;
				}

				p1 = p2;
				Index1 = FromIter[i];
			}
			swap(ToBase, FromBase);
			FromCount = ToCount;
			ToCount = 0;
		}

		u32 ClipPlanes[] =
		{
			LEFT_PLANE,
			RIGHT_PLANE,
			TOP_PLANE,
			BOTTOM_PLANE
		};

		float ClipEdges[] =
		{
			0.0f,
			(float)TargetWidth,
			0.0f,
			(float)TargetHeight
		};

		for (u32 p = 0; p < 4; p++)
		{
			// then clip against the sides of the viewport
			if (FromCount > 2 && TriOutCode & ClipPlanes[p])
			{
				FromIter = FromBase;
				ToIter = ToBase;
				u16 Index1 = FromIter[FromCount - 1];
				const RenderVertex* p1 = pIn + Index1;
				for (u32 i = 0; i < FromCount; i++)
				{
					if (Inside(*p1, ClipEdges[p], p))
					{
						*ToIter++ = Index1;
						ToCount++;
					}

					const RenderVertex* p2 = pIn + FromIter[i];

					if ((p1->OutCode & ClipPlanes[p]) != (p2->OutCode & ClipPlanes[p]))
					{
						float t = ClipT(*p1, *p2, ClipEdges[p], p);
						*pNewVerts = RenderVertex::Lerp(*p1, *p2, t);
						pNewVerts->uw = lerp(p1->uw, p2->uw, t);
						pNewVerts->vw = lerp(p1->vw, p2->vw, t);
						pNewVerts->u = pNewVerts->uw / pNewVerts->ScreenPos.w;
						pNewVerts->v = pNewVerts->vw / pNewVerts->ScreenPos.w;
						if (p < 2)
							pNewVerts->ScreenPos.x = ClipEdges[p];
						else
							pNewVerts->ScreenPos.y = ClipEdges[p];
						pNewVerts->OutCode = ClipCode(*pNewVerts);
						pNewVerts++;
						*ToIter++ = NewIndex++;
						ToCount++;
					}

					p1 = p2;
					Index1 = FromIter[i];
				}
				swap(ToBase, FromBase);
				FromCount = ToCount;
				ToCount = 0;
			}
		}
		for (u32 i = 0; i < FromCount; i++)
		{
			OutIndices[i] = FromBase[i];
		}

		return FromCount;
	}

	Mesh CreateCube()
	{
		const int NumCubeCorners = 8;
		const int NumCubeFaces = 6;
		const int NumCubeTris = 2 * NumCubeFaces;
		const int NumCubeVerts = 3 * NumCubeTris;
		static MeshVertex CubeVerts[]
		{
			{ {1.0f,	1.0f,	1.0f},	{0.0f, 0.0f, 1.0f}, 0.0f, 0.0f },
			{ {-1.0f,	1.0f,	1.0f},	{0.0f, 0.0f, 1.0f}, 0.0f, 1.0f },
			{ {1.0f,	-1.0f,	1.0f},	{0.0f, 0.0f, 1.0f}, 1.0f, 0.0f },
			{ {-1.0f,	-1.0f,	1.0f},	{0.0f, 0.0f, 1.0f}, 1.0f, 1.0f },

			{ {1.0f,	1.0f,	1.0f},	{1.0f, 0.0f, 0.0f}, 1.0f, 1.0f },
			{ {1.0f,	-1.0f,	1.0f},	{1.0f, 0.0f, 0.0f}, 0.0f, 1.0f },
			{ {1.0f,	1.0f,	-1.0f},	{1.0f, 0.0f, 0.0f}, 1.0f, 0.0f },
			{ {1.0f,	-1.0f,	-1.0f},	{1.0f, 0.0f, 0.0f}, 0.0f, 0.0f },

			{ {1.0f,	1.0f,	1.0f},	{0.0f, 1.0f, 0.0f}, 0.0f, 0.0f },
			{ {-1.0f,	1.0f,	1.0f},	{0.0f, 1.0f, 0.0f}, 0.0f, 1.0f },
			{ {1.0f,	1.0f,	-1.0f},	{0.0f, 1.0f, 0.0f}, 1.0f, 0.0f },
			{ {-1.0f,	1.0f,	-1.0f},	{0.0f, 1.0f, 0.0f}, 1.0f, 1.0f },

			{ {1.0f,	1.0f,	-1.0f},	{0.0f, 0.0f, -1.0f}, 0.0f, 0.0f },
			{ {-1.0f,	1.0f,	-1.0f},	{0.0f, 0.0f, -1.0f}, 0.0f, 1.0f },
			{ {1.0f,	-1.0f,	-1.0f},	{0.0f, 0.0f, -1.0f}, 1.0f, 0.0f },
			{ {-1.0f,	-1.0f,	-1.0f},	{0.0f, 0.0f, -1.0f}, 1.0f, 1.0f },

			{ {-1.0f,	1.0f,	1.0f},	{-1.0f, 0.0f, 0.0f}, 1.0f, 0.0f },
			{ {-1.0f,	-1.0f,	1.0f},	{-1.0f, 0.0f, 0.0f}, 0.0f, 0.0f },
			{ {-1.0f,	1.0f,	-1.0f},	{-1.0f, 0.0f, 0.0f}, 1.0f, 1.0f },
			{ {-1.0f,	-1.0f,	-1.0f},	{-1.0f, 0.0f, 0.0f}, 0.0f, 1.0f },

			{ {1.0f,	-1.0f,	1.0f},	{0.0f, -1.0f, 0.0f}, 1.0f, 0.0f },
			{ {-1.0f,	-1.0f,	1.0f},	{0.0f, -1.0f, 0.0f}, 1.0f, 1.0f },
			{ {1.0f,	-1.0f,	-1.0f},	{0.0f, -1.0f, 0.0f}, 0.0f, 0.0f },
			{ {-1.0f,	-1.0f,	-1.0f},	{0.0f, -1.0f, 0.0f}, 0.0f, 1.0f },
		};

		static u16 CubeIndices[]
		{
			0,1,2, 2,1,3,
			4,5,6, 6,5,7,
			9,8,10, 9,10,11,
			13,12,14, 13,14,15,
			16,18,17, 17,18,19,
			20,21,22, 21,23,22
		};

		Mesh m = { NumCubeVerts, CubeVerts, NumCubeTris, CubeIndices, {-1.0f,-1.0f,-1.0f}, {1.0f,1.0f,1.0f} };

		return m;
	}

	Mesh CreateTetra()
	{
		const int NumTetraCorners = 4;
		const int NumTetraTris = 4;
		const int NumTetraVerts = 3 * NumTetraTris;

		static MeshVertex TetraVerts[NumTetraVerts] = {
			{ { -1.f,-1.f,-1.f }, {}, 0.0f, 0.0f },
			{ { -1.f, 1.f, 1.f }, {}, 1.0f, 0.0f },
			{ {  1.f, 1.f,-1.f }, {}, 0.0f, 1.0f },

			{ {  1.f,-1.f, 1.f }, {}, 0.0f, 0.0f },
			{ {  1.f, 1.f,-1.f }, {}, 1.0f, 0.0f },
			{ { -1.f, 1.f, 1.f }, {}, 0.0f, 1.0f },

			{ {  1.f, 1.f,-1.f }, {}, 0.0f, 0.0f },
			{ {  1.f,-1.f, 1.f }, {}, 1.0f, 0.0f },
			{ { -1.f,-1.f,-1.f }, {}, 0.0f, 1.0f },

			{ { -1.f, 1.f, 1.f }, {}, 0.0f, 0.0f },
			{ { -1.f,-1.f,-1.f }, {}, 1.0f, 0.0f },
			{ {  1.f,-1.f, 1.f }, {}, 0.0f, 1.0f },
		};

		static u16 TetraTris[3 * NumTetraTris] = {
			0,1,2,
			3,4,5,
			6,7,8,
			9,10,11
		};

		// fill in the normals:
		for (u32 i = 0; i < NumTetraTris; i++)
		{
			Vector3 a = TetraVerts[i * 3 + 1].Pos - TetraVerts[i * 3].Pos;
			Vector3 b = TetraVerts[i * 3 + 2].Pos - TetraVerts[i * 3].Pos;
			Vector3 n = Vector3::Cross(a, b);
			n.Normalize();

			for (u32 j = 0; j < 3; j++)
			{
				TetraVerts[i * 3 + j].Normal = n;
			}
		}

		Mesh m = { NumTetraVerts, TetraVerts, NumTetraTris, TetraTris, {-1.0f,-1.0f,-1.0f}, {1.0f, 1.0f, 1.0f} };
		return m;
	}

	Mesh CreateOcta()
	{
		const int NumOctaCorners = 6;
		const int NumOctaTris = 8;
		const int NumOctaVerts = 3 * NumOctaTris;

		static MeshVertex OctaVerts[NumOctaVerts] = {
			{ { 0.f, 1.f, 0.f}, {}, 0.0f, 0.0f },
			{ {-1.f, 0.f, 0.f}, {}, 1.0f, 0.0f },
			{ { 0.f, 0.f, 1.f}, {}, 0.0f, 1.0f },
			{ { 0.f, 1.f, 0.f}, {}, 0.0f, 0.0f },
			{ { 0.f, 0.f, 1.f}, {}, 1.0f, 0.0f },
			{ { 1.f, 0.f, 0.f}, {}, 0.0f, 1.0f },
			{ { 0.f, 1.f, 0.f}, {}, 0.0f, 0.0f },
			{ { 1.f, 0.f, 0.f}, {}, 1.0f, 0.0f },
			{ { 0.f, 0.f,-1.f}, {}, 0.0f, 1.0f },
			{ { 0.f, 1.f, 0.f}, {}, 0.0f, 0.0f },
			{ { 0.f, 0.f,-1.f}, {}, 1.0f, 0.0f },
			{ {-1.f, 0.f, 0.f}, {}, 0.0f, 1.0f },

			{ { 0.f,-1.f, 0.f}, {}, 0.0f, 0.0f },
			{ { 0.f, 0.f, 1.f}, {}, 0.0f, 1.0f },
			{ {-1.f, 0.f, 0.f}, {}, 1.0f, 0.0f },
			{ { 0.f,-1.f, 0.f}, {}, 0.0f, 0.0f },
			{ { 1.f, 0.f, 0.f}, {}, 0.0f, 1.0f },
			{ { 0.f, 0.f, 1.f}, {}, 1.0f, 0.0f },
			{ { 0.f,-1.f, 0.f}, {}, 0.0f, 0.0f },
			{ { 0.f, 0.f,-1.f}, {}, 0.0f, 1.0f },
			{ { 1.f, 0.f, 0.f}, {}, 1.0f, 0.0f },
			{ { 0.f,-1.f, 0.f}, {}, 0.0f, 0.0f },
			{ {-1.f, 0.f, 0.f}, {}, 0.0f, 1.0f },
			{ { 0.f, 0.f,-1.f}, {}, 1.0f, 0.0f },
		};

		static u16 OctaTris[3 * NumOctaTris] = {
			0,1,2,		3,4,5,
			6,7,8,		9,10,11,
			12,13,14,	15,16,17,
			18,19,20,	21,22,23
		};


		// fill in the normals:
		for (u32 i = 0; i < NumOctaTris; i++)
		{
			Vector3 a = OctaVerts[i * 3 + 1].Pos - OctaVerts[i * 3].Pos;
			Vector3 b = OctaVerts[i * 3 + 2].Pos - OctaVerts[i * 3].Pos;
			Vector3 n = Vector3::Cross(a, b);
			n.Normalize();

			for (u32 j = 0; j < 3; j++)
			{
				OctaVerts[i * 3 + j].Normal = n;
			}
		}

		Mesh m = { NumOctaVerts, OctaVerts, NumOctaTris, OctaTris, {-1.0f,-1.0f,-1.0f}, {1.0f, 1.0f, 1.0f} };
		return m;
	}

	Mesh CreateIcosa()
	{
		const int NumIcosaCorners = 12;
		const int NumIcosaTris = 20;
		const int NumIcosaVerts = 3 * NumIcosaTris;
		const float G = 1.618034f;		//golden mean

		/*
		** These coordinates are at the corners of 3
		** rectangles, dimensions of 2 *golden ration long
		** and 2 wide, along the 3 main coordinate axes
		*/

		static MeshVertex IcosaVerts[NumIcosaVerts] = {
			{ {   G, 1.f, 0.f }, {}, 0.0f, 0.0f },
			{ { 0.f,   G,-1.f }, {}, 1.0f, 0.0f },
			{ { 0.f,   G, 1.f }, {}, 0.0f, 1.0f },

			{ {   G, 1.f, 0.f }, {}, 0.0f, 0.0f },
			{ { 0.f,   G, 1.f }, {}, 1.0f, 0.0f },
			{ { 1.f, 0.f,   G }, {}, 0.0f, 1.0f },

			{ {   G, 1.f, 0.f }, {}, 0.0f, 0.0f },
			{ { 1.f, 0.f,   G }, {}, 1.0f, 0.0f },
			{ {   G,-1.f, 0.f }, {}, 0.0f, 1.0f },

			{ {G, 1.f, 0.f}, {}, 0.0f, 0.0f },
			{ {G, -1.f, 0.f}, {}, 1.0f, 0.0f },
			{ {1.f, 0.f, -G}, {}, 0.0f, 1.0f },

			{ {G, 1.f, 0.f}, {}, 0.0f, 0.0f },
			{ {1.f, 0.f, -G}, {}, 0.0f, 1.0f },
			{ { 0.f,   G,-1.f }, {}, 1.0f, 0.0f },

			{ {-G, -1.f, 0.f}, {}, 0.0f, 0.0f },
			{ {0.f, -G, -1.f}, {}, 1.0f, 0.0f },
			{ {0.f, -G, 1.f}, {}, 0.0f, 1.0f },

			{ {-G, -1.f, 0.f}, {}, 0.0f, 0.0f },
			{ {0.f, -G, 1.f}, {}, 1.0f, 0.0f },
			{ {-1.f, 0.f, G}, {}, 0.0f, 1.0f },

			{ {-G, -1.f, 0.f}, {}, 0.0f, 0.0f },
			{ {-1.f, 0.f, G}, {}, 1.0f, 0.0f },
			{ {-G, 1.f, 0.f}, {}, 0.0f, 1.0f },

			{ {-G, -1.f, 0.f}, {}, 0.0f, 0.0f },
			{ {-G, 1.f, 0.f}, {}, 1.0f, 0.0f },
			{ {-1.f, 0.f, -G}, {}, 0.0f, 1.0f },

			{ {-G, -1.f, 0.f}, {}, 0.0f, 0.0f },
			{ {-1.f, 0.f, -G}, {}, 1.0f, 0.0f },
			{ {0.f, -G, -1.f}, {}, 0.0f, 1.0f },

			{ {0.f, -G, -1.f}, {}, 0.0f, 0.0f },
			{ {1.f, 0.f, -G}, {}, 1.0f, 0.0f },
			{ {G, -1.f, 0.f}, {}, 0.0f, 1.0f },

			{ {1.f, 0.f, -G}, {}, 0.0f, 0.0f },
			{ {0.f, -G, -1.f}, {}, 1.0f, 0.0f },
			{ {-1.f, 0.f, -G}, {}, 0.0f, 1.0f },

			{ {-1.f, 0.f, -G}, {}, 0.0f, 0.0f },
			{ {0.f, G, -1.f}, {}, 1.0f, 0.0f },
			{ {1.f, 0.f, -G}, {}, 0.0f, 1.0f },

			{ {0.f, G, -1.f}, {}, 0.0f, 0.0f },
			{ {-1.f, 0.f, -G}, {}, 1.0f, 0.0f },
			{ {-G, 1.f, 0.f}, {}, 0.0f, 1.0f },

			{ {-G, 1.f, 0.f}, {}, 0.0f, 0.0f },
			{ {0.f, G, 1.f}, {}, 1.0f, 0.0f },
			{ {0.f, G, -1.f}, {}, 0.0f, 1.0f },

			{ {0.f, G, 1.f}, {}, 0.0f, 0.0f },
			{ {-G, 1.f, 0.f}, {}, 1.0f, 0.0f },
			{ {-1.f, 0.f, G}, {}, 0.0f, 1.0f },

			{ {-1.f, 0.f, G}, {}, 0.0f, 0.0f },
			{ {1.f, 0.f, G}, {}, 1.0f, 0.0f },
			{ {0.f, G, 1.f}, {}, 0.0f, 1.0f },

			{ {1.f, 0.f, G}, {}, 0.0f, 0.0f },
			{ {-1.f, 0.f, G}, {}, 1.0f, 0.0f },
			{ {0.f, -G, 1.f}, {}, 0.0f, 1.0f },

			{ {G, -1.f, 0.f}, {}, 0.0f, 0.0f },
			{ {1.f, 0.f, G}, {}, 1.0f, 0.0f },
			{ {0.f, -G, 1.f}, {}, 0.0f, 1.0f },

			{ {0.f, -G, 1.f}, {}, 0.0f, 0.0f },
			{ {0.f, -G, -1.f}, {}, 1.0f, 0.0f },
			{ {G, -1.f, 0.f}, {}, 0.0f, 1.0f },
		};

		static u16 IcosaTris[3 * NumIcosaTris] = {
			0,1,2,
			3,4,5,	
			6,7,8,	
			9,10,11,	
			12,13,14,
			15,16,17,	
			18,19,20,	
			21,22,23,	
			24,25,26,
			27,28,29,
			30,31,32,
			33,34,35,
			36,37,38,
			39,40,41,
			42,43,44,
			45,46,47,
			48,49,50,
			51,52,53,
			54,55,56,
			57,58,59
		};

		// fill in the normals:
		for (u32 i = 0; i < NumIcosaTris; i++)
		{
			Vector3 a = IcosaVerts[i * 3 + 1].Pos - IcosaVerts[i * 3].Pos;
			Vector3 b = IcosaVerts[i * 3 + 2].Pos - IcosaVerts[i * 3].Pos;
			Vector3 n = Vector3::Cross(a, b);
			n.Normalize();

			for (u32 j = 0; j < 3; j++)
			{
				IcosaVerts[i * 3 + j].Normal = n;
			}
		}

		Mesh m = { NumIcosaVerts, IcosaVerts, NumIcosaTris, IcosaTris, {-G,-G,-G}, {G, G, G} };
		return m;
	}

	Mesh CreateDodeca()
	{
		const int NumDodecaFaces = 12;
		const int NumDodecaTris = 3 * NumDodecaFaces;
		const int NumDodecaVerts = 3 * NumDodecaTris;
		const float G = 1.618034f;		//golden mean
		const float GG = G - 1;
		
		/*
		**	This solid has 20 vertices and 12 pentagonal faces
		**	Use unit cube vertices +/-1, +/-1, +/-1
		**	Also use 3 orthogonal rectangular extents 
		**  2 * golden ratio long and 2 * (golden ratio -1) wide
		**  each of these rectangles extends along the main coordinate axes
		*/

		static MeshVertex DodecaVerts[NumDodecaVerts] = {
			{{   G,  GG, 0.f}},
			{{ 1.f, 1.f,-1.f}},
			{{   0,   G, -GG}},
			{{   0,   G,  GG}},
			{{ 1.f, 1.f, 1.f}},

			{{  -G,  GG, 0.f}},
			{{-1.f, 1.f, 1.f}},
			{{   0,   G,  GG}}, 
			{{   0,   G, -GG}},
			{{-1.f, 1.f,-1.f}},

			{{   0,   G,  GG}},
			{{-1.f, 1.f, 1.f}},
			{{ -GG, 0.f,   G}},
			{{  GG, 0.f,   G}},
			{{ 1.f, 1.f, 1.f}},

			{{   0,   G, -GG}},
			{{ 1.f, 1.f,-1.f}},
			{{  GG, 0.f,  -G}},
			{{ -GG, 0.f,  -G}},
			{{-1.f, 1.f,-1.f}},

			{{  GG, 0.f,  -G}},
			{{ 1.f, 1.f,-1.f}},
			{{   G,  GG, 0.f}},
			{{   G, -GG, 0.f}},
			{{ 1.f,-1.f,-1.f}},

			{{ -GG, 0.f,  -G}},
			{{-1.f,-1.f,-1.f}},
			{{  -G, -GG, 0.f}},
			{{  -G,  GG, 0.f}},
			{{-1.f, 1.f,-1.f}},

			{{  GG, 0.f,   G}},
			{{ 1.f,-1.f, 1.f}},
			{{   G, -GG, 0.f}},
			{{   G,  GG, 0.f}},
			{{ 1.f, 1.f, 1.f}},

			{{ -GG, 0.f,   G}},
			{{-1.f, 1.f, 1.f}},
			{{  -G,  GG, 0.f}},
			{{  -G, -GG, 0.f}},
			{{-1.f,-1.f, 1.f}},

			{{   G, -GG, 0.f}},
			{{ 1.f,-1.f, 1.f}},
			{{   0,  -G,  GG}},
			{{   0,  -G, -GG}},
			{{ 1.f,-1.f,-1.f}},

			{{  -G, -GG, 0.f}},
			{{-1.f,-1.f,-1.f}},
			{{   0,  -G, -GG}},
			{{   0,  -G,  GG}},
			{{-1.f,-1.f, 1.f}},

			{{   0,  -G, -GG}},
			{{-1.f,-1.f,-1.f}},
			{{ -GG, 0.f,  -G}},
			{{  GG, 0.f,  -G}},
			{{ 1.f,-1.f,-1.f}},

			{{   0,  -G,  GG}},
			{{ 1.f,-1.f, 1.f}},
			{{  GG, 0.f,   G}},
			{{ -GG, 0.f,   G}},
			{{-1.f,-1.f, 1.f}},
		};

		static u16 DodecaTris[3 * NumDodecaTris] = {
			0,1,2, 0,2,3, 0,3,4,
			5,6,7, 5,7,8, 5,8,9,
			10,11,12, 10,12,13, 10,13,14,
			15,16,17, 15,17,18, 15,18,19,
			20,21,22, 20,22,23, 20,23,24,
			25,26,27, 25,27,28, 25,28,29,
			30,31,32, 30,32,33, 30,33,34,
			35,36,37, 35,37,38, 35,38,39,
			40,41,42, 40,42,43, 40,43,44,
			45,46,47, 45,47,48, 45,48,49,
			50,51,52, 50,52,53, 50,53,54,
			55,56,57, 55,57,58, 55,58,59
		};

		f32 TextureCoordinates[] =
		{
			0.5f, 0.95f,
			1.0f, 0.59f,
			0.81f, 0.0f,
			0.19f, 0.0f,
			0.0f, 0.59f
		};
		// fill in the normals and texture coordinates
		for (u32 i = 0; i < NumDodecaFaces; i++)
		{
			Vector3 a = DodecaVerts[i * 5 + 1].Pos - DodecaVerts[i * 5].Pos;
			Vector3 b = DodecaVerts[i * 5 + 2].Pos - DodecaVerts[i * 5].Pos;
			Vector3 n = Vector3::Cross(a, b);
			n.Normalize();

			for (u32 j = 0; j < 5; j++)
			{
				DodecaVerts[i * 5 + j].Normal = n;
				DodecaVerts[i * 5 + j].u = TextureCoordinates[j * 2];
				DodecaVerts[i * 5 + j].v = TextureCoordinates[j * 2 + 1];
			}
		}

		Mesh m = { NumDodecaVerts, DodecaVerts, NumDodecaTris, DodecaTris, {-G,-G,-G}, {G, G, G} };
		return m;
	}

	Mesh CreateSphere(u32 layers, u32 slices, Arena& arena)
	{
		u32 lat = max(layers, (u32)2);
		u32 lon = max(slices, (u32)3);

		u32 NumVerts = (lat+1) * (lon+1);
		u32 NumTris = (lat - 1) * lon * 2;
		MeshVertex* SphereVerts = (MeshVertex*)arena.Allocate(NumVerts * sizeof(MeshVertex));
		u16* SphereIndices = (u16*)arena.Allocate(NumTris * 3 * sizeof(u16));
		f32 dtheta = 2 * PI / lon;
		f32 dphi = PI / lat;
		f32 theta = 0.0;
		MeshVertex* dest = SphereVerts;
		u16 Index = 0;
		u16* destIndices = SphereIndices;
		for (u32 j = 0; j <= lon; j++, theta += dtheta)
		{
			f32 s = sine(theta);
			f32 c = cosine(theta);
			f32 phi = 0.0f;
			for (u32 i = 0; i <= lat; i++, phi += dphi)
			{
				f32 cphi = cosine(phi);
				f32 sphi = sine(phi);
				dest->Pos = Vector3 { sphi * c, cphi, sphi * s };
				Vector3 norm = SphereVerts->Pos;
				norm.Normalize();
				dest->Normal = norm;
				dest->u = (f32)j / lon;
				dest->v = (f32)i / lat;
				dest++;
				if (i < lat && j < lon)
				{
					if (i > 0)
					{
						*destIndices++ = Index;
						*destIndices++ = Index + lat + 1;
						*destIndices++ = Index + lat + 2;
					}
					if (i < lat - 1)
					{
						*destIndices++ = Index;
						*destIndices++ = Index + lat + 2;
						*destIndices++ = Index + 1;
					}
				}
				Index++;
			}
		}

		Mesh m = { NumVerts, SphereVerts, NumTris, SphereIndices, {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f} };

		return m;
	}
};