#pragma once
#define _USE_MATH_DEFINES
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include "GridFiller.h"

// Mesh type
typedef OpenMesh::TriMesh_ArrayKernelT<> Mesh;

// Macros for extracting buffers from OpenMesh
#define VERTEX_DATA(mesh) (float*) &(mesh.point(*mesh.vertices_begin()))
#define NORMAL_DATA(mesh) (float*) &(mesh.normal(*mesh.vertices_begin()))
#define TEXTURE_DATA(mesh) (float*) &(mesh.texcoord2D(*mesh.vertices_begin()))


struct mesh_data {
	unsigned int 
		vbuffLen, 
		nbuffLen, 
		tbuffLen, 
		ibuffLen;

	float* vbuff;
	float* nbuff;
	float* tbuff;
	unsigned int* ibuff;
};

class MeshBuilder {
public:
	static void buildGridNxN(Mesh& mesh, int N);
};

class GridFillerMeshNxN : public GridFillerNxN {
private:
	const float d = 1.0f / (n - 1); // step distance
	const OpenMesh::Vec3f o = OpenMesh::Vec3f(-1.0f, 1.0f, 0.0f); // origin
	const OpenMesh::Vec3f ux = OpenMesh::Vec3f(1.0f, 0.0f, 0.0f); // unit x direction
	const OpenMesh::Vec3f uy = OpenMesh::Vec3f(0.0f, -1.0f, 0.0f); // unit y direction
	std::vector<OpenMesh::VertexHandle> handle_table; // table storing vertex handles for easy grid connectivity establishment
	Mesh& mesh;

	virtual void fill_cell(int i, int j);

public:
	GridFillerMeshNxN(Mesh& mesh, int n);
};

class GridFillerIBuffNxN : public GridFillerNxN {
private:
	unsigned int idx;
	unsigned int* ibuff;
	
	virtual void fill_cell(int i, int j);

public:
	GridFillerIBuffNxN(unsigned int* ibuff, int n);
};
