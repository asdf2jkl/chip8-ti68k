@ECHO ON
TITLE C8-68k build script (windows)
rmdir /s /q bin
mkdir bin
tigcc src/Calculator/calc.c -DUSE_TI89 -o bin/c8-89
tigcc -Os src/Calculator/calc.c -DUSE_TI89 -o bin/c8-89-Os
tigcc -O2 src/Calculator/calc.c -DUSE_TI89 -o bin/c8-89-O2
tigcc -O3 src/Calculator/calc.c -DUSE_TI89 -o bin/c8-89-O3
//tigcc src/Calculator/calc.c -DUSE_TI92P -o bin/c8-92
dir bin
rmdir /s bin
cls