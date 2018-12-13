#pragma once
#include <Eigen/Dense>
#include <Eigen/Sparse>
class MassSpringSystem {
private:
	typedef Eigen::MatrixXf MatrixXf;
	typedef Eigen::DiagonalMatrix<float, Eigen::Dynamic, Eigen::Dynamic> DiagonalMatrixXf;
	typedef Eigen::VectorXf VectorXf;
	typedef Eigen::Map<Eigen::VectorXf> ArrayXf;

	const float time_step;
	const float stiffness;
	const float rest_length;
	const float damping_factor;

	DiagonalMatrixXf M; // mass matrix
	MatrixXf L, J; // connectivity
	VectorXf d; // spring directions
	VectorXf fext; // external forces

	ArrayXf x; // x = q(n + 1), next state
	VectorXf y1, y2; // y1 = q(n - 1), y2 = q(n), previous two states

public:


};