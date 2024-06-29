# srcs = $(wildcard src/*.cpp) 
srcs = src/main.cpp src/util.cpp

all:
	g++ -Wall $(srcs) -o main -l gdi32 -std=c++20