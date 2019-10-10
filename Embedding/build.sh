#!/bin/bash

g++ -pthread -o embedding embedding.cpp ../RDF_parser/progress_bar.cpp --std=c++11 -O2 -I /home/anabur/Github/include -DARMA_DONT_USE_WRAPPER -llapack -lblas
