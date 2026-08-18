NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -Icoders
UNSAFE_FLAGS = -pthread
RM = rm -f

SRCS = coders/main.c coders/utils.c coders/parse.c coders/initialize.c \
	coders/simulation.c coders/threads.c coders/utils/error.c \
	coders/utils/handlers.c
OBJS = $(SRCS:.c=.o)

all: $(NAME)

run: $(NAME)
	./$(NAME) 5 800 200 200 200 3 100 fifo

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
