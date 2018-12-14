#include "MassSpringSolver.h"

mass_spring_system::mass_spring_system(
	unsigned int n_points,
	unsigned int n_springs,
	float time_step,
	EdgeList spring_list,
	VectorXf rest_lengths,
	VectorXf stiffness_list,
	SparseMatrix M,
	VectorXf fext,
	float damping_factor
)
	: n_points(n_points), n_springs(n_springs),
	time_step(time_step), spring_list(spring_list),
	rest_lengths(rest_lengths), fext(fext),
	damping_factor(damping_factor), M(M) {

	// compute L and J
	TripletList LTriplets, JTriplets;
	// L
	L.resize(3 * n_points, 3 * n_points);
	for (Edge& i : spring_list) {
		for (int j = 0; j < 3; j++) {
			LTriplets.push_back(Triplet(3 * i.first + j, 3 * i.first + j, 1));
			LTriplets.push_back(Triplet(3 * i.first + j, 3 * i.second + j, -1));
			LTriplets.push_back(Triplet(3 * i.second + j, 3 * i.first + j, -1));
			LTriplets.push_back(Triplet(3 * i.second + j, 3 * i.second + j, 1));
		}
	}
	L.setFromTriplets(LTriplets.begin(), LTriplets.end());

	// compute J
	J.resize(3 * n_points, 3 * n_springs);

	for (int i = 0; i < spring_list.size(); i++) {
		for (int j = 0; j < 3; j++) {
			JTriplets.push_back(Triplet(3 * spring_list[i].first + j, 3 * i + j, 1));
			JTriplets.push_back(Triplet(3 * spring_list[i].second + j, 3 * i + j, -1));
		}
	}
	J.setFromTriplets(JTriplets.begin(), JTriplets.end());
}

MassSpringSolver::MassSpringSolver(mass_spring_system* system, float* vbuff) 
	: system(system), current_state(vbuff, system->n_points * 3), 
	prev_state(current_state), spring_directions(system->n_springs * 3) {
	
	float h2 = system->time_step * system->time_step; // shorthand
	float a = system->damping_factor; // shorthand
	
	// pre-factor system matrix
	SparseMatrix A = system->M + h2 * system->L;
	system_matrix.analyzePattern(A);
	system_matrix.factorize(A); 
}

void MassSpringSolver::globalStep() {
	float h2 = system->time_step * system->time_step; // shorthand

	// compute right hand side
	VectorXf b = inertial_term
		+ h2 * system->J * spring_directions
		+ h2 * system->fext;

	// save current state in previous state
	prev_state = current_state;

	// solve system and update state
	current_state = system_matrix.solve(b);
}

void MassSpringSolver::localStep() {
	unsigned int j = 0;
	for (Edge& i : system->spring_list) {
		Vector3f p12(
			current_state[3 * i.first + 0] - current_state[3 * i.second + 0],
			current_state[3 * i.first + 1] - current_state[3 * i.second + 1],
			current_state[3 * i.first + 2] - current_state[3 * i.second + 2]
		);

		p12.normalize();
		spring_directions[3 * j + 0] = 	system->rest_lengths[j] * p12[0];
		spring_directions[3 * j + 1] =	system->rest_lengths[j] * p12[1];
		spring_directions[3 * j + 2] =	system->rest_lengths[j] * p12[2];
		j++;
	}
}

void MassSpringSolver::solve(unsigned int n) {
	float a = system->damping_factor; // shorthand

	// update inertial term
	inertial_term = system->M * ((a + 1) * (current_state) - a * prev_state);

	// perform steps
	for (unsigned int i = 0; i < n; i++) {
		localStep();
		globalStep();
	}
}

void MassSpringSolver::timedSolve(unsigned int ms) {
	// TODO
}


mass_spring_system* MassSpringBuilder::UniformGrid(
	unsigned int n,
	float time_step,
	float rest_length,
	float stiffness,
	float mass,
	float damping_factor,
	float gravity
) {
	// compute n_points and n_springs

	// compute stiffness and rest length lists

	// compute mass matrix

	// compute edge list

	// compute external forces
	return nullptr;
}