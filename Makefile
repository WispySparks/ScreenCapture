# srcs = $(wildcard src/*.cpp) 
srcs = src/main.cpp src/util.cpp

all:
	g++ -Wall $(srcs) -o main -l gdi32 -l d3d11 -l dxgi -std=c++20