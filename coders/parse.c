/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 12:32:09 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 12:32:30 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	parse_positive_int(char *arg, int *value, int i)
{
	int	result;

	result = atoi(arg);
	if (result <= 0)
	{
		fprintf(stderr, "Error: argument %d (%s) must be a positive "
			"integer between 1 and %d.\n", i + 1, arg, INT_MAX);
		return (-1);
	}
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
		if (parse_positive_int(argv[i + 1], fields[i], i) == -1)
			return (-1);
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
