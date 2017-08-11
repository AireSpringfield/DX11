#ifndef GEOMETRYGENERATOR_H
#define GEOMETRYGENERATOR_H

#include"d3dutility.h"
#include<vector>

class GeometryGenerator {
public:
	struct Vertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT2 texcoord;

		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			position(px, py, pz), normal(nx, ny, nz),
			tangent(tx, ty, tz), texcoord(u, v) {}

		Vertex(
			const DirectX::XMFLOAT3 &position,
			const DirectX::XMFLOAT3 &normal,
			const DirectX::XMFLOAT3 &tangent,
			const DirectX::XMFLOAT2 &texcoord) :
			position(position), normal(normal),
			tangent(tangent), texcoord(texcoord) {}

		Vertex() {}
	};

	struct MeshData {
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
		void clear() {
			vertices.clear();
			indices.clear();
		}
	};

	void CreateGrid(float length_x, float length_z, UINT num_x, UINT num_z, MeshData &meshData);

	void CreateBox(float length_x, float length_y, float length_z, MeshData &mesh_data);

	void CreateGeosphere(float radius, UINT num_subdiv, MeshData &mesh_data);

	void CreateSphere(float radius, UINT slice_count, UINT stack_count, MeshData &mesh_data);

	void CreateCylinder(float radius_top, float radius_bottom, float height, UINT stack_cnt, UINT slice_cnt, MeshData &mesh_data);

	void Subdivide(MeshData &mesh_data);


	};




#endif 