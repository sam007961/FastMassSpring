#pragma once
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// Mass-Spring System struct
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

// Mass-Spring System Solver class
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

// Mass-Spring System Builder Class
// TODO: refactor builder class to be non-static, i.e implement proper builder pattern
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

// Constraint Graph
class CgNodeVisitor; // Constraint graph node visitor

// Constraint graph node
class CgNode {
protected:
	mass_spring_system* system;
	float* vbuff;

public:
	CgNode(mass_spring_system* system, float* vbuff);

	virtual bool query(unsigned int i) = 0; // check if item with index i is constrained
	virtual void satisfy() = 0; // satisfy constraint
	virtual bool accept(CgNodeVisitor& visitor) = 0; // accept visitor

};

// point constraint node
class CgPointNode : public CgNode {
public:
	CgPointNode(mass_spring_system* system, float* vbuff);

	virtual bool accept(CgNodeVisitor& visitor);

};

// spring constraint node
class CgSpringNode : public CgNode {
protected:
	typedef std::vector<CgNode*> NodeList;
	NodeList children;

public:
	CgSpringNode(mass_spring_system* system, float* vbuff);

	virtual bool accept(CgNodeVisitor& visitor);
	void addChild(CgNode* node);
	void removeChild(CgNode* node);
};

// root node 
class CgRootNode : public CgSpringNode {
public:
	CgRootNode(mass_spring_system* system, float* vbuff);

	virtual bool query(unsigned int i);
	virtual void satisfy();
	virtual bool accept(CgNodeVisitor& visitor);
};

class CgPointFixNode : public CgPointNode {
protected:
	typedef Eigen::Vector3f Vector3f;
	std::unordered_map<unsigned int, Vector3f> fix_map;
public:
	CgPointFixNode(mass_spring_system* system, float* vbuff);
	virtual bool query(unsigned int i);
	virtual void satisfy();

	void fixPoint(int i); // add point at index i to list
	void releasePoint(int i); // remove point at index i from list
};

// TODO: add spring deformation constraint

//class CgSpringDeformationNode : public CgSpringNode {
//protected:
//	std::unordered_set<unsigned int> items;
//
//public:
//	virtual bool query(unsigned int i);
//	virtual void satisfy();
//};

class CgNodeVisitor {
public:
	virtual bool visit(CgPointNode& node);
	virtual bool visit(CgSpringNode& node);
};

class CgPointQueryVisitor : public CgNodeVisitor {
private:
	unsigned int i;
	bool queryResult;
public:
	virtual bool visit(CgPointNode& node);

	bool queryPoint(CgNode& root, unsigned int i);
};

class CgSatisfyVisitor : public CgNodeVisitor {
public:
	virtual bool visit(CgPointNode& node);
	virtual bool visit(CgSpringNode& node);

	void satisfy(CgNode& root);
};