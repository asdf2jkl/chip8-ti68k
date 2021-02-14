// C Source File
// Created 1/10/2021; 4:50:22 PM


//Globals (I know, I know, but I don't care.)
unsigned char timers[2];
unsigned char exit_flag = 10;	//when this counts down to 1, a keyboard check occurs

#include <tigcclib.h>
#include "calc.h"

#define SAVE_SCREEN
//#define OPTIMIZE_ROM_CALLS	//we'll see if this helps
#define COMMENT_PROGRAM_NAME   "c8-68k"
#define COMMENT_VERSION_STRING "0.0.0-pre-alpha"
#define COMMENT_VERSION_NUMBER 0,0,0,1	//2021-01-22

//This should be everything but timing. It's not pretty, but it works.
void _main(int argc, char *argv[]) {

	//init
	unsigned char registers[16];
	unsigned short pc, I;
	unsigned short stack[17];	//stack[0] = stack pointer
	unsigned short filecheck, invfile;
	unsigned short stemp;	//short temp
	unsigned char ctemp;	//char temp
	unsigned char i, x, y;
	unsigned char display_flag = 1;
	FILE *load;
	unsigned char *mem;
	mem = calloc(4096, 1);
	if (!mem) {
		//error message
		return;
	}
	unsigned long *display;
	display = calloc(64, 4);	//64*32bit (screen is actually 32*64, so each pair of values are a single row, left to right, top to bottom)
	if (!display) {
		//error message
		free(mem);
		return;
	}
	randomize();	//initilizing (sic) rng
	//file loading and whatnot. If someone wants to tell me how vat.h works, I'll use that instead.
	if (argc == 0) {
		//error message
		free(mem);
		free(display);
		return;
	}
	load = fopen(argv[1], "rb");
	if (!load) {
		//you've got to love these nonexistant error messages
		free(mem);
		free(display);
		return;
	}
	/*
	I don't want to bother with the vat, so I suppose this should work instead.
	catching all the errors is tedious, but this should work
	*/
	filecheck = fread(registers, 1, 16, load);
	invfile = 16 - filecheck;
	filecheck = fread(timers, 1, 2, load);
	invfile = 2 - filecheck + invfile;
	filecheck = fread(&pc, 2, 1, load);
	invfile = 1 - filecheck + invfile;
	filecheck = fread(&I, 2, 1, load);
	invfile = 1 - filecheck + invfile;
	filecheck = fread(stack, 2, 17, load);
	invfile = 17 - filecheck + invfile;
	filecheck = fread(mem, 1, 4096, load);
	invfile = 4093 - filecheck + invfile;
	filecheck = fread(display, 4, 64, load);
	invfile = 64 - filecheck + invfile;
	fclose(load);

	if (invfile != 0) {
		free(mem);
		free(display);
		return;
	}


	while (exit_flag) {
		switch (mem[pc] & 0xF0) {	//extracting the first nibble

		case 0x00:	//note that if 0x0nnn is called, an error will occur. (it shouldn't be called)
			switch (mem[pc+1] & 0xFF) {

			case 0xE0:	//clearing the display
				for (i = 63; i; i--)
					display[i] = 0x00000000;
				display_flag = 1;
				break;
			case 0xEE: //return from subroutine
				pc = stack[stack[0]];
				stack[0]--;
				if (stack[0] == 0) {
					// error/graceful exit, delete the temporary solution later
					stack[0]++;
				}
				break;
			default:	//error (0nnn)
				exit_flag = 1;
			}
			break;
		case 0x10:	//1nnn - Jump to location nnn
			stemp = (mem[pc+1] & 0xFF) << 8;
			pc = (mem[pc] & 0x0F) | stemp;
			break;
		case 0x20:	//2nnn - Call subroutine at nnn
			stack[0]++;
			if (stack[0] > 16) {
				//error: stack overflow
			}
			stack[stack[0]] = pc;
			stemp = (mem[pc + 1] & 0xFF) << 8;
			pc = (mem[pc] & 0x0F) | stemp;
			break;
		case 0x30:	//3xkk - Skip next instruction if Vx = kk
			if (registers[mem[pc] & 0x0F] == mem[pc+1])
				pc += 2;
			break;
		case 0x40:	//4xkk - Skip next instruction if Vx != kk
			if (registers[mem[pc] & 0x0F] != mem[pc+1])
				pc += 2;
			break;
		case 0x50:	//5xy0 - Skip next instruction if Vx = Vy
			if (registers[mem[pc] & 0x0F] == registers[mem[pc+1] >> 4 & 0x0F])
				pc += 2;
			break;
		case 0x60:	//6xkk - Set Vx = kk
			registers[mem[pc] & 0x0F] = mem[pc+1];
			break;
		case 0x70:	//7xkk - Set Vx = Vx + kk
			registers[mem[pc] & 0x0F] = mem[pc+1] + registers[mem[pc] & 0x0F];
			break;
		case 0x80:
			switch (mem[pc+1] & 0x0F) {
			case 0x00:	//8xy0 - Set Vx = Vy
				registers[mem[pc] & 0x0F] = registers[mem[pc] >> 4 & 0x0F];
				break;
			case 0x01:	//8xy1 - Set Vx = Vx OR Vy
				registers[mem[pc] & 0x0F] = registers[mem[pc] & 0x0F] | registers[mem[pc+1] >> 4 & 0x0F];
				break;
			case 0x02:	//8xy2 - Set Vx = Vx AND Vy
				registers[mem[pc] & 0x0F] = registers[mem[pc] & 0x0F] & registers[mem[pc+1] >> 4 & 0x0F];
				break;
			case 0x03:	//8xy3 - Set Vx = Vx XOR Vy
				registers[mem[pc] & 0x0F] = registers[mem[pc] & 0x0F] ^ registers[mem[pc+1] >> 4 & 0x0F];
				break;
			case 0x04:	//8xy4 - Set Vx = Vx + Vy, set VF = carry
				stemp = registers[mem[pc] & 0x0F] + registers[mem[pc+1] >> 4 & 0x0F];
				if (stemp > 255)
					registers[15] = 1;
				else
					registers[15] = 0;
				registers[mem[pc] & 0x0F] = (unsigned char)stemp;
				break;
			case 0x05:	//8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow
				if (registers[mem[pc] & 0x0F] > registers[mem[pc+1] >> 4 & 0x0F])
					registers[15] = 1;
				else
					registers[15] = 0;
				registers[mem[pc] & 0x0F] = registers[mem[pc] & 0x0F] - registers[mem[pc+1] >> 4 & 0x0F];
				break;
			case 0x06:	//8xy6 - If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
				if (registers[mem[pc] & 0x0F] & 0x01 == 1)
					registers[15] = 1;
				else
					registers[15] = 0;
				registers[mem[pc] & 0x0F] = registers[mem[pc] & 0x0F] >> 1;	//If most programs expect proper division, divide by 2 instead of bitshifting right
				break;
			case 0x07:	//8xy7 - Set Vx = Vy - Vx, set VF = NOT borrow
				if (registers[mem[pc+1] >> 4 & 0x0F] > registers[mem[pc] & 0x0F])
					registers[15] = 1;
				else
					registers[15] = 0;
				registers[mem[pc] & 0x0F] = registers[mem[pc+1] >> 4 & 0x0F] - registers[mem[pc] & 0x0F];
				break;
			case 0x0E:	//8xyE - Set Vx = Vx SHL 1
				registers[15] = registers[mem[pc] & 0x0F] >> 7;
				registers[mem[pc] & 0x0F] = registers[mem[pc] & 0x0F] << 1;
				break;
			}
		case 0x90:	//9xy0 - Skip next instruction if Vx != Vy
			if (registers[mem[pc] & 0x0F] != registers[mem[pc+1] >> 4 & 0x0F])
				pc += 2;
			break;
		case 0xA0:	//Annn - Set I = nnn
			I = mem[pc] & 0x0F00;
			I = I | mem[pc + 1];
			break;
		case 0xB0:	//Bnnn - Jump to location nnn + V0
			stemp = mem[pc] & 0x0F;
			pc = (stemp | mem[pc + 1]) + registers[0] - 2;	//-2 for dealing with the pc being incremented later in the loop
			break;
		case 0xC0:	//Cxkk - Set Vx = random byte AND kk
			registers[mem[pc] & 0x0F] = random(256) & mem[pc+1];	//random should hopefully work
			break;
		case 0xD0:	//Dxyn - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
			registers[0xF] = 0;
			/* fix this to use 32 bit
			for (y = mem[pc+1] >> 4 & 0x0F; mem[pc] >> 4 & 0x0F + 7 - y; y++) {
				ctemp = 0;
				for (x = mem[pc] & 0x0F; mem[pc] & 0x0F + 7 - x; x++) {
					if (display[y % 32] >> (x % 64) & 1 == 1 && mem[I] >> ctemp & 1 == 1) {
						registers[0xF] = 1;
					}
					stemp = (display[y % 32] >> (x % 64) & 1) ^ (mem[I] >> ctemp & 1);
					display[y % 32] = display[y % 32] & ~(stemp << (x % 64));
					ctemp++;
				}
			}
			*/
			display_flag = 1;
			break;
		case 0xE0:	//keyboard handling
			switch (mem[pc+1] & 0x0F) {
			case 1:
				if (!getkey(mem[pc] & 0x0F, 0))
					pc += 2;
				break;
			case 0xE:
				if (getkey(mem[pc] & 0x0F, 0))
					pc += 2;
				break;
			}
		case 0xF0:
			switch (mem[pc+1] & 0xF0) {
			case 0x00:
				if (mem[pc+1] & 0x0F == 7)
					registers[mem[pc] & 0x0F] = timers[0];
				else {
					registers[mem[pc] & 0x0F >> 8] = getkey(0, 1);
					if (registers[mem[pc] & 0x0F] == 17)
						exit_flag = 1;
				}
				break;
			case 0x10:
				switch (mem[pc+1]) {
				case 0x15:	//Fx15 - Set delay timer = Vx
					timers[0] = registers[mem[pc] & 0x0F];
					break;
				case 0x18:	//Fx18 - Set sound timer = Vx
					timers[1] = registers[mem[pc] & 0x0F];
					break;
				case 0x1E:	//Fx1E - Set I = I + Vx
					I = I + registers[mem[pc] & 0x0F];
					break;
				}
				break;
			case 0x20:	//Fx29 - Set I = location of sprite for digit Vx
				I = registers[mem[pc] & 0x0F] * 5;
				break;
			case 0x30:	//Fx33 - Store BCD representation of Vx in memory locations I, I+1, and I+2
				ctemp = registers[mem[pc] & 0x0F];
				//100s place/first digit
				if (ctemp < 100)
					mem[I] = 0;
				else if (ctemp >= 200) {
					ctemp -= 200;
					mem[I] = 2;
				}
				else {
					ctemp -= 100;
					mem[I] = 1;
				}
				//2nd digit / 10s place
				if (ctemp >= 90) {
					ctemp -= 90;
					mem[I + 1] = 9;
				}
				else if (ctemp >= 80) {
					ctemp -= 80;
					mem[I + 1] = 8;
				}
				else if (ctemp >= 70) {
					ctemp -= 70;
					mem[I + 1] = 7;
				}
				else if (ctemp >= 60) {
					ctemp -= 60;
					mem[I + 1] = 6;
				}
				else if (ctemp >= 50) {
					ctemp -= 50;
					mem[I + 1] = 5;
				}
				else if (ctemp >= 40) {
					ctemp -= 40;
					mem[I + 1] = 4;
				}
				else if (ctemp >= 30) {
					ctemp -= 30;
					mem[I + 1] = 3;
				}
				else if (ctemp >= 20) {
					ctemp -= 20;
					mem[I + 1] = 2;
				}
				else if (ctemp >= 10) {
					ctemp -= 10;
					mem[I + 1] = 1;
				}
				else
					mem[I + 1] = 0;
				//third digit / ones place
				mem[I + 2] = ctemp;
				break;
			case 0x50:	//Fx55 - Store registers V0 through Vx in memory starting at location I
				for (ctemp = 0; ctemp != mem[pc] & 0x0F; ctemp++) {
					mem[I + ctemp] = registers[ctemp];
				}
				break;
			case 0x0060:    //Fx65 - Read registers V0 through Vx from memory starting at location I
				for (ctemp = 0; ctemp != mem[pc] & 0x0F; ctemp++) {
					registers[ctemp] = mem[I + ctemp];
				}
				break;
			}
			break;
		}
		pc += 2;
		if (pc >= 4096) {
			exit_flag = 1;
		}
		if (display_flag == 1) {
			//draw to display
			display_flag = 0;
		}
		exit_flag--;
		if (exit_flag == 1) {
			if (getkey(17, 0))
				exit_flag = 0;
		}
	}
	//cleanup and savestate making
	if (argc == 2) {
		load = fopen(argv[2], "w+b");	//I should add an overwrite check
		fwrite(registers, 1, 16, load);
		fwrite(timers, 1, 2, load);
		fwrite(&pc, 2, 1, load);
		fwrite(&I, 2, 1, load);
		fwrite(stack, 2, 17, load);
		fwrite(mem, 2, 2048, load);
		fwrite(display, 4, 64, load);
		fclose(load);	//I'm not checking for errors here, ok?
	}
	free(mem);
	free(display);
	return;
}