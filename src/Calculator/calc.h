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
