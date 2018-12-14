#pragma once
#include <Eigen/Dense>
#include <Eigen/Sparse>

class StateBuffer {
public:
	typedef Eigen::VectorXf State;
private:
	State *_prev_state, *_prev_prev_state;
public:
	StateBuffer();
	StateBuffer(State* const _prev_state,
		State* const _prev_prev_state);
	State* prev_state() const;
	State* prev_prev_state() const;
	void push(State* const state);
	~StateBuffer();

};

struct mass_spring_system { 
	typedef Eigen::SparseMatrix<float> SparseMatrix;
	typedef Eigen::VectorXf VectorXf;
	
	// parameters
	unsigned int n_points;
	unsigned int n_springs;
	float time_step;
	VectorXf rest_lengths;
	float damping_factor;
	

	// system matrices
	SparseMatrix M;
	SparseMatrix L;
	SparseMatrix J;

	// external forces
	VectorXf fext;
	
	mass_spring_system(
		unsigned int n_points,       // number of points
		unsigned int n_springs,      // number of springs
		float time_step,             // time step
		VectorXf rest_lengths,       // spring rest lengths
		float damping_factor,        // damping factor
		SparseMatrix M,              // mass matrix
		SparseMatrix L,              // L matrix
		SparseMatrix J,              // J matrix
		VectorXf fext                // external forces
	);
};

class MassSpringSolver {
private:
	typedef Eigen::VectorXf VectorXf;
	typedef Eigen::SparseMatrix<float> SparseMatrix;
	typedef Eigen::SimplicialCholesky<Eigen::SparseMatrix<float> > Cholesky;
	typedef Eigen::Map<Eigen::VectorXf> Map;

	// system
	mass_spring_system* system;
	Cholesky system_matrix;

	// state
	Map current_state;
	StateBuffer prev_states;
	VectorXf spring_directions;
	VectorXf inertial_term;

	// inertial term computation
	void computeInertialTerm();

	// steps
	void globalStep();
	void localStep();

public:
	MassSpringSolver(mass_spring_system* system, float* vbuff);

	// solve iterations
	void solveStep();
	void solveSteps(unsigned int n);

	// move to next time step
	void pushState();

};

static mass_spring_system* UniformGrid(
	unsigned int n,          // grid size
	float time_step,         // time step
	float rest_length,       // spring rest length (non-diagonal)
	float stiffness,         // spring stiffness
	float mass,              // node mass
	float damping_factor,    // damping factor
	float gravity,           // gravitationl force (-z axis)
	float* points            // vertex buffer
);