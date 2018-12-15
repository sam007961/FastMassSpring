#pragma once
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

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
	static void buildGridNxN(Mesh& mesh, float w, int N);
	static void buildGridIBuffNxN(unsigned int* ibuff, int N);
};