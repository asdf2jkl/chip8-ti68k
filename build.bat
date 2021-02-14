@ECHO OFF
TITLE C8-68k build script (windows)
del /q bin
tigcc src/Calculator/calc.c -DUSE_TI89 -o bin/c8-89
// tigcc -O2 src/Calculator/calc.c -DUSE_TI92P -o bin/c8-92
dir bin
PAUSE
cls