#pragma once
#include <Eigen/Dense>
#include <Eigen/Sparse>

class MassSpringSystem {
private:
	typedef Eigen::Vector3f Vector3f;
	typedef Eigen::VectorXf VectorXf;
	typedef Eigen::MatrixXf MatrixXf;
	typedef Eigen::SparseMatrix<float> SparseXf;
	typedef Eigen::Map<Eigen::VectorXf> ArrayXf;
	typedef Eigen::Triplet<float> Triplet;

	const float n_points;
	const float n_springs;
	const float time_step;
	//const float stiffness;
	const float rest_length;
	const float damping_factor;

	const SparseXf M; // mass matrix
	const SparseXf J; // connectivity
	const SparseXf SystemMatrix; // System Matrix
	VectorXf fext; // external forces

	VectorXf d; // spring directions
	ArrayXf x; // x = q(n + 1), next state
	VectorXf y1, y2; // y1 = q(n - 1), y2 = q(n), previous two states

public:
	MassSpringSystem(
		unsigned int n_points,  // number of points
		unsigned int n_springs, // number of springs
		float time_step,        // time step
		float rest_length,      // spring rest length
		float damping_factor,   // damping factor
		SparseXf M,             // mass matrix
		SparseXf L,             // connectivity and stiffness
		SparseXf J,             // connectivity and stiffness
		VectorXf fext,          // external forces
		float* points           // vertex buffer
	);

	void step();

	static MassSpringSystem* UniformGrid(
		unsigned int n,       // grid size
		float time_step,      // time step
		float rest_length,    // spring rest length
		float stiffness,      // spring stiffness
		float damping_factor, // damping factor
		Vector3f gravity,     // gravity force
		float* points         // vertex buffer
	);
};

