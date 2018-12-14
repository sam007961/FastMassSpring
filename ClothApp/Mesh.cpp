#include "Mesh.h"

void MeshBuilder::buildGridNxN(Mesh& mesh, int n) {
	// request mesh properties
	mesh.request_vertex_normals();
	mesh.request_vertex_normals();
	mesh.request_vertex_texcoords2D();

	// generate mesh
	GridFillerMeshNxN filler(mesh, n);
	filler.fill();

	// calculate normals
	mesh.request_face_normals();
	mesh.update_normals();
	mesh.release_face_normals();
}

GridFillerMeshNxN::GridFillerMeshNxN(Mesh& mesh, int n)
	: GridFillerNxN(n), mesh(mesh) {
	handle_table.resize(n*n);
}

void GridFillerMeshNxN::fill_cell(int i, int j) {
	handle_table[j + i * n] = mesh.add_vertex(o + d*j*ux + d*i*uy); // add vertex
	mesh.set_texcoord2D(handle_table[j + i * n], OpenMesh::Vec2f(d*j, d*i)); // add texture coordinates

	//add connectivity
	if (i > 0 && j < n - 1) {
		mesh.add_face(
			handle_table[j + i * n],
			handle_table[j + 1 + (i - 1) * n],
			handle_table[j + (i - 1) * n]
		);
	}

	if (j > 0 && i > 0) {
		mesh.add_face(
			handle_table[j + i * n],
			handle_table[j + (i - 1) * n],
			handle_table[j - 1 + i * n]
		);
	}
}

GridFillerIBuffNxN::GridFillerIBuffNxN(unsigned int* ibuff, int n) 
	: GridFillerNxN(n), ibuff(ibuff) {}

void GridFillerIBuffNxN::fill_cell(int i, int j) {
	if (i > 0 && j < n - 1) {
		ibuff[idx++] = j + i * n;
		ibuff[idx++] = j + 1 + (i - 1) * n;
		ibuff[idx++] = j + (i - 1) * n;
	}

	if (j > 0 && i > 0) {
		ibuff[idx++] = j + i * n;
		ibuff[idx++] = j + (i - 1) * n;
		ibuff[idx++] = j - 1 + i * n;
	}
}