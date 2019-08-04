/*****************************************************************************
 * PETSeq.        						                                     *
 *                                                                           *
 * (c)2015 by Stewart Waller                                                 *
 *                                                                           *
 *  v1.1                                                                     *
 *                                                                           *
 *****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <peekpoke.h> 
#include <time.h>
#include <pet.h>

/*
 * Sound generation shamelessly *borrowed* from
 * PETSynth - by Chiron Bramberger  www.petsynth.org
 *
 * 'r': randomize notes
 * 'RUN/STOP': quit
 * 'space': start/stop		
 * 'm': mode (loop/bounce)
 * 'z': decrease bpm
 * 'x': increase bpm
 * 'c': decrease bpm x 10
 * 'v': increase bpm x 10
 * 'e': edit mode
 * 'n': step forward
 * 'b': step back
 */

#define RUN_STOP 3
#define MAX_BPM 360

/* Use static local variables for speed */
#pragma static-locals (1); 
	
// global vars -----------------

int *notes;
int bpm 		= 0;
int step 		= 0;
int last_step 	= 0;
int length 		= 0;
int mode 		= 0;
int state 		= 0;
int quit 		= 0;
int dir 		= 0;
unsigned long sec, dur, tim = 0;

static void drawSplash() {
	
	//Graphics Mode (lines touching)
	putchar(142); 
	
	puts ("                                       ");
	puts ("      ###           ##                 ");
	puts ("      #  #         #  #                ");
	puts ("      #  # ### ### #    ### ####       ");
	puts ("      ###  #    #   #   #   #  #       ");
	puts ("      #    ##   #    #  ##  #  #       ");
	puts ("      #    #    #     # #   # ##       ");
	puts ("      #    ###  #  #  # ### ####       ");
	puts ("      #             ##          #      ");
	
	puts ("                                   v1.1");
	
	gotoxy(0,22);
	
	puts ("  ");
	puts ("PETSeq ........ (c) 2015 Stewart Waller");
 	
	POKE(0xE84C, 0xE);	
}

static void drawScreen() {
	int c,r;

	//Graphics Mode (lines touching)
	putchar(142); 	
	
	clrscr ();
	
	printf("   %c", 240);
	for(c=0;c<7;c++) printf("%c%c%c%c",192,192,192,242);
	printf("%c%c%c%c    ",192,192,192,238);	
	printf("   %c",221);
	for(c=0;c<7;c++) printf(" %i %c",(c+1),221);
	printf(" 8 %c    ",221);
	for(r=0; r<2; r++) {
		printf("   %c",235);
		for(c=0;c<7;c++) printf("%c%c%c%c",192,192,192,219);
		printf("%c%c%c%c    ",192,192,192,243);	
		printf("   %c",221);
		for(c=0;c<7;c++) printf("   %c",221);
		printf("   %c    ",221);
	}
	printf("   %c", 237);
	for(c=0;c<7;c++) printf("%c%c%c%c",192,192,192,241);
	printf("%c%c%c%c    ",192,192,192,253);	
	puts ("");
	printf("   %c", 240);
	for(c=0;c<7;c++) printf("%c%c%c%c",192,192,192,242);
	printf("%c%c%c%c    ",192,192,192,238);
	printf("   %c",221);
	for(c=0;c<7;c++) {	
		if(c+9<10) printf(" ");
		printf(" %i%c",(c+9),221);
	}
	printf(" 16%c    ",221);
	for(r=0; r<2; r++) {
		printf("   %c",235);
		for(c=0;c<7;c++) printf("%c%c%c%c",192,192,192,219);
		printf("%c%c%c%c    ",192,192,192,243);	
		printf("   %c",221);
		for(c=0;c<7;c++) printf("   %c",221);
		printf("   %c    ",221);
	}
	printf("   %c", 237);
	for(c=0;c<7;c++) printf("%c%c%c%c",192,192,192,241);
	printf("%c%c%c%c    ",192,192,192,253);	
	puts ("");
	puts ("  Mode:         Step:      Speed: ");
	puts ("");
	puts ("  State:");
	puts ("");
	puts ("");
	puts ("");
    puts ("");
	puts ("PETSeq ........ (c) 2015 Stewart Waller");
}

