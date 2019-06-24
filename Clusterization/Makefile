all: clusterization retrieve_size

clusterization: main.cpp clusterization.cpp
	g++ -o clusterization main.cpp clusterization.cpp ../RDF_parser/progress_bar.cpp -I . -pthread -std=c++11

retrieve_size: retrieve_size.cpp
	g++ -o retrieve_size retrieve_size.cpp -I . -std=c++11
