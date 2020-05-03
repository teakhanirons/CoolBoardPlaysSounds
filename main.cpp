#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h> 
#include <psp2/types.h>
#include <psp2/kernel/clib.h> 
#include <psp2/ctrl.h> 
#include <psp2/audioout.h>
#include <psp2/kernel/processmgr.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/audioout.h> 
#include <psp2/sas.h> 

#include "pspdebug.h"
#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_speech.h"	
#define printer psvDebugScreenPrintf

struct SceIoDirenter {
     char d_name[256];  
     struct SceIoDirenter * next;   
 };

SoLoud::Soloud gSoloud; // SoLoud engine
SoLoud::Wav gWave;      // One wave file

SceIoDirenter * head = NULL;
SceCtrlData ctrl_peek, ctrl_press;
int length = 0;
int select = 1;

void screenDraw() {
  psvDebugScreenClear();
  psvDebugScreenClearLineDisable();

  psvDebugScreenSetXY(0, 0);
  printer("\nCoolBoardPlaysSounds!\n--------------------\n");
  SceIoDirenter * current = head;

  for (int i = 0; i < length - 1; i++) {
    printer("%s\n", current->d_name);
    current = current->next;
  }
  printer("Exit\n");
  psvDebugScreenSetXY(30, (select * 1) + 2);
  printer("<\n");
}

char * getItem(int number) {
	sceClibPrintf("number %i\n", number);
  SceIoDirenter * currenter = head;
  	sceClibPrintf("name 1 %s\n", currenter->d_name);
  for (int i = 0; i < number - 1; i++) {
    currenter = currenter->next;
  }
  sceClibPrintf("name 2 %s\n", currenter->d_name);
  return currenter->d_name;
}

int main(int argc, const char *argv[]) {
	sceIoMkdir("ux0:data/CBPS", 0777);
	head = (SceIoDirenter *) malloc(sizeof(SceIoDirenter));
	SceIoDirenter * current = head;

	int dfd;
	dfd = sceIoDopen("ux0:data/CBPS");
	if(dfd >= 0) { 
		int res = 1;
		while(res > 0) {
			SceIoDirent dir;
			res = sceIoDread(dfd, &dir);
				memcpy(current->d_name, dir.d_name, strlen(dir.d_name)+1);
				sceClibPrintf("found a file! %s\n", current->d_name);
				current->next = NULL;
				current->next = (SceIoDirenter *) malloc(sizeof(SceIoDirenter));
				current = current->next;
				length = length + 1;
		}
  sceIoDclose(dfd);
  psvDebugScreenInit();
  screenDraw();
  gSoloud.init(); // Initialize SoLoud
  while(1) {
  	ctrl_press = ctrl_peek;
	sceCtrlPeekBufferPositive(0, &ctrl_peek, 1);
	ctrl_press.buttons = ctrl_peek.buttons & ~ctrl_press.buttons;
	if(ctrl_press.buttons == SCE_CTRL_UP) { 
		select = select - 1;
		if(select < 1) select = length;
		screenDraw();
	}
	if(ctrl_press.buttons == SCE_CTRL_DOWN) { 
		select = select + 1;
		if(select > length) select = 1;
		screenDraw();
	}
	if(ctrl_press.buttons == SCE_CTRL_CROSS || ctrl_press.buttons == SCE_CTRL_CIRCLE) {
		if(select == length) {
			break;
		} else {
			char path[1024] = "ux0:data/CBPS/\0";
			strcat(path, getItem(select));
			sceClibPrintf("path combined: %s\n", path);
			int res = 0;
			res = gWave.load(path); // Load a wav or ogg, others fail but don't crash so it's probably okay this way
  			sceClibPrintf("load path: %i\n", res);
  			res = gSoloud.play(gWave); // Play the wave
			sceClibPrintf("play path: %i\n", res);
		}
	}
  }
	}
	gSoloud.deinit();
	return 0;
}
