#include "MassSpringSolver.h"
#include <iostream>

// S Y S T E M //////////////////////////////////////////////////////////////////////////////////////
mass_spring_system::mass_spring_system(
	unsigned int n_points,
	unsigned int n_springs,
	float time_step,
	EdgeList spring_list,
	VectorXf rest_lengths,
	VectorXf stiffnesses,
	VectorXf masses,
	VectorXf fext,
	float damping_factor
)
	: n_points(n_points), n_springs(n_springs),
	time_step(time_step), spring_list(spring_list),
	rest_lengths(rest_lengths), fext(fext),
	damping_factor(damping_factor) {

	// compute L and J
	TripletList LTriplets, JTriplets;
	// L
	L.resize(3 * n_points, 3 * n_points);
	unsigned int k = 0; // spring counter
	for (Edge& i : spring_list) {
		for (int j = 0; j < 3; j++) {
			LTriplets.push_back(
				Triplet(3 * i.first  + j, 3 * i.first + j,   1 * stiffnesses[k]));
			LTriplets.push_back(
				Triplet(3 * i.first  + j, 3 * i.second + j, -1 * stiffnesses[k]));
			LTriplets.push_back(
				Triplet(3 * i.second + j, 3 * i.first  + j, -1 * stiffnesses[k]));
			LTriplets.push_back(
				Triplet(3 * i.second + j, 3 * i.second + j,  1 * stiffnesses[k]));
		}
		k++;
	}
	L.setFromTriplets(LTriplets.begin(), LTriplets.end());

	// J
	J.resize(3 * n_points, 3 * n_springs);
	k = 0; // spring counter
	for (Edge& i : spring_list) {
		for (int j = 0; j < 3; j++) {
			JTriplets.push_back(
				Triplet(3 * i.first  + j, 3 * k + j,  1 * stiffnesses[k]));
			JTriplets.push_back(
				Triplet(3 * i.second + j, 3 * k + j, -1 * stiffnesses[k]));
		}
		k++;
	}
	J.setFromTriplets(JTriplets.begin(), JTriplets.end());

	// compute M
	TripletList MTriplets;
	this->M.resize(3 * n_points, 3 * n_points);
	for (int i = 0; i < n_points; i++) {
		MTriplets.push_back(Triplet(3 * i + 0, 3 * i + 0, masses[i]));
		MTriplets.push_back(Triplet(3 * i + 1, 3 * i + 1, masses[i]));
		MTriplets.push_back(Triplet(3 * i + 2, 3 * i + 2, masses[i]));
	}
	this->M.setFromTriplets(MTriplets.begin(), MTriplets.end());
}

// S O L V E R //////////////////////////////////////////////////////////////////////////////////////
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

	std::cout << "next state: \n" << current_state << std::endl;
}

void MassSpringSolver::timedSolve(unsigned int ms) {
	// TODO
}


// B U I L D E R ////////////////////////////////////////////////////////////////////////////////////
mass_spring_system* MassSpringBuilder::UniformGrid(
	unsigned int n,
	float time_step,
	float rest_length,
	float stiffness,
	float mass,
	float damping_factor,
	float gravity
) {
	// n must be odd
	assert(n % 2 == 1);

	// compute n_points and n_springs
	unsigned int n_points = n * n;
	unsigned int n_springs = (n - 1) * (5 * n - 2);

	// build mass list
	VectorXf masses(mass * VectorXf::Ones(n_springs));

	// build spring list and spring parameters
	EdgeList spring_list;
	VectorXf rest_lengths(n_springs);
	VectorXf stiffnesses(n_springs);
	unsigned int k = 0; // spring counter
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < n; j++) {
			// bottom right corner
			if(i == n - 1 && j == n - 1) {
				continue;
			}

			if (i == n - 1) {
				// structural spring
				spring_list.push_back(Edge(n * i + j, n * i + j + 1));
				rest_lengths[k++] = rest_length;

				// bending spring
				if (j % 2 == 0) {
					spring_list.push_back(Edge(n * i + j, n * i + j + 2));
					rest_lengths[k++] = 2 * rest_length;
				}
				continue;
			}

			// right edge
			if (j == n - 1) {
				// structural spring
				spring_list.push_back(Edge(n * i + j, n * (i + 1) + j));
				rest_lengths[k++] = rest_length;

				// bending spring
				if (i % 2 == 0){
					spring_list.push_back(Edge(n * i + j, n * (i + 2) + j));
					rest_lengths[k++] = 2 * rest_length;
				}
				continue;
			}

			// structural springs
			spring_list.push_back(Edge(n * i + j, n * i + j + 1));
			spring_list.push_back(Edge(n * i + j, n * (i + 1) + j));
			rest_lengths[k++] = rest_length;
			rest_lengths[k++] = rest_length;

			// shearing springs
			spring_list.push_back(Edge(n * i + j, n * (i + 1) + j + 1));
			spring_list.push_back(Edge(n * (i + 1) + j, n * i + j + 1));
			rest_lengths[k++] = 1.41421356237f * rest_length;
			rest_lengths[k++] = 1.41421356237f * rest_length;

			// bending springs
			if (j % 2 == 0) {
				spring_list.push_back(Edge(n * i + j, n * i + j + 2));
				rest_lengths[k++] = 2 * rest_length;
			}
			if (i % 2 == 0) {
				spring_list.push_back(Edge(n * i + j, n * (i + 2) + j));
				rest_lengths[k++] = 2 * rest_length;
			}
		}
	}

	// compute external forces
	VectorXf fext(Vector3f(0, 0, -gravity).replicate(n_points, 1));
	auto temp = new mass_spring_system(n_points, n_springs, time_step,
		spring_list, rest_lengths, stiffnesses, masses, fext, damping_factor);
	// temporary for testing
	/*std::cout << "M: \n" << temp->M << std::endl;
	std::cout << "L: \n" << temp->L << std::endl;
	std::cout << "J: \n" << temp->J << std::endl;
	std::cout << "fext: \n" << temp->fext << std::endl;*/
	return temp;
}