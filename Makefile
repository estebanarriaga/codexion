NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
UNSAFE_FLAGS = -pthread
RM = rm -f

SRCS = coders/main.c coders/utils.c coders/parse.c
OBJS = $(SRCS:.c=.o)

all: $(NAME)

unsafe: CFLAGS = $(UNSAFE_FLAGS)
unsafe: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
