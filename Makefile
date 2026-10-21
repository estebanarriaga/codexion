NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -Icoders
UNSAFE_FLAGS = -pthread
RM = rm -f
RMDIR = rm -rf

SRCS = coders/main.c coders/utils.c coders/parse.c coders/initialize.c \
	coders/coders.c coders/dongles.c coders/heap.c \
	coders/utils/error.c \
	coders/utils/handlers.c coders/scheduler.c coders/monitor.c
OBJS = $(patsubst %.c,dist/%.o,$(SRCS))
LEGACY_OBJS = $(SRCS:.c=.o)

all: $(NAME)

run: $(NAME)
	./$(NAME) 5 800 200 200 200 3 100 fifo

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

dist/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RMDIR) dist
	$(RM) $(LEGACY_OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
