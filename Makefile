srcs = $(wildcard src/*.cpp) 

all:
	g++ -Wall $(srcs) -o main -l DXGI -l D3D11