UNAME := $(shell uname)


BIN=bin

CFLAGS = -std=gnu2x -Og -g
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm
SRC = $(wildcard src/*.c) $(wildcard src/render/*.c)
OBJ = $(SRC:%.c=$(BIN)/%.o)

CC = cc

.PHONY: all dirs run

all: dirs chess_2

run:
	./bin/chess_2.out

dirs:
	mkdir -p bin/src/ bin/src/thc bin/src/render

clean:
	rm -rf $(BIN)/*
chess_2: $(OBJ)
	gcc -o $(BIN)/$@.out $(CFLAGS) $(OBJ) $(LDFLAGS) -L$(BIN) -lthc -lstdc++
	
$(BIN)/%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# thc for chess moves
THC_DIR=src/thc
THC_SRC=$(wildcard $(THC_DIR)/*.cpp)

THC_FLAGS=-g -Og -std=c++17

thc: $(THC_OBJ)
	g++ -c $(THC_FLAGS) -o $(BIN)/thc.o $(THC_DIR)/thc.cpp
	g++ -c $(THC_FLAGS) -o $(BIN)/thc_wrap.o $(THC_DIR)/thc_wrap.cpp
	ar rcs $(BIN)/libthc.a $(BIN)/thc.o $(BIN)/thc_wrap.o