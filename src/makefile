all: render 

render: render.cpp octree.cpp morton.cpp
	clang++ -o $@ $< -O3 -std=c++11 -L/usr/local/lib -lOpenMeshCore
