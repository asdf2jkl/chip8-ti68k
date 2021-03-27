@ECHO ON
TITLE C8-68k build script (windows)
mkdir bin
tigcc src/Calculator/calc.c -DUSE_TI89 -o c889
tigcc -Os src/Calculator/calc.c -DUSE_TI89 -o c889-Os
tigcc -O2 src/Calculator/calc.c -DUSE_TI89 -o c889-O2
tigcc -O3 src/Calculator/calc.c -DUSE_TI89 -o c889-O3
//tigcc src/Calculator/calc.c -DUSE_TI92P -o c892
copy *.89z bin
del *.89z
gcc -Os -s -o c8prep.exe src/Preprocessor/prep.c src/Preprocessor/bin2oth.c
c8prep.exe roms/IBM_Logo.ch8
copy c8prep.exe bin\c8prep_raw.exe
upx c8prep.exe
copy c8prep.exe bin\c8prep.exe
del c8prep.exe
ttbin2oth -89 c8rm roms/IBM_Logo.ch8 ibm main
copy *.89y bin\*.89y
del *.89y
dir bin
rmdir /s bin
cls