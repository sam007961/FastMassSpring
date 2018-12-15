#include "Mesh.h"

void MeshBuilder::buildGridNxN(Mesh& mesh, int n) {
	// request mesh properties
	mesh.request_vertex_normals();
	mesh.request_vertex_normals();
	mesh.request_vertex_texcoords2D();

	// generate mesh
	const float d = 4.0f / (n - 1); // step distance
	const OpenMesh::Vec3f o = OpenMesh::Vec3f(-1.0f, 1.0f, 0.0f); // origin
	const OpenMesh::Vec3f ux = OpenMesh::Vec3f(1.0f, 0.0f, 0.0f); // unit x direction
	const OpenMesh::Vec3f uy = OpenMesh::Vec3f(0.0f, -1.0f, 0.0f); // unit y direction
	std::vector<OpenMesh::VertexHandle> handle_table(n * n); // table storing vertex handles for easy grid connectivity establishment

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
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
	}

	// calculate normals
	mesh.request_face_normals();
	mesh.update_normals();
	mesh.release_face_normals();
}

void MeshBuilder::buildGridIBuffNxN(unsigned int* ibuff, int n) {
	unsigned int idx = 0;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
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
	}
}