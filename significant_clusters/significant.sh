#!/bin/bash

g++ -o significant $1 RDF_parser/progress_bar.cpp -pthread --std=c++11
