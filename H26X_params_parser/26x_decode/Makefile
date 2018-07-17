#CROSS := arm-hisiv300-linux-
CC := $(CROSS)g++
CFLAG := -Wno-write-strings -lpthread  \
		-I./include \
		
BINS := parser

SRCS := $(wildcard ./src/*.cpp)
OBJS := $(wildcard ./src/*.o )

all: $(BINS)

parser:
	$(CC) main.cpp $(SRCS) -o parser $(CFLAG)
	
clean:
	@rm -rf $(BINS)
	@rm -rf $(OBJS)