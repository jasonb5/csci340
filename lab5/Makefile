# Makefile for the CS:APP Shell Lab


VERSION = 1
sim = ./sim
#simec= ./simec
CC = gcc
CXX = g++
CFLAGS = -Wall -O
CPPFLAGS = -g -Igoogletest/include
#FILES = $(sim) $(simec)
FILES = $(sim)

all: $(FILES)

test: schedule_test.o schedule.o
	$(CXX) -o test schedule_test.o schedule.o -pthread -Lgoogletest -lgtest
	./test

sim: sim.o schedule.o
	$(CC) -o sim sim.o schedule.o

#simec: simec.o schedule_ec.o
#	$(CC) -o simec simec.o schedule_ec.o

clean:
	rm -f $(FILES) *.o