static void updateScreen() {
	//lower/uppercase mode (not space change) - POKE 59468,14
	POKE(0xE84C, 0xE);
	
	// Draw mode
	gotoxy(8,16);
	switch(mode) {
		case 0 : 
			printf("LOOP  ");
			break;
		case 1 : 
			printf("BOUNCE");
			break;
	}
	
	// Draw step
	gotox(22);
	printf("%i ", step + 1);
	
	// Draw BPM
	gotox(34);
	printf("%i ", bpm);
	
	// Draw state
	gotoxy(9,18);
	switch(state) {
		case 0 : 
			printf("STOPPED");
			break;
		case 1 : 
			printf("RUNNING");
			break;
		case 2 : 
			printf("EDITING");
			break;
	}
}

static void updateStep() {	
	sec = (clock() - 0 / CLOCKS_PER_SEC) * 10;
	tim = (600/bpm)*100;
   
	if(sec - dur > (tim / 4)) {
		dur = sec;
		
		if(mode == 0) {
          step++;
          if(step > 15) step = 0;
		} else {
          if (dir == 0) {
		     step++;
			 if(step > 15) { 
			   step = 14;
			   dir = 1;
			 }
		  } else {
		     step--;
			 if(step < 0) { 
			   step = 1;
			   dir = 0;
			 }
		  }
		}
	}
}

static void drawStep() {
	int i;
	
	for(i=0;i<8;i++) {
		if(step == i) {
			POKE( 32893 + (4 * i), 102 ) ;
		} else {
			POKE( 32893 + (4 * i), 32 ) ;			
		}
	}
	
	for(i=0;i<8;i++) {
		if(step == 8+i) {
			POKE( 32893 + 320 + (4 * i), 102 ) ;
		} else {
			POKE( 32893 + 320 + (4 * i), 32 ) ;			
		}
	}
}

static void drawNotes() {
	int i;
	
	for(i=0;i<8;i++) {	
		gotoxy(4 + (i*4),5);
		if(notes[i] < 10) {
			printf(" ");
		}
		if(notes[i] < 100) {
			printf(" ");
		}
		printf("%i",notes[i],221);
	}

	for(i=0;i<8;i++) {	
		gotoxy(4 + (i*4),13);
		if(notes[i+8] < 10) {
			printf(" ");
		}
		if(notes[i+8] < 100) {
			printf(" ");
		}
		printf("%i",notes[i+8],221);
	}		
}

static void randomizeNotes() {
	int i;
	
	for(i = 0; i < 16; i++) {
		notes[i] = rand() % 127;
	}
	
	drawNotes();
}

static void stopSound() {
	POKE( 0xE848, 0 ); // freq off				 
}

static void playSound() {
	POKE( 0xE848, notes[step]);	
}

static void doKeys() {
	if (kbhit()) {
	
		char key_hit = cgetc();	
		
		// Steps
		if (key_hit == 'q') {
			
		} else if (key_hit == 'e') { // editing
			state = 2;
		} else if (key_hit == 'r') { // randomize notes
			randomizeNotes();
		} else if (key_hit == RUN_STOP) { // run stop
			quit = 1;
		} else if (key_hit == ' ') { // start/stop		
			if(state == 0) {
				playSound();
				state = 1;
			} else {
				stopSound();
				state = 0;
			}
		} else if (key_hit == 'm') { // cycle mode
			mode++;
			if(mode > 1) mode = 0;
		} else if (key_hit == 'z') { // decrease bpm
			bpm--;
			if(bpm < 1) bpm = 1;
		} else if (key_hit == 'x') { // increase bpm
			bpm++;
			if(bpm > MAX_BPM) bpm = MAX_BPM;
		} else if (key_hit == 'c') { // decrease bpm x 10
			bpm-=10;
			if(bpm < 1) bpm = 1;
		} else if (key_hit == 'v') { // increase bpm x 10
			bpm+=10;
			if(bpm > MAX_BPM) bpm = MAX_BPM;
		} else if (key_hit == 'n' && state == 0) { // step forward
			step++;
			if(step > 15) step = 0;
			drawStep();
			length = 0;
			playSound();
		} else if (key_hit == 'b' && state == 0) { // step back
			step--;
			if(step < 0) step = 15;
			drawStep();
			length = 0;
			playSound();
		}
	}
}

