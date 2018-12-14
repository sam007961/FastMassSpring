#include "MassSpringSolver.h"

StateBuffer::StateBuffer() : _prev_state(new State()), _prev_prev_state(new State()) {}
StateBuffer::StateBuffer(State* const _prev_state, State* const _prev_prev_state)
	: _prev_state(new State(_prev_state)), _prev_prev_state(new State(_prev_prev_state)) {}

StateBuffer::State* StateBuffer::prev_state() const { return _prev_state; }
StateBuffer::State* StateBuffer::prev_prev_state() const { return _prev_prev_state; }

void StateBuffer::push(State* state) {
	delete _prev_prev_state;
	_prev_prev_state = _prev_state;
	_prev_state = state;
}

StateBuffer::~StateBuffer() { delete _prev_state; delete _prev_prev_state; }

mass_spring_system::mass_spring_system(
	unsigned int n_points,
	unsigned int n_springs,
	float time_step,
	VectorXf rest_lengths,
	float damping_factor,
	SparseMatrix system_matrix,
	SparseMatrix M,
	SparseMatrix J,
	VectorXf fext
) 
	: n_points(n_points), n_springs(n_springs),
	time_step(time_step), rest_lengths(rest_lengths),
	damping_factor(damping_factor), M(M), L(L), J(J),
	fext(fext) {}

MassSpringSolver::MassSpringSolver(mass_spring_system* system, float* vbuff) 
	: system(system), current_state(vbuff, system->n_points * 3) {
	// short hands
	float h2 = system->time_step * system->time_step;
	float a = system->damping_factor;
	
	// pre-factor system matrix
	SparseMatrix A = system->M + h2 * system->L;
	system_matrix.analyzePattern(A);
	system_matrix.factorize(A); 

	// compute local step to find initial spring directions
	localStep();

	// compute initial inertial term
	computeInertialTerm();
	
}

void MassSpringSolver::computeInertialTerm() {
	// short hands
	float a = system->damping_factor;

	// compute inertial term
	inertial_term = (a + 1) * (*prev_states.prev_state())
		- a * (*prev_states.prev_prev_state());
}

void MassSpringSolver::globalStep() {
	// short hands
	float h2 = system->time_step * system->time_step;
	float a = system->damping_factor;
	VectorXf& y = inertial_term;

	// compute right hand side
	VectorXf b = system->M * y + h2 * system->J * spring_directions
		+ h2 * system->fext;

	// solve system and update state
	current_state = system_matrix.solve(b);
}

void MassSpringSolver::localStep() {

}

void MassSpringSolver::solveStep() {
	globalStep();
	localStep();
}

void MassSpringSolver::solveSteps(unsigned int n) {
	for (unsigned int i = 0; i < n; i++) {
		solveStep();
	}
}

void MassSpringSolver::pushState() {
	VectorXf* state = new VectorXf(current_state);
	prev_states.push(state);
	computeInertialTerm();
}