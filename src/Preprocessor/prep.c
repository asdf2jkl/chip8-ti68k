//Chip-8 rom preprocessor v0.0.1-alpha 2021-03-14

#include <stdio.h>
#include <stdlib.h>
#include "bin2oth.h"

int main(int argc, char *argv[]) {
    unsigned int romSize;
    unsigned char *calc_file;
    void *ttOut;
    FILE *output;
    if (argc == 1) {
        fprintf(stderr, "ERROR: input file required\n");
        return 0;
    }
    FILE* c8rom = fopen(argv[1], "rb");
	//FILE* c8rom = fopen("../roms/ibm.ch8", "rb");
	fseek(c8rom, 0, SEEK_END);
	romSize = ftell(c8rom);
	rewind(c8rom);
	calc_file = malloc(romSize);
    if (!calc_file) {
        fprintf(stderr, "ERROR: cannot allocate %u bytes of memory\n", romSize);
        return 0;
    }
	fread(calc_file, romSize, 1, c8rom);
	fclose(c8rom);

	/*
	* TODO: store size of file in array, then call bin2oth
	*/
    ttOut = DataBuffer2OTHBuffer(CALC_TI89, "acme", "ch8test", "c8rm", romSize, calc_file, &romSize);

    output = fopen("ch8test.89y", "wb");
    if (fwrite(ttOut, 1, romSize, output) != romSize){
        fprintf(stderr, "ERROR: failure writing file to disk\naborting");
        }
    fclose(output);
	free(calc_file);
	free(ttOut);
	return 0;
}
