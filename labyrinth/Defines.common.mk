# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================


LIBS += -lm

PROG := labyrinth

SRCS += \
	coordinate.c \
	grid.c \
	labyrinth.c \
	maze.c \
	router.c \
	$(LIB)/list.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/pair.c \
	$(LIB)/queue.c \
	$(LIB)/random.c \
	$(LIB)/thread.c \
	$(LIB)/vector.c \
#
OBJS := ${SRCS:.c=.o}

CFLAGS += -DUSE_EARLY_RELEASE


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================
