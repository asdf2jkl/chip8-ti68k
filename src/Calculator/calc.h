/*
* All the functions other than _main go here (keyboard handling, display, timing [to be handled next])
*/


/*
keyboard handling functions
|1|2|3|C|
|4|5|6|D|
|7|8|9|E|
|A|0|B|F|
*/
#if defined(USE_TI92P)
char getkey(unsigned char in_key, char mode) {
	/*
	deal with this later
	*/
}
#else	//TI-89
char getkey(unsigned char in_key, char mode) {
	/*
		|7|8|9|x|
		|4|5|6|-|
		|1|2|3|+|
		|0|.|-|e|
	*/
	char out_key[17] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	char i;
	do {
		BEGIN_KEYTEST
			out_key[0x10] = _keytest_optimized(RR_F5);
			out_key[0xC] = _keytest_optimized(RR_MULTIPLY);
			out_key[0xD] = _keytest_optimized(RR_MINUS);
			out_key[0xE] = _keytest_optimized(RR_PLUS);
			out_key[0xF] = _keytest_optimized(RR_ENTER);
		END_KEYTEST
		BEGIN_KEYTEST
			out_key[3] = _keytest_optimized(RR_3);
			out_key[6] = _keytest_optimized(RR_6);
			out_key[9] = _keytest_optimized(RR_9);
			out_key[0xB] = _keytest_optimized(RR_NEGATE);
		END_KEYTEST
		BEGIN_KEYTEST
			out_key[2] = _keytest_optimized(RR_8);
			out_key[5] = _keytest_optimized(RR_5);
			out_key[8] = _keytest_optimized(RR_2);
			out_key[0] = _keytest_optimized(RR_DOT);
		END_KEYTEST
		BEGIN_KEYTEST
			out_key[1] = _keytest_optimized(RR_7);
			out_key[4] = _keytest_optimized(RR_4);
			out_key[7] = _keytest_optimized(RR_1);
			out_key[0xA] = _keytest_optimized(RR_0);
		END_KEYTEST
		if (out_key[0x10]) {
			exit_flag = 1;
			return 0;
		}
		if (mode) {
			for (i = 16; i; i--) {
				if (out_key[i])
					return i;
			}
		}
	} while (mode);
	return out_key[in_key];
}
#endif


/*
Removing the last character in input string in order to pass it to FOpen. (TI-OS sanitizes inputs, so you have to enclose rom file paths in quotation marks)
I'm not changing the input, though I really have no idea if it actually matters.
*/
char *unmaskRomArg(char* inputStr) {
	unsigned long strLength = strlen(inputStr) - 2;
	char *safeStr = malloc(strLength);	//I may be able to allocate one less byte, the math is just not working right now
	if (!safeStr);
		return NULL;
	memcpy(safeStr, inputStr + 1, strLength);	//cuts off the first quotation mark, and cuts the last null byte off
	safeStr[strLength - 1] = 0x00;
	return safeStr;
}

/*
Drawing to screen
*/
void draw_display(unsigned long *c8_display, void *virtual_display, char dark_mode) {
	short x, y;
	for (y = 0; y != 32; y++) {
		for (x = 0; x != 64; x++) {
			if (c8_display[(y % 32) * 2 + (x >= 32)] >> (x % 32) & 0x01) {	//please note that the screen may not start at zero, in which case I'll need to add one to each of the draw commands (or one in the if statement)
				DrawPix(x, y, dark_mode ? A_REVERSE : A_NORMAL);
				DrawPix(x + 1, y, dark_mode ? A_REVERSE : A_NORMAL);
				DrawPix(x, y + 1, dark_mode ? A_REVERSE : A_NORMAL);
				DrawPix(x + 1, y + 1, dark_mode ? A_REVERSE : A_NORMAL);
			} else {
				DrawPix(x, y, dark_mode ? A_NORMAL : A_REVERSE);
				DrawPix(x + 1, y, dark_mode ? A_NORMAL : A_REVERSE);
				DrawPix(x, y + 1, dark_mode ? A_NORMAL : A_REVERSE);
				DrawPix(x + 1, y + 1, dark_mode ? A_NORMAL : A_REVERSE);
			}
		}
	}
	LCD_restore(virtual_display);
	return;
}

/*
CALLBACK void interrupt_timer(void) {   //Set PRG so that this runs at 60hz intervals
    timers[0] ? timers[0]++ : timers[0];
    timers[1] ? timers[1]++ : timers[1];
    //display sync and instruction pacing go here later
    return;
}
*/


//Credit to Ben Ingram (creator of gb68k) for these two functions, I really am/was lost
SYM_ENTRY* file_entry(const char* name)
{
	SYM_ENTRY* entry = NULL;

	FolderOp(NULL, FOP_LOCK | FOP_ALL_FOLDERS);
	entry = SymFindFirst(NULL, FO_RECURSE | FO_SKIP_TEMPS);
	while (entry) {
		if (!entry->flags.bits.folder && strncmp(entry->name, name, 8) == 0) break;
		else entry = SymFindNext();
	}
	FolderOp(NULL, FOP_UNLOCK | FOP_ALL_FOLDERS);
	return entry;
}

void* file_pointer(const char* name)
{
	SYM_ENTRY* entry = file_entry(name);
	if (entry == NULL) return NULL;
	else return HeapDeref(entry->handle);
}
