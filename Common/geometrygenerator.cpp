#include"geometrygenerator.h"
using namespace DirectX;

void GeometryGenerator::CreateGrid(float length_x, float length_z, UINT num_x, UINT num_z, MeshData &mesh_data) {
	if (num_x == 0 || num_z == 0) return;

	mesh_data.clear();

	UINT vertex_count = num_x*num_z;
	mesh_data.vertices.reserve(vertex_count);
	UINT face_count = (num_x - 1)*(num_z - 1)*2;
	mesh_data.indices.reserve(face_count*6); // Triangles X2, vertices X3

	float x_by2 = length_x / 2.0f, z_by2 = length_z / 2.0f;
	float dx = length_x / (num_x - 1), dz = length_z / (num_z - 1);
	float du = 1.0f / (num_x - 1), dv = 1.0f / (num_z - 1);

	float x, z, u, v;
	
	// Add vertices, from top-left (with maximun z and minimum x) to bottom-right
	for (UINT iz = 0; iz < num_z; ++iz) {
		z = z_by2 - iz*dz;
		v = iz*dv;
		for (UINT ix = 0; ix < num_x; ++ix) {
			x = -x_by2 + ix*dx;
			u = ix*du;

			Vertex v(XMFLOAT3(x, 0.0f, z), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(u, v));
			mesh_data.vertices.push_back(v);
		}
	}


	// Add indices
	
	for (UINT iz = 0; iz < num_z - 1; ++iz) {
		
		for (UINT ix = 0; ix < num_x - 1; ++ix) {
			mesh_data.indices.push_back(iz*num_x+ ix);
			mesh_data.indices.push_back(iz*num_x + ix + 1);
			mesh_data.indices.push_back((iz + 1)*num_x + ix);

			mesh_data.indices.push_back((iz + 1)*num_x + ix);
			mesh_data.indices.push_back(iz*num_x + ix + 1);
			mesh_data.indices.push_back((iz + 1)*num_x + ix + 1);
		}
	}

}
void GeometryGenerator::CreateBox(float length_x, float length_y, float length_z, MeshData &mesh_data) {
	Vertex v[24];

	// Coordinate origin at the geometry center

	// Half width, height, and depth 
	float x_by2 = length_x / 2.0f, y_by2 = length_y / 2.0f, z_by2 = length_z / 2.0f;

	// Six faces. Clockwise winding order when looking down the normal for each face.

	// Fill in the front face vertex data.
	v[0] = Vertex(-x_by2, -y_by2, -z_by2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = Vertex(-x_by2, +y_by2, -z_by2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex(+x_by2, +y_by2, -z_by2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = Vertex(+x_by2, -y_by2, -z_by2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = Vertex(-x_by2, -y_by2, +z_by2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = Vertex(+x_by2, -y_by2, +z_by2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = Vertex(+x_by2, +y_by2, +z_by2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = Vertex(-x_by2, +y_by2, +z_by2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8] = Vertex(-x_by2, +y_by2, -z_by2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9] = Vertex(-x_by2, +y_by2, +z_by2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = Vertex(+x_by2, +y_by2, +z_by2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = Vertex(+x_by2, +y_by2, -z_by2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = Vertex(-x_by2, -y_by2, -z_by2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = Vertex(+x_by2, -y_by2, -z_by2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = Vertex(+x_by2, -y_by2, +z_by2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = Vertex(-x_by2, -y_by2, +z_by2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = Vertex(-x_by2, -y_by2, +z_by2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = Vertex(-x_by2, +y_by2, +z_by2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = Vertex(-x_by2, +y_by2, -z_by2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = Vertex(-x_by2, -y_by2, -z_by2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = Vertex(+x_by2, -y_by2, -z_by2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = Vertex(+x_by2, +y_by2, -z_by2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = Vertex(+x_by2, +y_by2, +z_by2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = Vertex(+x_by2, -y_by2, +z_by2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	mesh_data.vertices.assign(v, v + 24);

	// Fill the indices
	UINT i[36];


	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	mesh_data.indices.assign(i, i + 36);

}


void GeometryGenerator::CreateCylinder(float radius_top, float radius_bottom, float height, UINT stack_cnt, UINT slice_cnt, MeshData &mesh_data) {
	mesh_data.clear();

	float dr = (radius_top - radius_bottom) / stack_cnt;
	float dy = height / stack_cnt;
	float dtheta = 2.0f*XM_PI / slice_cnt;
	
	UINT ring_cnt = stack_cnt + 1;
	UINT vertices_per_ring = slice_cnt + 1;

	// Generate vertex data
	for (UINT i = 0; i < ring_cnt; ++i) {
		float y = -0.5f*height + dy*i;
		float r = radius_bottom + dr*i;
		for (UINT j = 0; j < vertices_per_ring; ++j) {
			// Note that the first and last vertices share common position but different UV
			float cos = cosf(j*dtheta), sin = sinf(j*dtheta);
			Vertex v;
			v.position = XMFLOAT3(r*cos, y, r*sin);
			v.texcoord = XMFLOAT2(j *1.0f / slice_cnt, 1.0f - i * 1.0f / stack_cnt);
			// Cylinder can be parameterized as follows, where we introduce v
			// parameter that goes in the same direction as the v tex-coord
			// so that the bitangent goes in the same direction as the v tex-coord.
			//   Let r0 be the bottom radius and let r1 be the top radius.
			//   y(v) = h - hv for v in [0,1].
			//   r(v) = r1 + (r0-r1)v
			//
			//   x(u, v) = r(v)*cos(2*pi*u)
			//   y(u, v) = h - hv
			//   z(u, v) = r(v)*sin(2*pi*u)
			// 
			//  dx/du = -2*pi*r(v)*sin(2*pi*u)
			//  dy/du = 0
			//  dz/du = +2*pi*r(v)*cos(2*pi*u)
			//
			//  dx/dv = (r0-r1)*cos(2*pi*u)
			//  dy/dv = -h
			//  dz/dv = (r0-r1)*sin(2*pi*u)

			float r_diff = radius_bottom - radius_top;
			// dp/du as tangent, length scaled
			v.tangent = XMFLOAT3(-r*sin, 0.0f, r*cos);
			// dp/dv as bitangent
			XMFLOAT3 bitangent(r_diff*cos, -height, r_diff*sin);

			XMVECTOR T = XMLoadFloat3(&v.tangent);
			XMVECTOR B = XMLoadFloat3(&bitangent);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));

			XMStoreFloat3(&v.normal, N);

			mesh_data.vertices.push_back(v);
		}
	}

	// Generate index data
	for (UINT i = 0; i < stack_cnt; ++i) {
		for (UINT j = 0; j < slice_cnt; ++j) {
			mesh_data.indices.push_back(i*vertices_per_ring + j);
			mesh_data.indices.push_back((i + 1)*vertices_per_ring + j);
			mesh_data.indices.push_back((i + 1)*vertices_per_ring + j + 1);
			mesh_data.indices.push_back(i*vertices_per_ring + j);
			mesh_data.indices.push_back((i + 1)*vertices_per_ring + j + 1);
			mesh_data.indices.push_back(i*vertices_per_ring + j + 1);
		}
	}

	// Construct top cap

	// Duplicate the cap ring vertices
	UINT base_index = mesh_data.vertices.size();
	float y = 0.5f*height;
	for (UINT j = 0; j < vertices_per_ring; ++j) {
		float cos = cosf(j*dtheta), sin = sinf(j*dtheta);
		float x = radius_top*cos, z = radius_top*sin;
		float u = x / height + 0.5f, v = y / height + 0.5f; // ????
		mesh_data.vertices.push_back(Vertex(
			XMFLOAT3(x, y, z), 
			XMFLOAT3(0.0f, 1.0f, 0.0f), 
			XMFLOAT3(1.0f, 0.0f, 0.0f), 
			XMFLOAT2(u, v)));
	}
	// Top center vertex
	mesh_data.vertices.push_back(Vertex(
		XMFLOAT3(0.0f, y, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT2(0.5f, 0.5f)));

	UINT center_index = mesh_data.vertices.size() - 1;
	for (UINT j = 0; j < slice_cnt; ++j) {
		mesh_data.indices.push_back(center_index);
		mesh_data.indices.push_back(base_index + j + 1);
		mesh_data.indices.push_back(base_index + j);
	}


	// Construct bottom cap

	base_index = mesh_data.vertices.size();
	y = -0.5f*height;
	for (UINT j = 0; j < vertices_per_ring; ++j) {
		float cos = cosf(j*dtheta), sin = sinf(j*dtheta);
		float x = radius_bottom*cos, z = radius_bottom*sin;
		float u = x / height + 0.5f, v = y / height + 0.5f; // ????
		mesh_data.vertices.push_back(Vertex(
			XMFLOAT3(x, y, z),
			XMFLOAT3(0.0f, -1.0f, 0.0f),
			XMFLOAT3(1.0f, 0.0f, 0.0f),
			XMFLOAT2(u, v)));
	}
	// Bottom center vertex
	mesh_data.vertices.push_back(Vertex(
		XMFLOAT3(0.0f, y, 0.0f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT2(0.5f, 0.5f)));

	center_index = mesh_data.vertices.size() - 1;
	for (UINT j = 0; j < slice_cnt; ++j) {
		mesh_data.indices.push_back(center_index);
		mesh_data.indices.push_back(base_index + j); // Note the difference
		mesh_data.indices.push_back(base_index + j + 1);
	}

}

void GeometryGenerator::CreateSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData)
{
	meshData.vertices.clear();
	meshData.indices.clear();

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	meshData.vertices.push_back(topVertex);

	float phiStep = XM_PI / stackCount;
	float thetaStep = 2.0f*XM_PI / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (UINT i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i*phiStep;

		// vertices of ring.
		for (UINT j = 0; j <= sliceCount; ++j)
		{
			float theta = j*thetaStep;
			
			Vertex v;

			// spherical to cartesian
			v.position.x = radius*sinf(phi)*cosf(theta);
			v.position.y = radius*cosf(phi);
			v.position.z = radius*sinf(phi)*sinf(theta);

			// Partial derivative of P with respect to theta
			v.tangent.x = -radius*sinf(phi)*sinf(theta);
			v.tangent.y = 0.0f;
			v.tangent.z = +radius*sinf(phi)*cosf(theta);

			XMVECTOR T = XMLoadFloat3(&v.tangent);
			XMStoreFloat3(&v.tangent, XMVector3Normalize(T));

			XMVECTOR p = XMLoadFloat3(&v.position);
			XMStoreFloat3(&v.normal, XMVector3Normalize(p));

			v.texcoord.x = theta / XM_2PI;
			v.texcoord.y = phi / XM_PI;

			meshData.vertices.push_back(v);
		}
	}

	meshData.vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (UINT i = 1; i <= sliceCount; ++i)
	{
		meshData.indices.push_back(0);
		meshData.indices.push_back(i + 1);
		meshData.indices.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	UINT baseIndex = 1;
	UINT ringVertexCount = sliceCount + 1;
	for (UINT i = 0; i < stackCount - 2; ++i)
	{
		for (UINT j = 0; j < sliceCount; ++j)
		{
			meshData.indices.push_back(baseIndex + i*ringVertexCount + j);
			meshData.indices.push_back(baseIndex + i*ringVertexCount + j + 1);
			meshData.indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);

			meshData.indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);
			meshData.indices.push_back(baseIndex + i*ringVertexCount + j + 1);
			meshData.indices.push_back(baseIndex + (i + 1)*ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	UINT southPoleIndex = (UINT)meshData.vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.indices.push_back(southPoleIndex);
		meshData.indices.push_back(baseIndex + i);
		meshData.indices.push_back(baseIndex + i + 1);
	}
}



void GeometryGenerator::CreateGeosphere(float radius, UINT num_subdiv, MeshData &mesh_data) {
	mesh_data.clear();

	const float x = 0.525731f;
	const float z = 0.850651f;

	XMFLOAT3 pos[12] = 
	{
		XMFLOAT3(-x, 0.0f, z),  XMFLOAT3(x, 0.0f, z),  
		XMFLOAT3(-x, 0.0f, -z), XMFLOAT3(x, 0.0f, -z),    
		XMFLOAT3(0.0f, z, x),   XMFLOAT3(0.0f, z, -x), 
		XMFLOAT3(0.0f, -z, x),  XMFLOAT3(0.0f, -z, -x),    
		XMFLOAT3(z, x, 0.0f),   XMFLOAT3(-z, x, 0.0f), 
		XMFLOAT3(z, -x, 0.0f),  XMFLOAT3(-z, -x, 0.0f)
	};

	DWORD idx[60] = 
	{
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,    
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,    
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0, 
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7 
	};

	mesh_data.vertices.resize(12);
	mesh_data.indices.resize(60);

	for (UINT i = 0; i < 12; ++i)
		mesh_data.vertices[i].position = pos[i];

	for (UINT i = 0; i < 60; ++i)
		mesh_data.indices[i] = idx[i];

	for (UINT i = 0; i < num_subdiv; ++i)
		Subdivide(mesh_data);

	for (UINT i = 0; i < mesh_data.vertices.size(); ++i) {
		XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&mesh_data.vertices[i].position));
		XMStoreFloat3(&mesh_data.vertices[i].normal, n);
		// Project subdivided vertices to the sphere
		XMStoreFloat3(&mesh_data.vertices[i].position, n*radius);

		float phi = acosf(mesh_data.vertices[i].position.y / radius);
		float theta = MathHelper::AngleFromXY(
			mesh_data.vertices[i].position.x, 
			mesh_data.vertices[i].position.z);

		// Use theta and phi to parameterize sphere
		mesh_data.vertices[i].texcoord = XMFLOAT2(theta / (2.0f*XM_PI), phi / XM_PI);
		// Tangent is parallel to dp/du
		mesh_data.vertices[i].tangent = XMFLOAT3(-sinf(theta), 0.0f, cosf(theta));
	}

}

void GeometryGenerator::Subdivide(MeshData& mesh_data)
{
	// Save a copy of the input geometry.
	MeshData input_copy = mesh_data;

	mesh_data.clear();
	mesh_data.vertices.reserve(input_copy.vertices.size() * 2);
	mesh_data.indices.reserve(input_copy.indices.size() * 4);
	//       1
	//       v1
	//       *
	//      / \
	//  3  /   \ 4
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2    v2
	// 0     5     2
	UINT tris_cnt = input_copy.indices.size() / 3;
	for (UINT i = 0; i < tris_cnt; ++i)
	{
		Vertex v0 = input_copy.vertices[input_copy.indices[i * 3 + 0]];
		Vertex v1 = input_copy.vertices[input_copy.indices[i * 3 + 1]];
		Vertex v2 = input_copy.vertices[input_copy.indices[i * 3 + 2]];

		//
		// Generate the midpoints.
		//

		Vertex m0, m1, m2;

		// For subdivision, we just care about the position component.  We derive the other
		// vertex components in CreateGeosphere.

		m0.position = XMFLOAT3(
			0.5f*(v0.position.x + v1.position.x),
			0.5f*(v0.position.y + v1.position.y),
			0.5f*(v0.position.z + v1.position.z));

		m1.position = XMFLOAT3(
			0.5f*(v1.position.x + v2.position.x),
			0.5f*(v1.position.y + v2.position.y),
			0.5f*(v1.position.z + v2.position.z));

		m2.position = XMFLOAT3(
			0.5f*(v0.position.x + v2.position.x),
			0.5f*(v0.position.y + v2.position.y),
			0.5f*(v0.position.z + v2.position.z));

		//
		// Add new geometry.
		//

		mesh_data.vertices.push_back(v0); // 0
		mesh_data.vertices.push_back(v1); // 1
		mesh_data.vertices.push_back(v2); // 2
		mesh_data.vertices.push_back(m0); // 3
		mesh_data.vertices.push_back(m1); // 4
		mesh_data.vertices.push_back(m2); // 5

		mesh_data.indices.push_back(i * 6 + 0);
		mesh_data.indices.push_back(i * 6 + 3);
		mesh_data.indices.push_back(i * 6 + 5);

		mesh_data.indices.push_back(i * 6 + 3);
		mesh_data.indices.push_back(i * 6 + 4);
		mesh_data.indices.push_back(i * 6 + 5);

		mesh_data.indices.push_back(i * 6 + 5);
		mesh_data.indices.push_back(i * 6 + 4);
		mesh_data.indices.push_back(i * 6 + 2);

		mesh_data.indices.push_back(i * 6 + 3);
		mesh_data.indices.push_back(i * 6 + 1);
		mesh_data.indices.push_back(i * 6 + 4);
	}
}

void GeometryGenerator::CreateFullscreenQuad(MeshData& meshData)
{
	meshData.vertices.resize(4);
	meshData.indices.resize(6);

	// Position coordinates specified in NDC space.
	meshData.vertices[0] = Vertex(
		-1.0f, -1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	meshData.vertices[1] = Vertex(
		-1.0f, +1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	meshData.vertices[2] = Vertex(
		+1.0f, +1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	meshData.vertices[3] = Vertex(
		+1.0f, -1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	meshData.indices[0] = 0;
	meshData.indices[1] = 1;
	meshData.indices[2] = 2;

	meshData.indices[3] = 0;
	meshData.indices[4] = 2;
	meshData.indices[5] = 3;
}
