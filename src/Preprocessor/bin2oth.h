#define CALC_TI89    0
#define CALC_TI92P   1

unsigned char* DataBuffer2OTHBuffer(int calctype, char* folder, char* varname, char* extension, unsigned int inlength, unsigned char* indata, unsigned int* outlength);
