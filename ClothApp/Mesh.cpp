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
	handle_table[i + j * n] = mesh.add_vertex(o + d*i*ux + d*j*uy); // add vertex
	mesh.set_texcoord2D(handle_table[i + j * n], OpenMesh::Vec2f(d*i, d*j)); // add texture coordinates

	//add connectivity
	if (j > 0 && i < n - 1) {
		mesh.add_face(
			handle_table[i + j * n],
			handle_table[i + 1 + (j - 1) * n],
			handle_table[i + (j - 1) * n]
		);
	}

	if (j > 0 && i > 0) {
		mesh.add_face(
			handle_table[i + j * n],
			handle_table[i + (j - 1) * n],
			handle_table[i - 1 + j * n]
		);
	}
}

GridFillerIBuffNxN::GridFillerIBuffNxN(unsigned int* ibuff, int n) 
	: GridFillerNxN(n), ibuff(ibuff) {}

void GridFillerIBuffNxN::fill_cell(int i, int j) {
	if (j > 0 && i < n - 1) {
		ibuff[idx++] = i + j * n;
		ibuff[idx++] = i + 1 + (j - 1) * n;
		ibuff[idx++] = i + (j - 1) * n;
	}

	if (j > 0 && i > 0) {
		ibuff[idx++] = i + j * n;
		ibuff[idx++] = i + (j - 1) * n;
		ibuff[idx++] = i - 1 + j * n;
	}
}