static void editMode() {
	char key_hit;
	char edit_mode = 0;
	char edit_done = 0;
	char es[3] = {};
	char *edit_string = es;
	char edit_pos = 0;
	int edit_value = 0;
	int i = 0;
	int s = 0;
	
	stopSound();
	
	// position cursor at step
	if(step < 8) {
		gotoxy(7 + (step*4),5);
	} else if(step < 16) {
		gotoxy(7 + ((step-8)*4),13);
	}

	// turn on cursor
	cursor(1);
	
	// begin editing
	while (!edit_done) { 
		key_hit = cgetc();
	
		if (key_hit == 'b') {
			step -= 1;
			if(step < 0) step = 15;
			edit_pos = 0;
			drawStep();	
		//	length=0;
		//	playSound();

		} else if (key_hit == 'n') {
			
			step += 1;
			if(step > 15) step = 0;
			edit_pos = 0;
			drawStep();
		
		//	length=0;
		//	playSound();
			
		} else if (key_hit == 't') {
			edit_pos = 0;
			if(!s) {
				s=1;
				playSound();
			} else {
				s=0;
				stopSound();
			}
		} else if (key_hit == ' ') {
			edit_done = 1;
			stopSound();
		} else if(key_hit >= 48 && key_hit <= 57 && edit_pos < 3) {
			if(edit_pos == 0) {
				notes[step] = 0;
				edit_string[0] = ' ';
				edit_string[1] = ' ';
				edit_string[2] = ' ';
			}
			edit_string[edit_pos] = key_hit;
			notes[step] = atoi(edit_string);
			if(notes[step] > 128) notes[step] = 127;
			if(notes[step] < 0) notes[step] = 0;
			edit_pos++;
			
			if(edit_pos > 2) {
				edit_pos = 0;
			}
		}
		
		// Update notes
		for(i=0;i<8;i++) {
			if(step == i) {
				gotoxy(4 + (i*4),5);
				if(notes[i] < 10) {
					printf(" ");
				}
				if(notes[i] < 100) {
					printf(" ");
				}
				printf("%i",notes[i],221);
			}
		}

		for(i=0;i<8;i++) {	
			if(step == i+8) {
				gotoxy(4 + (i*4),13);
				if(notes[i+8] < 10) {
					printf(" ");
				}
				if(notes[i+8] < 100) {
					printf(" ");
				}
				printf("%i",notes[i+8],221);
			}
		}
		
		//if(++length > 10) {
		//	stopSound();
		//}
	}

	state = 0;
	cursor(0);
}

static void setup() {

	// init notes
	int n[16] 	= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	notes 		= n;
	bpm 		= 120;

	// seed rand with clock
	srand(clock());

	// init sound
	POKE( 0xE84B, 0x10 ); // sound on
	POKE( 0xE84A, 0x15 ); // octave
	POKE( 0xE848, 0x0 );  // frequency
		
	drawScreen();
	
	drawNotes();
	
	drawStep();
}

int main (void)
{	
	int splash_time, i = 0;
	
	clrscr();
	
	drawSplash();

	for(i=0;i<5;i++) {
		splash_time = 10000;
		while (splash_time) { 
			splash_time--;
		}
	}
	
	clrscr();
	
	setup();
	
	// main loop
	for(;;) {
		
		doKeys();
			
		if(state == 1) {
			updateStep();		
			drawStep();			
			if(last_step != step) {
				length = 0;
				playSound();
			} else if(++length > 10) {
				stopSound();
			}
			last_step = step;
		} else if(state == 0) {
			if(++length > 10) {
				stopSound();
			}
		}
	
		updateScreen();

		if(state == 2) {
			editMode();
		}
		
		if(quit) {
			bpm			= 0;
			step 		= 0;
			last_step 	= 0;
			length 		= 0;
			mode 		= 0;
			state 		= 0;
			quit 		= 0;
			stopSound();
			POKE( 0xE84B, 0x0 );
			clrscr();
			gotoxy(0,24);
			puts("Exiting PETSeq ...   ");
			return EXIT_SUCCESS;		
		}
	}
	
	putchar(14);
	POKE( 0xE84B, 0x0 );
	
	return EXIT_SUCCESS;
}


