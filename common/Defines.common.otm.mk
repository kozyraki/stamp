# ==============================================================================
#
# Defines.common.otm.mk
#
# ==============================================================================

OTM_DIR := /tcc/u/wkbaek/OpenTM
OTM_COMPILER_DIR := $(OTM_DIR)/build_c_cyc/gcc
LIBOTM_DIR := $(OTM_DIR)/gcc/libotm
LINUX_BIN_DIR := /usr/local/x86_64-unknown-linux-gnu/bin

CC := $(OTM_COMPILER_DIR)/xgcc -B$(OTM_COMPILER_DIR) -B$(LINUX_BIN_DIR)

CFLAGS_OTM      := -DOTM
CFLAGS_OTM      += -fopenmp
CFLAGS_OTM      += -I $(LIBOTM_DIR)

CFLAGS          += -g -Wall -pthread
CFLAGS          += -O3
CFLAGS          += -I$(LIB)
CPP             := g++
CPPFLAGS        += $(CFLAGS)
LD              := g++

LIBOTM_STM      := $(LIBOTM_DIR)/libotm_stm.a
LIBOTM_STM_SIM  := $(LIBOTM_DIR)/libotm_stm_sim.a
LIBOTM_HTM_SIM  := $(LIBOTM_DIR)/libotm_htm_sim.a

LIBS            += -lpthread
LIBS            += -lm

# Remove these files when doing clean
OUTPUT +=

LIB := ../lib

STM := ../../tl2

LOSTM := ../../OpenTM/lostm


# ==============================================================================
#
# End of Defines.common.otm.mk
#
# ==============================================================================
