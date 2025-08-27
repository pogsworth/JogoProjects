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
	void RenderMesh(Mesh& mesh, Matrix4& ModelToWorld, Camera& camera, Bitmap& Target, Arena& arena)
	{
		// build MVT transform
		Matrix4 View = camera.GetInverse();;
		Matrix4 MVT = ModelToWorld * View;

		Frustum ViewFrustum = camera.GetViewFrustum();

		float ViewMinZ;
		// early out if the mesh bbox is completely out any of the frustum planes
		if (ClipAABB(mesh.MinAABB, mesh.MaxAABB, MVT, ViewFrustum, ViewMinZ, mesh.AABBOutCode))
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
			VertIter->bIsLit = false;
			VertIter->OutCode = mesh.AABBOutCode ? camera.ClipCode(*VertIter, mesh.AABBOutCode) : 0;
		}

		// loop over all triangles (indices)
		// test if 3 verts are all out same 
		// OutCode & OutCode & OutCode != 0 then skip this triangle - it is clipped
		u16* ShortIndices = mesh.SmallIndices;
		u16* VisibleTriIter = VisibleTris;

		for (u32 i = 0; i < mesh.NumTris; i++, ShortIndices+=3)
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
				// do backface checkin screen space
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
				u32 NumVerts = camera.ClipTriangle(camera, RenderVerts, VertIter, ShortIndices, ClippedIndices, OrCode);
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

		for (u16* TriIter = VisibleTris; TriIter < VisibleTriIter; TriIter +=3)
		{
			RenderVertex& p = RenderVerts[TriIter[0]];
			RenderVertex& q = RenderVerts[TriIter[1]];
			RenderVertex& r = RenderVerts[TriIter[2]];

			Bitmap::Vertex tri[3] = {
				{p.ScreenPos.x, p.ScreenPos.y, p.color},
				{q.ScreenPos.x, q.ScreenPos.y, q.color},
				{r.ScreenPos.x, r.ScreenPos.y, r.color},
			};
			Target.FillTriangle(tri);	// tri, tri + 1, tri + 2);
			//Target.DrawLine(p.ScreenPos.x, p.ScreenPos.y, q.ScreenPos.x, q.ScreenPos.y, 0);
			//Target.DrawLine(q.ScreenPos.x, q.ScreenPos.y, r.ScreenPos.x, r.ScreenPos.y, 0);
			//Target.DrawLine(r.ScreenPos.x, r.ScreenPos.y, p.ScreenPos.x, p.ScreenPos.y, 0);
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
		f.planes[1] = Plane{  HCotFOV,	0.0f,	1.0f };
		f.planes[2] = Plane{  0.0f,    -CotFOV,	1.0f };
		f.planes[3] = Plane{  0.0f,	    CotFOV, 1.0f };
		f.planes[4] = Plane{  0.0f,		0.0f,	ProjectZ,	-NearZ * ProjectZ };
		f.planes[5] = Plane{  0.0f,		0.0f,	1-ProjectZ,	NearZ * ProjectZ};

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
			code |= v.ScreenPos.x > TargetWidth ? RIGHT_PLANE: 0;
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

	// return number of verts
	u32 Camera::ClipTriangle(Camera& camera, RenderVertex* pIn, RenderVertex*& pNewVerts, u16* TriIndices, u16* OutIndices, u32 TriOutCode)
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
		// clip against NearZ first
		// and project and backface check
		if (TriOutCode & NEAR_PLANE)
		{
			u16 Index1 = FromIter[FromCount-1];
			RenderVertex* p1 = pIn + Index1;
			for (u32 i = 0; i < FromCount; i++)
			{
				if (p1->ViewPos.z >= NearZ)
				{
					*ToIter++ = Index1;
					ToCount++;
				}

				RenderVertex* p2 = pIn + FromIter[i];

				if ((p1->OutCode & NEAR_PLANE) != (p2->OutCode & NEAR_PLANE))
				{
					float t = (p1->ViewPos.z - NearZ) / (p1->ViewPos.z - p2->ViewPos.z);
					*pNewVerts = RenderVertex::Lerp(*p1, *p2, t);
					pNewVerts->ViewPos.z = NearZ;
					pNewVerts->ScreenPos = camera.Project(pNewVerts->ViewPos);
					pNewVerts->OutCode = camera.ClipCode(*pNewVerts);
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

		// then clip against the sides of the viewport
		if (FromCount > 2 && TriOutCode & LEFT_PLANE)
		{
			FromIter = FromBase;
			ToIter = ToBase;
			u16 Index1 = FromIter[FromCount - 1];
			RenderVertex* p1 = pIn + Index1;
			for (u32 i = 0; i < FromCount; i++)
			{
				if (p1->ScreenPos.x >= 0.0f)
				{
					*ToIter++ = Index1;
					ToCount++;
				}

				RenderVertex* p2 = pIn + FromIter[i];

				if ((p1->OutCode & LEFT_PLANE) != (p2->OutCode & LEFT_PLANE))
				{
					float t = p1->ScreenPos.x / (p1->ScreenPos.x - p2->ScreenPos.x);
					*pNewVerts = RenderVertex::Lerp(*p1, *p2, t);
					pNewVerts->ScreenPos.x = 0.0f;
					pNewVerts->OutCode = camera.ClipCode(*pNewVerts);
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

		if (FromCount > 2 && TriOutCode & RIGHT_PLANE)
		{
			FromIter = FromBase;
			ToIter = ToBase;
			u16 Index1 = FromIter[FromCount - 1];
			RenderVertex* p1 = pIn + Index1;
			for (u32 i = 0; i < FromCount; i++)
			{
				if (p1->ScreenPos.x <= camera.TargetWidth)
				{
					*ToIter++ = Index1;
					ToCount++;
				}

				RenderVertex* p2 = pIn + FromIter[i];

				if ((p1->OutCode & RIGHT_PLANE) != (p2->OutCode & RIGHT_PLANE))
				{
					float t = (p1->ScreenPos.x - camera.TargetWidth) / (p1->ScreenPos.x - p2->ScreenPos.x);
					*pNewVerts = RenderVertex::Lerp(*p1, *p2, t);
					pNewVerts->ScreenPos.x = (float)camera.TargetWidth;
					pNewVerts->OutCode = camera.ClipCode(*pNewVerts);
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

		if (FromCount > 2 && TriOutCode & TOP_PLANE)
		{
			FromIter = FromBase;
			ToIter = ToBase;
			u16 Index1 = FromIter[FromCount - 1];
			RenderVertex* p1 = pIn + Index1;
			for (u32 i = 0; i < FromCount; i++)
			{
				if (p1->ScreenPos.y >= 0.0f)
				{
					*ToIter++ = Index1;
					ToCount++;
				}

				RenderVertex* p2 = pIn + FromIter[i];

				if ((p1->OutCode & TOP_PLANE) != (p2->OutCode & TOP_PLANE))
				{
					float t = p1->ScreenPos.y / (p1->ScreenPos.y - p2->ScreenPos.y);
					*pNewVerts = RenderVertex::Lerp(*p1, *p2, t);
					pNewVerts->ScreenPos.y = 0.0f;
					pNewVerts->OutCode = camera.ClipCode(*pNewVerts);
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

		if (FromCount > 2 && TriOutCode & BOTTOM_PLANE)
		{
			FromIter = FromBase;
			ToIter = ToBase;
			u16 Index1 = FromIter[FromCount - 1];
			RenderVertex* p1 = pIn + Index1;
			for (u32 i = 0; i < FromCount; i++)
			{
				if (p1->ScreenPos.y <= camera.TargetHeight)
				{
					*ToIter++ = Index1;
					ToCount++;
				}

				RenderVertex* p2 = pIn + FromIter[i];

				if ((p1->OutCode & BOTTOM_PLANE) != (p2->OutCode & BOTTOM_PLANE))
				{
					float t = (p1->ScreenPos.y - camera.TargetHeight) / (p1->ScreenPos.y - p2->ScreenPos.y);
					*pNewVerts = RenderVertex::Lerp(*p1, *p2, t);
					pNewVerts->ScreenPos.y = (float)camera.TargetHeight;
					pNewVerts->OutCode = camera.ClipCode(*pNewVerts);
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

		for (u32 i = 0; i < FromCount; i++)
		{
			OutIndices[i] = FromBase[i];
		}

		return FromCount;
	}

	MeshVertex CubeVerts[]
	{
		{ {1.0f,	1.0f,	1.0f},	{0.0f, 0.0f, 1.0f}, 0.0f, 0.0f },
		{ {-1.0f,	1.0f,	1.0f},	{0.0f, 0.0f, 1.0f}, 0.0f, 0.0f },
		{ {1.0f,	-1.0f,	1.0f},	{0.0f, 0.0f, 1.0f}, 0.0f, 0.0f },
		{ {-1.0f,	-1.0f,	1.0f},	{0.0f, 0.0f, 1.0f}, 0.0f, 0.0f },

		{ {1.0f,	1.0f,	1.0f},	{1.0f, 0.0f, 0.0f}, 0.0f, 0.0f },
		{ {1.0f,	-1.0f,	1.0f},	{1.0f, 0.0f, 0.0f}, 0.0f, 0.0f },
		{ {1.0f,	1.0f,	-1.0f},	{1.0f, 0.0f, 0.0f}, 0.0f, 0.0f },
		{ {1.0f,	-1.0f,	-1.0f},	{1.0f, 0.0f, 0.0f}, 0.0f, 0.0f },

		{ {1.0f,	1.0f,	1.0f},	{0.0f, 1.0f, 0.0f}, 0.0f, 0.0f },
		{ {-1.0f,	1.0f,	1.0f},	{0.0f, 1.0f, 0.0f}, 0.0f, 0.0f },
		{ {1.0f,	1.0f,	-1.0f},	{0.0f, 1.0f, 0.0f}, 0.0f, 0.0f },
		{ {-1.0f,	1.0f,	-1.0f},	{0.0f, 1.0f, 0.0f}, 0.0f, 0.0f },

		{ {1.0f,	1.0f,	-1.0f},	{0.0f, 0.0f, -1.0f}, 0.0f, 0.0f },
		{ {-1.0f,	1.0f,	-1.0f},	{0.0f, 0.0f, -1.0f}, 0.0f, 0.0f },
		{ {1.0f,	-1.0f,	-1.0f},	{0.0f, 0.0f, -1.0f}, 0.0f, 0.0f },
		{ {-1.0f,	-1.0f,	-1.0f},	{0.0f, 0.0f, -1.0f}, 0.0f, 0.0f },

		{ {-1.0f,	1.0f,	1.0f},	{-1.0f, 0.0f, 0.0f}, 0.0f, 0.0f },
		{ {-1.0f,	-1.0f,	1.0f},	{-1.0f, 0.0f, 0.0f}, 0.0f, 0.0f },
		{ {-1.0f,	1.0f,	-1.0f},	{-1.0f, 0.0f, 0.0f}, 0.0f, 0.0f },
		{ {-1.0f,	-1.0f,	-1.0f},	{-1.0f, 0.0f, 0.0f}, 0.0f, 0.0f },

		{ {1.0f,	-1.0f,	1.0f},	{0.0f, -1.0f, 0.0f}, 0.0f, 0.0f },
		{ {-1.0f,	-1.0f,	1.0f},	{0.0f, -1.0f, 0.0f}, 0.0f, 0.0f },
		{ {1.0f,	-1.0f,	-1.0f},	{0.0f, -1.0f, 0.0f}, 0.0f, 0.0f },
		{ {-1.0f,	-1.0f,	-1.0f},	{0.0f, -1.0f, 0.0f}, 0.0f, 0.0f },
	};

	u16 CubeIndices[]
	{
		0,1,2, 2,1,3,
		4,5,6, 6,5,7,
		9,8,10, 9,10,11,
		13,12,14, 13,14,15,
		16,18,17, 17,18,19,
		20,21,22, 21,23,22
	};

	Mesh CreateCube(float size)
	{
		Mesh m = { 24, CubeVerts, 12, CubeIndices, {-1.0f,-1.0f,-1.0f}, {1.0f,1.0f,1.0f} };

		return m;
	}
};