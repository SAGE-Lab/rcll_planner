#*****************************************************************************
#           Makefile for smt_planning project
#                            -------------------
#   Created on Wed Mar 21 22:55 2018 by Igor Nicolai Bongartz
#
#*****************************************************************************

CC      = g++
CFLAGS  += -std=c++14
CFLAGS  += -I/home/smt_planning/
LDFLAGS = -lz3 -lboost_regex -lboost_program_options

all: smt_export clean

smt_export: smt_export.o smt_planner.o
	$(CC) -o $@ $^ $(LDFLAGS)

smt_export.o: smt_export.cpp
	$(CC) -c $(CFLAGS) $<

smt_planner.o: smt_planner.cpp
	$(CC) -c $(CFLAGS) $<

.PHONY: clean cleanest

clean:
	rm *.o

clean_formulas:
	rm formulas/*.smt
