/*
* All the functions other than _main go here (keyboard handling, display, timing [to be handled next])
*/

#define DARK_TRUE 0xFF
#define DARK_FALSE 0x00

#define KEYMODE_LOOP 1
#define KEYMODE_TEST 0
#define KEY_DUMMY 0
#define KEY_F5	17

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
	char out_key[17] = 0;
	char i;
	do {
		BEGIN_KEYTEST
			out_key[KEY_F5] = _keytest_optimized(RR_F5);
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
			out_key[] = _keytest_optimized(RR_DOT);
		END_KEYTEST
		BEGIN_KEYTEST
			out_key[1] = _keytest_optimized(RR_7);
			out_key[4] = _keytest_optimized(RR_4);
			out_key[7] = _keytest_optimized(RR_1);
			out_key[0xA] = _keytest_optimized(RR_0);
		END_KEYTEST
		if (out_key[KEY_F5]) {
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
Drawing to screen
*/
void draw_display(unsigned long *c8_display, void *virtual_display) {
	short x, y;
	for (y = 0; y != 32; y++) {
		for (x = 0; x != 64; x++) {
			if (c8_display[(y % 32) * 2 + (x >= 32)] >> (x % 32) & 0x01) {	//please note that the screen may not start at zero, in which case I'll need to add one to each of the draw commands (or one in the if statement)
				DrawPix(x, y, A_NORMAL);
				DrawPix(x + 1, y, A_NORMAL);
				DrawPix(x, y + 1, A_NORMAL);
				DrawPix(x + 1, y + 1, A_NORMAL);
			} else {
				DrawPix(x, y, A_REVERSE);
				DrawPix(x + 1, y, A_REVERSE);
				DrawPix(x, y + 1, A_REVERSE);
				DrawPix(x + 1, y + 1, A_REVERSE);
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
