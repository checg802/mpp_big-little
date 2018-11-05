ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../Makefile.param
     include $(PARAM_FILE)
endif

SRC  := $(wildcard *.c)
OBJ  := $(SRC:%.c=%.o)

INC_FLAGS := -I$(REL_INC)
INC_FLAGS += -I$(SDK_PATH)/mpp/code/include/inner_inc
CFLAGS += -Wall -g -lpthread $(INC_FLAGS)
CFLAGS += $(LIBS_CFLAGS)
CFLAGS += -ldl

LIBS += $(REL_LIB)/lib_hiae.a
LIBS += $(REL_LIB)/libisp.a

LIBS += $(REL_LIB)/lib_hiawb.a
LIBS += $(REL_LIB)/lib_hiaf.a
LIBS += $(REL_LIB)/lib_hidefog.a

LIBS += $(REL_LIB)/libmpi.a
LIBS += $(REL_LIB)/libhistitch.a

LIBS += $(REL_LIB)/libhifisheyecalibrate.a

# sensor input type: imx274 
SNS:=$(MAKECMDGOALS)

CFLAGS += $(LIBS_CFLAGS)

ifeq ($(SNS),)
SNS:=imx274
endif

ifeq ($(SNS), imx274)
LIBS += $(REL_LIB)/libsns_imx274.a
CFLAGS += -DSNS_IMX274
endif


CFLAGS += -lm

include $(PWD)/Make.$(OSTYPE)
