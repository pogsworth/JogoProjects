#include "gfx.h"
#include "Bitmap.h"
#include "Arena.h"

namespace Jogo
{

	u32 ClipAABB(Vector3 min, Vector3 max, Matrix4& MVT, Frustum& ViewFrustum, float& MinZ, u32& OrCode)
	{
		// transform mesh AABB and abort if all out
		Vector3 origin = min * MVT;
		Vector3 axes[3] = {
			{ Vector3{ max.x, 0.0f, 0.0f } *MVT + origin},
			{ Vector3{ max.y, 0.0f, 0.0f } *MVT + origin},
			{ Vector3{ max.z, 0.0f, 0.0f } *MVT + origin}
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

	// Maybe Camera, that has VT, Frustum, Projection
	void RenderMesh(Mesh& mesh, Matrix4& ModelToWorld, Camera& camera, Bitmap& Target, Arena& arena)
	{
		// build MVT transform
		Matrix4 View = camera;
		View.Inverse();
		Matrix4 MVT = ModelToWorld * View;
		Matrix3 NormalMVT = MVT;
		NormalMVT.Normalize();

		Frustum ViewFrustum = camera.GetViewFrustum();

		float ViewMinZ;
		// early out if the mesh bbox is completely out any of the frustum planes
		if (ClipAABB(mesh.MinAABB, mesh.MaxAABB, MVT, ViewFrustum, ViewMinZ, mesh.AABBOutCode))
			return;

		RenderVertex* RenderVerts = (RenderVertex*)arena.Allocate((2 * mesh.NumVerts + 6) * sizeof(RenderVertex));
		RenderVertex* VertIter = RenderVerts;
		for (u32 i = 0; i < mesh.NumVerts; i++, VertIter++)
		{
			VertIter->ViewPos = mesh.Verts[i].Pos * MVT;
			VertIter->ViewNormal = mesh.Verts[i].Normal * NormalMVT;
			VertIter->ScreenPos = camera.Project(VertIter->ViewPos);
			VertIter->u = mesh.Verts[i].u;
			VertIter->v = mesh.Verts[i].v;
			VertIter->bIsList = false;
			VertIter->OutCode = mesh.AABBOutCode ? camera.ClipCode(*VertIter, mesh.AABBOutCode) : 0;
		}

		// loop over all triangles (indices)
		// test if 3 verts are all out same 
		// OutCode & OutCode & OutCode != 0 then skip this triangle - it is clipped
		u16* ShortIndices = mesh.SmallIndices;
		for (u32 i = 0; i < mesh.NumTris; i++)
		{
			RenderVertex p = RenderVerts[*ShortIndices++];
			RenderVertex q = RenderVerts[*ShortIndices++];
			RenderVertex r = RenderVerts[*ShortIndices++];

			if (p.OutCode & q.OutCode & r.OutCode)
				continue;

			// can we use screen verts for backface?
			if (ViewMinZ > 0)
			{
				if ((r.ScreenPos.x - p.ScreenPos.x) * (q.ScreenPos.y - p.ScreenPos.y) >=
					(r.ScreenPos.y - p.ScreenPos.y) * (q.ScreenPos.x - p.ScreenPos.x))
					continue;
			}

			u32 OrCode = p.OutCode | q.OutCode | r.OutCode;
			if (!OrCode)
			{
				Target.DrawLine((u32)p.ScreenPos.x, (u32)p.ScreenPos.y, (u32)q.ScreenPos.x, (u32)q.ScreenPos.y, 0);
				Target.DrawLine((u32)q.ScreenPos.x, (u32)q.ScreenPos.y, (u32)r.ScreenPos.x, (u32)r.ScreenPos.y, 0);
				Target.DrawLine((u32)r.ScreenPos.x, (u32)r.ScreenPos.y, (u32)p.ScreenPos.x, (u32)p.ScreenPos.y, 0);
			}
			else
			{
				// need to clip this triangle
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

	Vector4 Camera::Project(Vector3& v)
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

	Frustum Camera::GetViewFrustum()
	{
		Frustum f;

		// build the planes from Projection
		f.planes[0] = Plane{  HCotFOV,	0.0f,	1.0f };
		f.planes[1] = Plane{ -HCotFOV,	0.0f,	1.0f };
		f.planes[2] = Plane{  0.0f,    -CotFOV,	1.0f };
		f.planes[3] = Plane{  0.0f,		CotFOV, 1.0f };
		f.planes[4] = Plane{  0.0f,		0.0f,	ProjectZ,	-NearZ * ProjectZ };
		f.planes[5] = Plane{  0.0f,		0.0f,	1-ProjectZ,	NearZ * ProjectZ};

		for (u32 i = 0; i < 6; i++)
			f.planes[i].Normalize();
		
		return f;
	}


	u32 Camera::ClipCode(RenderVertex& v, u32 MeshCode)
	{
		u32 code = 0;
		if (1 & MeshCode)
			code |= v.ScreenPos.x < 0;
		code <<= 1;
		if (2 & MeshCode)
			code |= v.ScreenPos.x > TargetWidth;
		code <<= 1;
		if (4 & MeshCode)
			code |= v.ScreenPos.y < 0;
		code <<= 1;
		if (8 & MeshCode)
			code |= v.ScreenPos.y > TargetHeight;
		code <<= 1;
		if (16 & MeshCode)
			code |= v.ViewPos.z < NearZ;
		code <<= 1;
		if (32 & MeshCode)
			code |= v.ViewPos.z > FarZ;

		return code;
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