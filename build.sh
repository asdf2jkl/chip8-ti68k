#!/bin/sh
rm -r bin
mkdir bin
tigcc src/Calculator/calc.c -DUSE_TI89 -o bin/c889
#tigcc -Os src/Calculator/calc.c -DUSE_TI89 -o bin/c8-89-Os
#tigcc -O2 src/Calculator/calc.c -DUSE_TI89 -o bin/c8-89-O2
#tigcc -O3 src/Calculator/calc.c -DUSE_TI89 -o bin/c8-89-O3
tigcc src/Calculator/calc.c -DUSE_TI92P -o bin/c868k
#tigcc -Os src/Calculator/calc.c -DUSE_TI92P -o bin/c8-92-Os
#tigcc -O2 src/Calculator/calc.c -DUSE_TI92P -o bin/c8-92-O2
#tigcc -O3 src/Calculator/calc.c -DUSE_TI92P -o bin/c8-92-O3

ls bin
#make better later
