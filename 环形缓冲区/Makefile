#CROSS := arm-hisiv300-linux-
CC := $(CROSS)gcc
CFLAG := -lpthread -I./include
BINS := stream_write stream_read1 stream_read2
SRCS := $(wildcard ./src/*.c)
OBJS := $(wildcard ./src/*.o)

all: $(BINS)

stream_write:
	$(CC) stream_write.c $(SRCS) -o stream_write $(CFLAG)

stream_read1:
	$(CC) stream_read1.c $(SRCS) -o stream_read1 $(CFLAG)

stream_read2:
	$(CC) stream_read2.c $(SRCS) -o stream_read2 $(CFLAG)

	
clean:
	@rm -rf $(BINS)
	@rm -rf $(OBJS)