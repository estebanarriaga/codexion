
#include "codexion.h"

static int	parse_positive_int(char *arg, int *value)
{
	long	result;
	int		i;
	int		digit;

	if (arg[0] == '\0')
		return (-1);
	result = 0;
	i = 0;
	while (arg[i] != '\0')
	{
		if (arg[i] < '0' || arg[i] > '9')
			return (-1);
		digit = arg[i] - '0';
		if (result > (INT_MAX - digit) / 10)
			return (-1);
		result = result * 10 + digit;
		i++;
	}
	if (result <= 0)
		return (-1);
	*value = (int)result;
	return (0);
}

static void	set_fields(t_params *params, int *fields[7])
{
	fields[0] = &params->number_of_coders;
	fields[1] = &params->time_to_burnout;
	fields[2] = &params->time_to_compile;
	fields[3] = &params->time_to_debug;
	fields[4] = &params->time_to_refactor;
	fields[5] = &params->number_of_compiles_required;
	fields[6] = &params->dongle_cooldown;
}

int	parse_arguments(int argc, char *argv[], t_params *params)
{
	int	i;
	int	*fields[7];

	if (argc != 9)
	{
		print_usage(argv[0]);
		return (-1);
	}
	set_fields(params, fields);
	i = 0;
	while (i < 7)
	{
		if (parse_positive_int(argv[i + 1], fields[i]) == -1)
		{
			fprintf(stderr, "Error: argument %d (%s) must be a positive "
				"integer between 1 and %d.\n", i + 1, argv[i + 1], INT_MAX);
			return (-1);
		}
		i++;
	}
	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
	{
		fprintf(stderr, "Error: scheduler must be exactly fifo or edf.\n");
		return (-1);
	}
	params->scheduler = argv[8];
	return (0);
}