# Hisilicon Hi3519 sample Makefile

include ../Makefile.param

# target source
SRCS  := $(wildcard *.c) 
TARGET := $(SRCS:%.c=%)

# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
