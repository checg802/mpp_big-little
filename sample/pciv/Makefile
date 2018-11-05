
include ../Makefile.param

LIBS:=$(REL_LIB)/libpciv.a $(REL_LIB)/libmpi.a

INC_FLAGS += -I$(REL_INC) -I../common

# target source
SRC  := $(wildcard *.c)  
OBJ  := $(SRC:%.c=%.o)

TARGET := $(OBJ:%.o=%)

#CFLAGS += -Wall -O2
#CFLAGS += -lpthread -lm 

slave:
	$(CC) -o sample_pciv_slave sample_pciv_slave.c pciv_msg.c pciv_trans.c $(COMMON_DIR)/loadbmp.c sample_common.c $(COMMON_DIR)/sample_comm_sys.c $(COMMON_DIR)/sample_comm_isp.c $(COMMON_DIR)/sample_comm_vi.c $(COMMON_DIR)/sample_comm_vpss.c $(COMMON_DIR)/sample_comm_venc.c $(INC_FLAGS) $(CFLAGS) $(LIBS) $(AUDIO_LIBA) $(SENSOR_LIBS)

clean:
	@rm sample_pciv_slave *.o -fr

cleanstream:
	@rm -f *.h264
	@rm -f *.h265
	@rm -f *.jpg
	@rm -f *.mjp
	@rm -f *.mp4
