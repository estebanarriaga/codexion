
#include "codexion.h"

void	print_usage(char *program)
{
	fprintf(stderr, "Usage: %s number_of_coders time_to_burnout "
		"time_to_compile time_to_debug time_to_refactor "
		"number_of_compiles_required dongle_cooldown scheduler\n", program);
}

// int gettimeofday(struct timeval *tv, struct timezone *tz);
/**
 * time_code -> SECONDS MILLISECONDS MICROSECONDS
 * gettime returns the current time in milliseconds.
 */
long	gettime(t_time_code time_code)
{
	struct timeval	tv;

	if (gettimeofday(&tv, NULL))
	{
		handle_error("Gettimeofday failed.", errno);
		return (-1);
	}
	if (time_code == SECONDS)
		return (tv.tv_sec + tv.tv_usec / 1e6);
	else if (time_code == MILLISECONDS)
		return (tv.tv_sec * 1e3 + tv.tv_usec / 1e3);
	if (time_code == MICROSECONDS)
		return (tv.tv_sec * 1e6 + tv.tv_usec);
	handle_error("Invalid time code.", EINVAL);
	return (-1);
}
