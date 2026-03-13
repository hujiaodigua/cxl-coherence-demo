# Makefile for atomic + numa + pthread test

CC = gcc

CFLAGS = -std=c11 -O2 -Wall

LIBS = -pthread -lnuma

TARGET = cxl-coherence-demo

SRC = cxl-coherence-demo.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# 运行程序，绑定 CPU node 0，CXL memory node 2
#run: $(TARGET)
#	numactl --cpunodebind=0 --membind=2 ./$(TARGET)

# 清理
clean:
	rm -f $(TARGET)

