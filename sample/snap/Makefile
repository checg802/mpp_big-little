# Hisilicon Hi35xx sample Makefile

include ../Makefile.param

SRCS := $(wildcard *.c)
TARGET := $(SRCS:%.c=%)

ifeq ($(CONFIG_SNAP),y)
ifeq ($(CONFIG_PHOTO_ALGS),y)
MPI_LIBS += -lrt -lstdc++ \
			$(REL_LIB)/libmpi_photo.a
endif
endif

# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
