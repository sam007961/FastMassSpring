#pragma once
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <vector>
#include <unordered_map>
#include <unordered_set>


struct mass_spring_system { 
	typedef Eigen::SparseMatrix<float> SparseMatrix;
	typedef Eigen::VectorXf VectorXf;
	typedef std::pair<unsigned int, unsigned int> Edge;
	typedef std::vector<Edge> EdgeList;
	
	// parameters
	unsigned int n_points; // number of points
	unsigned int n_springs; // number of springs
	float time_step; // time step
	EdgeList spring_list; // spring edge list
	VectorXf rest_lengths; // spring rest lengths
	VectorXf stiffnesses; // spring stiffnesses
	VectorXf masses; // point masses
	VectorXf fext; // external forces
	float damping_factor; // damping factor
	

	
	
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
	typedef Eigen::Triplet<float> Triplet;
	typedef std::vector<Triplet> TripletList;

	// system
	mass_spring_system* system;
	Cholesky system_matrix;

	// M, L, J matrices
	SparseMatrix M;
	SparseMatrix L;
	SparseMatrix J;

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
	typedef std::vector<unsigned int> IndexList;

public:
	static mass_spring_system* buildUniformGrid(
		unsigned int n,          // grid width
		float time_step,         // time step
		float rest_length,       // spring rest length (non-diagonal)
		float stiffness,         // spring stiffness
		float mass,              // node mass
		float damping_factor,    // damping factor
		float gravity            // gravitationl force (-z axis)
	);


	// indices
	static IndexList buildUniformGridStructIndex(unsigned int n); // structural springs
	static IndexList buildUniformGridShearIndex(unsigned int n); // shearing springs
	static IndexList buildUniformGridBendIndex(unsigned int n); // bending springs
};


class ConstraintVisitor;

class MassSpringConstraint {
protected:
	typedef std::vector<MassSpringConstraint*> NodeList;
	NodeList children;

public:
	virtual bool checkCondition(unsigned int i) const = 0;
	virtual void satisfy() = 0;
	bool accept(ConstraintVisitor* visitor);

};

class RootConstraint : public MassSpringConstraint {
public:
	virtual bool checkCondition(unsigned int i);
	virtual void satisfy();
};

class PointConstraint : public MassSpringConstraint {
protected:
	std::unordered_set<unsigned int> target;

public:


};

class SpringConstraint : public MassSpringConstraint {
protected:
	typedef std::pair<unsigned int, unsigned int> Edge;
	typedef std::unordered_set<Edge> EdgeSet;

};

class PointQueryVisitor;
class SpringQueryVisitor;
class SatisfyVisitor;

class MassSpringConstrainer {
private:
	typedef Eigen::Vector3f Vector3f;
	typedef std::unordered_map<int, Vector3f> index_map;

	mass_spring_system* system; // mass-spring system
	float* vbuff; // vertex buffer
	index_map fix_map; // list of fixed points

public:
	MassSpringConstrainer(mass_spring_system* system, float* vbuff);
	void fixPoint(int i); // add point at index i to list
	void releasePoint(int i); // remove point at index i from list
	void constrainSprings(); // prevent exessive spring deformation
	void constrainPoints(); // fix points to list values
};