#include "MassSpringSystem.h"

MassSpringSystem::MassSpringSystem(
	unsigned int n_points,
	unsigned int n_springs,
	float time_step,
	float rest_length,
	float damping_factor,
	SparseXf M,
	SparseXf L,
	SparseXf J,
	VectorXf fext,
	float* points
) : n_points(n_points),n_springs(n_springs), time_step(time_step),
	rest_length(rest_length), damping_factor(damping_factor),
	M(M), SystemMatrix(M + (L * n_points * n_points)), J(J), 
	fext(fext), x(points, n_points * 3) {}
