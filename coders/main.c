
#include "codexion.h"

int main(int argc, char *argv[])
{
	t_params	params;

	if (parse_arguments(argc, argv, &params) == -1)
		return (1);
	(void)params;

	// Initialize coders, dongles, and other necessary data structures here

    initialize_simulation(&sim, &params);

	// Create threads for each coder and start the simulation

	// Wait for all threads to finish

	// Clean up resources

	return (0);
}
