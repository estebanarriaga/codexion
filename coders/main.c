
#include "codexion.h"

// main
//    ├── initialize_simulation()
//    ├── create coder threads
//    ├── create scheduler thread
//    ├── create monitor thread
//    ├── join all threads
//    └── cleanup
int main(int argc, char *argv[])
{
	t_params	params;
	t_sim		sim;
	int			status;

	if (parse_arguments(argc, argv, &params) == -1)
		return (1);
	(void)params;

	// Initialize coders, dongles, and other necessary data structures here
	status = initialize_simulation(&sim, &params)
	if (status != 0)
		return (status);

	// Create threads for each coder and start the simulation
	status = create_coder_threads(&sim)
	if (status != 0)
		return (status);

	// TODO: Create simulation thread

	// TODO: Create monitor thread
	// Wait for all coder threads to finish
	if (wait_for_threads(&sim) != 0)
		return (1);

	// TODO: Clean up resources
    // cleanup_resources(&sim);

	return (0);
}
