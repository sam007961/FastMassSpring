#pragma once
#define _USE_MATH_DEFINES
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

// Mesh type
typedef OpenMesh::TriMesh_ArrayKernelT<> Mesh;

// Macros for extracting buffers from OpenMesh
#define VERTEX_DATA(mesh) (float*) &(mesh.point(*mesh.vertices_begin()))
#define NORMAL_DATA(mesh) (float*) &(mesh.normal(*mesh.vertices_begin()))
#define TEXTURE_DATA(mesh) (float*) &(mesh.texcoords2D(*mesh.vertices_begin()))


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