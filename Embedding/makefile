LIBS=-lafcuda -lcusolver -lcudart -lcufft -lcublas -lnvrtc -lcusparse -llapack -lblas
AF_LIB_PATH=-L/home/anabur/Github/libraries/arrayfire/lib64
CU_LIB_PATH=-L/home/anabur/Github/libraries/cuda/cuda-10.0/lib64
INCLUDES=-I/home/anabur/Github/libraries/arrayfire/include -I/home/anabur/Github/include
CC=g++ $(COMPILER_OPTIONS)
COMPILER_OPTIONS=-pthread -std=c++11 -g

all: embedding.cpp
	$(CC) embedding.cpp ../RDF_parser/progress_bar.cpp -o embedding -Wl,-rpath=/home/anabur/Github/libraries/arrayfire/lib64 $(INCLUDES) -s $(LIBS) $(AF_LIB_PATH) $(CU_LIB_PATH) -DARMA_DONT_USE_WRAPPER
