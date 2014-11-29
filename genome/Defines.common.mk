# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================


CFLAGS += -DLIST_NO_DUPLICATES
CFLAGS += -DCHUNK_STEP1=12

PROG := genome

SRCS += \
	gene.c \
	genome.c \
	segments.c \
	sequencer.c \
	table.c \
	$(LIB)/bitmap.c \
	$(LIB)/hash.c \
	$(LIB)/hashtable.c \
	$(LIB)/pair.c \
	$(LIB)/random.c \
	$(LIB)/list.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/thread.c \
	$(LIB)/vector.c \
#
OBJS := ${SRCS:.c=.o}


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================
