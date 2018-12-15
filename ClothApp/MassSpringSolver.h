#pragma once
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <vector>
#include <unordered_map>

struct mass_spring_system { 
	typedef Eigen::SparseMatrix<float> SparseMatrix;
	typedef Eigen::VectorXf VectorXf;
	typedef std::pair<unsigned int, unsigned int> Edge;
	typedef std::vector<Edge> EdgeList;
	typedef Eigen::Triplet<float> Triplet;
	typedef std::vector<Triplet> TripletList;
	
	// parameters
	unsigned int n_points; // number of points
	unsigned int n_springs; // number of springs
	float time_step; // time step
	EdgeList spring_list; // spring edge list
	VectorXf rest_lengths; // spring rest lengths
	VectorXf fext; // external forces
	float damping_factor; // damping factor
	

	// system matrices
	SparseMatrix M;
	SparseMatrix L;
	SparseMatrix J;
	
	mass_spring_system(
		unsigned int n_points,       // number of points
		unsigned int n_springs,      // number of springs
		float time_step,             // time step
		EdgeList spring_list,        // spring edge list
		VectorXf rest_lengths,       // spring rest lengths
		VectorXf stiffnesses,        // spring stiffnesses
		VectorXf masses,             // point masses
		VectorXf fext,               // external forces
		float damping_factor         // damping factor
	);
};

class MassSpringSolver {
private:
	typedef Eigen::Vector3f Vector3f;
	typedef Eigen::VectorXf VectorXf;
	typedef Eigen::SparseMatrix<float> SparseMatrix;
	typedef Eigen::SimplicialCholesky<Eigen::SparseMatrix<float> > Cholesky;
	typedef Eigen::Map<Eigen::VectorXf> Map;
	typedef std::pair<unsigned int, unsigned int> Edge;

	// system
	mass_spring_system* system;
	Cholesky system_matrix;

	// state
	Map current_state; // q(n), current state
	VectorXf prev_state; // q(n - 1), previous state
	VectorXf spring_directions; // d, spring directions
	VectorXf inertial_term; // M * y, y = (a + 1) * q(n) - a * q(n - 1)

	// steps
	void globalStep();
	void localStep();

public:
	MassSpringSolver(mass_spring_system* system, float* vbuff);

	// solve iterations
	void solve(unsigned int n);
	void timedSolve(unsigned int ms);
};

class MassSpringBuilder {
private:
	typedef Eigen::Vector3f Vector3f;
	typedef Eigen::VectorXf VectorXf;
	typedef std::pair<unsigned int, unsigned int> Edge;
	typedef std::vector<Edge> EdgeList;
	typedef Eigen::Triplet<float> Triplet;
	typedef std::vector<Triplet> TripletList;

public:
	static mass_spring_system* UniformGrid(
		unsigned int n,          // grid size
		float time_step,         // time step
		float rest_length,       // spring rest length (non-diagonal)
		float stiffness,         // spring stiffness
		float mass,              // node mass
		float damping_factor,    // damping factor
		float gravity            // gravitationl force (-z axis)
	);
};

class PointFixer {
private:
	typedef Eigen::Vector3f Vector3f;
	typedef std::unordered_map<int, Vector3f> index_map;

	int buffSize; // size of vertex buffer
	float* vbuff; // vertex buffer
	index_map fix_map; // list of fixed points

public:
	PointFixer(float* vbuff, int buffSize);
	void addPoint(int i); // add point at index i to list
	void removePoint(int i); // remove point at index i from list
	void fixPoints();
};