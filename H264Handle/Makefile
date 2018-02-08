
INC_DIR := 
INC := $(patsubst %,-I%/include,$(INC_DIR)) -I./

SRC := $(wildcard *.cpp *.c $(patsubst %,%/*.cpp,$(INC_DIR)) $(patsubst %,%/*.c,$(INC_DIR)))
OBJ := $(patsubst %.cpp,%.obj,$(patsubst %.c,%.o,$(SRC)))
DEP := $(patsubst %.obj,%.dep,$(patsubst %.o,%.d,$(OBJ)))

COMPILE_PREFIX := @echo -n ".";
CC := $(COMPILE_PREFIX)cc
CFLAGS := $(INC) -O0 -Wall -DBSD=1 -fPIC -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DALLOW_SERVER_PORT_REUSE -lrt 
CXX := $(COMPILE_PREFIX)c++
CXXFLAGS := $(CFLAGS)
LD := $(COMPILE_PREFIX)c++ -o
LDS := $(COMPILE_PREFIX)c++ -shared -o
LDSS := $(COMPILE_PREFIX)ar cr
STRIP := $(COMPILE_PREFIX)strip
LDFLAGS := -pthread -lrt 

TARGET := H264Handle

.PHONY : all clean

all : $(TARGET)

%.o : %.c
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c $< -o $@

%.obj : %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -MF"$(@:%.obj=%.dep)" -MT"$(@:%.obj=%.dep)" -c $< -o $@

$(TARGET) : $(OBJ)
	$(LD) $@ $(LDFLAGS) $^
	$(STRIP) $@

clean:
	@rm -rf $(OBJ) $(DEP) $(TARGET)

sinclude $(DEP)

