/* Main routine for chip8-ti68k interpreter
* Created 1/10/2021; 4:50:22 PM
* By Peter J. Lafreniere (Saladin, asdf2jkl)
*
* WARNING: THERE IS NO PROTECTION AGAINST SEGFAULTS/RAM CORRUPTION TRIGGERED BY BADLY FORMED CHIP-8 ROMS, EXECUTE AT YOUR OWN RISK
*/

//Globals (I know, I know, but I don't care.)
volatile unsigned char timers[2];
unsigned char exit_flag;	//when this counts down to 1, a keyboard check occurs


//#define SAVE_SCREEN
//#define OPTIMIZE_ROM_CALLS	//we'll see if this helps
#ifdef USE_TI89
#define COMMENT_PROGRAM_NAME   "c889"
#else
#define COMMENT_PROGRAM_NAME   "c868k"
#endif
#define COMMENT_VERSION_STRING "0.0.1-alpha"
#define COMMENT_VERSION_NUMBER 0,0,1,1	//2021-04-02

#include <tigcclib.h>
#include "calc.h"

//#define PRINT_DEBUG

//This should be everything but timing. It's not pretty, but it works.
void _main() {

    const unsigned char numbers[80] = {0xF0,0x90,0x90,0x90,0xF0,
                      			 0x20,0x60,0x20,0x20,0x70,
                        	   	 0xF0,0x10,0xF0,0x80,0xF0,
                       			 0xF0,0x10,0xF0,0x10,0xF0,
                	        	 0x90,0x90,0xF0,0x10,0x10,
        	                	 0xF0,0x80,0xF0,0x10,0xF0,
	                        	 0xF0,0x80,0xF0,0x90,0xF0,
                        		 0xF0,0x10,0x20,0x40,0x40,
                        		 0xF0,0x90,0xF0,0x90,0xF0,
                        		 0xF0,0x90,0xF0,0x10,0xF0,
                        		 0xF0,0x90,0xF0,0x90,0x90,
                        		 0xE0,0x90,0xE0,0x90,0xE0,
                        		 0xF0,0x80,0x80,0x80,0xF0,
                        		 0xE0,0x90,0x90,0x90,0xE0,
                        		 0xF0,0x80,0xF0,0x80,0xF0,
                        		 0xF0,0x80,0xF0,0x80,0x80};
	//init

	#ifdef PRINT_DEBUG
	unsigned char* dbg_saveScr = malloc(LCD_SIZE);
	if(!dbg_saveScr)
		return;
	#endif

	exit_flag = 20;
	unsigned char registers[16] = {0};
	unsigned short pc, I = 0;
	pc = 0x200;
	struct {
        unsigned short main[16];
        char pointer;
        } stack;
    stack.main[0] = 0;
    stack.pointer = 0;
	unsigned long ltemp;
	unsigned short stemp;	//short temp
	unsigned char ctemp;	//char temp
	unsigned char i, x, y;
	//I'll replace these flags with a bitfield later, but only if there are more of them
	unsigned char display_flag = 1;
	short dark_mode = DARK_FALSE;	//this should be user set later
	short *rom;
	unsigned char *mem = calloc(4096, 1);	//I don't know why vscode is flagging this, it compiles just fine, and if I fix it, then tigcc yells at me
	if (!mem) {
		//error message
		return;
	} 
	unsigned long *display = malloc(256);	//64*32bit (screen is actually 32*64, so each pair of values are a single row, left to right, top to bottom)
	if (!display) {
		//error message
		free(mem);
		return;
	} else memset(display, dark_mode, 256);
	randomize();	//initilizing (sic) rng
	//setting up the virtual display buffer for clean drawing
	void *virtual_display = malloc(LCD_SIZE);
	//SAVE_SCREEN doesn't seem to be working for me, so I'll just save the screen normally
	void* savedScreen = malloc(LCD_SIZE);
	if (!virtual_display && !savedScreen) {
		//error
		free(mem);
		free(display);
		return;
	}
	LCD_save(savedScreen);

	PortSet(virtual_display, 239, 127);
	memset(virtual_display, dark_mode, LCD_SIZE);	//now I actually don't know if memset needs one or two byte inputs, even though it accepts a short

	rom = file_pointer("ch8test") + 2;
	if (!rom) {
        free(mem);
        free(display);
        free(virtual_display);
		free(savedScreen);
        return;
    }
	
	memcpy(mem + 0x200, rom, rom[-1]);	//rom[-1] should contain file size

    memcpy(mem, numbers, 80);

	/*
	if (IsPRGEnabled());
        EnablePRG();
    char save_prg_rate = (char)PRG_getRate();
    unsigned char save_prg_start = PRG_getStart();
    OSVRegisterTimer(1, 1, interrupt_timer);
	*/
	while (exit_flag) {
		switch (mem[pc] & 0xF0) {	//extracting the first nibble

		case 0x00:	//note that if 0x0nnn is called, an error will occur. (it shouldn't be called)
			switch (mem[pc+1]) {

			case 0xE0:	//clearing the display
				memset(display, dark_mode, 256);
				//display_flag = 1;
				draw_display(display, virtual_display);

				#ifdef PRINT_DEBUG
				ngetchx();
				PortRestore();
				LCD_save(dbg_saveScr);
				clrscr();
				printf("%X-%X (00E0) clear display\npc: %X\nI: %X\nregisters: ", mem[pc], mem[pc + 1], pc, I);
				i = 0;
				while (i != 16)
				printf("%X ", registers[i++]);
				ngetchx();
				LCD_restore(dbg_saveScr);
				PortSet(virtual_display, 239, 127);
				#endif

				break;
			case 0xEE: //return from subroutine
				pc = stack.main[stack.pointer];
				stack.pointer--;
				if (stack.pointer < 0) {
					// error/graceful exit, delete the temporary solution later
					stack.pointer++;
				}
				break;
			default:	//error (0nnn)
				exit_flag = 1;
			}
			break;
		case 0x10:	//1nnn - Jump to location nnn
			stemp = mem[pc+1] << 8;
			pc = (second_nibble) | stemp;
			break;
		case 0x20:	//2nnn - Call subroutine at nnn
			stack.pointer++;
			if (stack.pointer > 16) {
				//error: stack overflow
				stack.pointer--;
			}
			stack.main[stack.pointer] = pc;
			stemp = (mem[pc + 1] & 0xFF) << 8;
			pc = (second_nibble) | stemp;
			break;
		case 0x30:	//3xkk - Skip next instruction if Vx = kk
			if (registers[second_nibble] == mem[pc+1])
				pc += 2;
			break;
		case 0x40:	//4xkk - Skip next instruction if Vx != kk
			if (registers[second_nibble] != mem[pc+1])
				pc += 2;
			break;
		case 0x50:	//5xy0 - Skip next instruction if Vx = Vy
			if (registers[second_nibble] == registers[third_nibble])
				pc += 2;
			break;
		case 0x60:	//6xkk - Set Vx = kk
			registers[second_nibble] = mem[pc+1];

				#ifdef PRINT_DEBUG
				ngetchx();
				PortRestore();
				LCD_save(dbg_saveScr);
				clrscr();
				printf("%X-%X (6xkk) Set Vx = kk\npc: %X\nI: %X\nregisters: ", mem[pc], mem[pc + 1], pc, I);
				i = 0;
				while (i != 16)
				printf("%X ", registers[i++]);
				ngetchx();
				LCD_restore(dbg_saveScr);
				PortSet(virtual_display, 239, 127);
				#endif

			break;
		case 0x70:	//7xkk - Set Vx = Vx + kk
			registers[second_nibble] = mem[pc+1] + registers[second_nibble];

				#ifdef PRINT_DEBUG
				ngetchx();
				PortRestore();
				LCD_save(dbg_saveScr);
				clrscr();
				printf("%X-%X (7xkk) Set Vx = Vx + kk\npc: %X\nI: %X\nregisters: ", mem[pc], mem[pc + 1], pc, I);
				i = 0;
				while (i != 16)
				printf("%X ", registers[i++]);
				ngetchx();
				LCD_restore(dbg_saveScr);
				PortSet(virtual_display, 239, 127);
				#endif

			break;
		case 0x80:
			switch (mem[pc+1] & 0x0F) {
			case 0x00:	//8xy0 - Set Vx = Vy
				registers[second_nibble] = registers[mem[pc] >> 4 & 0x0F];
				break;
			case 0x01:	//8xy1 - Set Vx = Vx OR Vy
				registers[second_nibble] = registers[second_nibble] | registers[third_nibble];
				break;
			case 0x02:	//8xy2 - Set Vx = Vx AND Vy
				registers[second_nibble] = registers[second_nibble] & registers[third_nibble];
				break;
			case 0x03:	//8xy3 - Set Vx = Vx XOR Vy
				registers[second_nibble] = registers[second_nibble] ^ registers[third_nibble];
				break;
			case 0x04:	//8xy4 - Set Vx = Vx + Vy, set VF = carry
				stemp = registers[second_nibble] + registers[third_nibble];
				if (stemp > 255)
					registers[15] = 1;
				else
					registers[15] = 0;
				registers[second_nibble] = (unsigned char)stemp;
				break;
			case 0x05:	//8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow
				if (registers[second_nibble] > registers[third_nibble])
					registers[15] = 1;
				else
					registers[15] = 0;
				registers[second_nibble] = registers[second_nibble] - registers[third_nibble];
				break;
			case 0x06:	//8xy6 - If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
				if (registers[second_nibble] & 0x01)
					registers[0xF] = 1;
				else
					registers[0xF] = 0;
				registers[second_nibble] >>=  1;	//If most programs expect proper division, divide by 2 instead of bitshifting right
				break;
			case 0x07:	//8xy7 - Set Vx = Vy - Vx, set VF = NOT borrow
				if (registers[third_nibble] > registers[second_nibble])
					registers[15] = 1;
				else
					registers[15] = 0;
				registers[second_nibble] = registers[third_nibble] - registers[second_nibble];
				break;
			case 0x0E:	//8xyE - Set Vx = Vx SHL 1
				registers[15] = registers[second_nibble] >> 7;
				registers[second_nibble] = registers[second_nibble] << 1;
				break;
			}
		case 0x90:	//9xy0 - Skip next instruction if Vx != Vy
			if (registers[second_nibble] != registers[third_nibble])
				pc += 2;
			break;
		case 0xA0:	//Annn - Set I = nnn
			I = (second_nibble) << 8;
			I = I | mem[pc + 1];

				#ifdef PRINT_DEBUG
				ngetchx();
				PortRestore();
				LCD_save(dbg_saveScr);
				clrscr();
				printf("%X-%X (Annn) Set I = nnn\npc: %X\nI: %X\nregisters: ", mem[pc], mem[pc + 1], pc, I);
				i = 0;
				while (i != 16)
				printf("%X ", registers[i++]);
				ngetchx();
				LCD_restore(dbg_saveScr);
				PortSet(virtual_display, 239, 127);
				#endif

			break;
		case 0xB0:	//Bnnn - Jump to location nnn + V0
			stemp = second_nibble;
			pc = (stemp | mem[pc + 1]) + registers[0] - 2;	//-2 for dealing with the pc being incremented later in the loop
			break;
		case 0xC0:	//Cxkk - Set Vx = random byte AND kk
			registers[second_nibble] = random(256) & mem[pc+1];	//random should hopefully work
			break;
		case 0xD0:	//Dxyn - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
			/*
			registers[0xF] = 0;
			stemp = 1;
			for (y = ((mem[pc+1] >> 4) & 0x0F) - 1; stemp != mem[pc+1] & 0x0F; stemp++) {
				y = (y + stemp) % 32;
				ctemp = 7;
				for (x = second_nibble; x != ((second_nibble) + 7) % 64; x++) {
					i = (y % 32) * 2 + (x >= 32);	//i is the proper value in the display array
					if (display[i] >> (x % 32) & 0x00000001 + mem[I + (y % 32)] >> ctemp & 0x01 == 2)
						registers[0xF] = 1;
					display[i] ^= (display[i] & (0x00000001 << (x % 32))) | (mem[I + (y % 32)] & (0x01 << ctemp));	//this is almost guaranteed to have an error in it somewhere
					ctemp--;
				}
			}
			*/

			registers[0xF] = 0;
			stemp = 0;
			for (y = third_nibble; stemp != fourth_nibble - 1; y = (y + 1) % 32) {
				ctemp = 0;
				for (x = second_nibble; ctemp != 8; x = (x + 1) % 64) {
					i = (y << 1) + (x >= 32);
					
					if (((display[i] >> (x % 32)) & 0x01) & ((mem[I + stemp] >> ctemp) & 0x01))
						registers[0xF] = 1;
					display[i] ^= mem[I + stemp] & (0x01 << ctemp);
					ctemp++;
				}
				stemp++;
			}

			//display_flag = 1;
			draw_display(display, virtual_display);

				#ifdef PRINT_DEBUG
				ngetchx();
				PortRestore();
				LCD_save(dbg_saveScr);
				clrscr();
				printf("%X-%X (Dxyn) draw n-byte sprite from I to Vx,Vy\npc: %X\nI: %X\nregisters: ", mem[pc], mem[pc + 1], pc, I);
				i = 0;
				while (i != 16)
				printf("%X ", registers[i++]);
				ngetchx();
				LCD_restore(dbg_saveScr);
				PortSet(virtual_display, 239, 127);
				#endif

			break;
		case 0xE0:	//keyboard handling
			switch (fourth_nibble) {
			case 1:
				if (!getkey(second_nibble, KEYMODE_TEST))
					pc += 2;
				break;
			case 0xE:
				if (getkey(second_nibble, KEYMODE_TEST))
					pc += 2;
				break;
			}
		case 0xF0:
			switch (third_nibble) {
			case 0x00:
				if (fourth_nibble == 7)
					registers[second_nibble] = timers[0];
				else {
					registers[second_nibble >> 8] = getkey(KEY_DUMMY, KEYMODE_LOOP);
					if (registers[second_nibble] == 17)
						exit_flag = 1;
				}
				break;
			case 0x10:
				switch (mem[pc+1]) {
				case 0x15:	//Fx15 - Set delay timer = Vx
					timers[0] = registers[second_nibble];
					break;
				case 0x18:	//Fx18 - Set sound timer = Vx
					timers[1] = registers[second_nibble];
					break;
				case 0x1E:	//Fx1E - Set I = I + Vx
					I = I + registers[second_nibble];
					break;
				}
				break;
			case 0x20:	//Fx29 - Set I = location of sprite for digit Vx
				I = registers[second_nibble] * 5;
				break;
			case 0x30:	//Fx33 - Store BCD representation of Vx in memory locations I, I+1, and I+2
				ctemp = registers[second_nibble];
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
				for (ctemp = 0; ctemp != second_nibble; ctemp++) {
					mem[I + ctemp] = registers[ctemp];
				}
				I += (second_nibble) + 1;
				break;
			case 0x0060:    //Fx65 - Read registers V0 through Vx from memory starting at location I
				for (ctemp = 0; ctemp != second_nibble; ctemp++) {
					registers[ctemp] = mem[I + ctemp];
				}
				I += (second_nibble) + 1;
				break;
			}
			break;
		}
		pc += 2;
		if (pc >= 4096) {
			exit_flag = 1;
		}
		/*
		if (display_flag) {
			draw_display(display, virtual_display);
			display_flag = 0;
		} */
		exit_flag--;
		if (exit_flag == 1) {
			if (getkey(KEY_F5, KEYMODE_TEST))
				exit_flag = 0;
		}
	}
	//OSVFreeTimer(1);

	free(mem);
	free(display);
	PortRestore();
	free(virtual_display);
	LCD_restore(savedScreen);
	free(savedScreen);
	#ifdef PRINT_DEBUG
	free(dbg_saveScr);
	#endif
	return;
}
