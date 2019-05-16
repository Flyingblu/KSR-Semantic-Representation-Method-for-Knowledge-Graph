rdf_parser: main.cpp rdf_parser.cpp map_serializer.cpp progress_bar.cpp
	g++ -o rdf_parser main.cpp rdf_parser.cpp map_serializer.cpp progress_bar.cpp -I . -std=c++11
