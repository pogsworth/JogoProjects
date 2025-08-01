
#include "Mesh.h"
#include "Bitmap.h"


namespace Jogo
{
	u32 ClipAABB(Vector3 min, Vector3 max, Matrix4& MVT, Frustum& ViewFrustum, u32& OrCode)
	{
		// transform mesh AABB and abort if all out
		Vector3 origin = min * MVT;
		Vector3 xaxis = Vector3{ max.x,0.0f, 0.0f } *MVT + origin;
		Vector3 yaxis = Vector3{ max.y,0.0f, 0.0f } *MVT + origin;
		Vector3 zaxis = Vector3{ max.z,0.0f, 0.0f } *MVT + origin;

		// 8 corners of the transformed AABB are:
		// origin
		// origin + xaxis
		// origin + yaxis
		// origin + zaxis
		// origin + xaxis + yaxis
		// origin + xaxis + zaxis
		// origin + yaxis + zaxis
		// origin + xaxis + yaxis + zaxis

		OrCode = 0;
		u32 AndCode = 0x3f;
		//test each of these corners against view frustum and save code in mesh.AABBClipCode
		// AndCode &= code;
		// clipcode |= code

		return AndCode;
	}

	//							Maybe Camera, that has VT, Frustum, Projection
	void RenderMesh(Mesh& mesh, Matrix4& MVT, Bitmap& Texture)
	{
//		if (ClipAABB(mesh.MinAABB, mesh.MaxAABB, Frustum& ViewFrustum, mesh.AABBClipCode))
//			return;

		// loop over all mesh.Verts
		// transform and project them
		// if AABBClipCode is nonzero, test vertex against all clipping planes and store OutCode
		// set bIsLit = false

		// loop over all triangles (indices)
		// test if 3 verts are all out same 
		// OutCode & OutCode & OutCode != 0 then skip this triangle - it is clipped

		// test for backface, or area too small

	}
}