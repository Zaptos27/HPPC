#!/bin/bash

nvc++ -I../include -O3 -fast  -Wall -march=native -g -std=c++14 -acc -gpu=cuda11.6 -Minfo=acc sw_parallel.cpp -o sw_parallel
./sw_parallel