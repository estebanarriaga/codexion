
#include "codexion.h"

int main(int argc, char *argv[])
{
	t_params	params;
	t_sim		sim;

	if (parse_arguments(argc, argv, &params) == -1)
		return (1);
	(void)params;

	// Initialize coders, dongles, and other necessary data structures here
    initialize_simulation(&sim, &params);

	// Create threads for each coder and start the simulation
    create_coder_threads(&sim);

	// Wait for all threads to finish
    wait_for_threads(&sim);

	// Clean up resources
    cleanup_resources(&sim);

	return (0);
}
