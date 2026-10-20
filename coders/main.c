
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
	status = initialize_simulation(&sim, &params);
	if (status != 0)
		return (status);

	// Create threads for each coder and start the simulation
	status = create_coder_threads(&sim);
	if (status != 0)
		return (status);

	// TODO: Create simulation thread
	status =  pthread_create(&sim.scheduler_thread, NULL, scheduler_thread, &sim);
	if (status != 0)
	{
		fprintf(stderr, "Error creating simulation thread: %d\n", status);
		return (status);
	}
	// TODO: Create monitor thread
	status =  pthread_create(&sim.monitor_thread, NULL, monitor_thread, &sim);
	if (status != 0)
	{
		fprintf(stderr, "Error creating monitor thread: %d\n", status);
		return (status);
	}

	if (wait_for_threads(&sim) != 0)
		return (1);
	safe_thread_handle(&sim.monitor_thread, JOIN, NULL, NULL);
	safe_thread_handle(&sim.scheduler_thread, JOIN, NULL, NULL);

	// TODO: Clean up resources
    // cleanup_resources(&sim);

	return (0);
}
