#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include "main_util.h"
#include "web_launch.h"

void showError(char* errorText1, char* errorText2, Result outrc)
{
	AppletHolder err;
	AppletStorage errStor;
	LibAppletArgs errArgs;

	appletCreateLibraryApplet(&err, AppletId_LibraryAppletError, LibAppletMode_AllForeground);
	libappletArgsCreate(&errArgs, 1);
	libappletArgsPush(&errArgs, &err);
	appletCreateStorage(&errStor, 4120);
	u8 args[4120] = {0};
	args[0] = 1;

	u64 e = (((outrc & 0x1ffu) + 2000) | (((outrc >> 9) & 0x1fff & 0x1fffll) << 32));
	*(u64*)&args[8] = e;
	strcpy((char*) &args[24], errorText1);
	strcpy((char*) &args[2072], errorText2);
	appletStorageWrite(&errStor, 0, args, 4120);
	appletHolderPushInData(&err, &errStor);

	appletHolderStart(&err);
	appletHolderJoin(&err);
	exit(1);
}

void web_launch(const char* url) {
	Result rc=0;
	nsvmInitialize();
	pminfoInitialize();

	bool nagOn;
	nsvmNeedsUpdateVulnerability(&nagOn);
	if(nagOn) {
		s32 pCount;
		u64 pids[100];
		u64 cId;
		u32 i = 0;
		bool isPatched = false;
		svcGetProcessList(&pCount, pids, 100);
		while (i <= pCount-1) {
			pminfoGetProgramId(&cId, pids[i]);
			if(cId == 0x00FF747765616BFF || cId == 0x01FF415446660000) {
				// printf(CONSOLE_GREEN "Supernag enabled, but patched via switch-sys-tweak!\n");
				isPatched = true;
				break;
			}
			i++;
		}
		if(!isPatched) {
			showError("Error: Nag active, check more details", "Browser won't launch if supernag is active\n\nUse gagorder or switch-sys-tweak (the latter is bundled with BrowseNX) to disable supernag.", 0);
		}
	}

	pminfoExit();
	nsvmExit();

	if(!isApplet()) { // Running as a title
		WebCommonConfig conf;
		WebCommonReply out;
		rc = webPageCreate(&conf, url);
		if (R_FAILED(rc)) {
			showError("Error starting Browser\nLookup error code for more info", "", rc);
		}
		webConfigSetJsExtension(&conf, true);
		webConfigSetPageCache(&conf, true);
		webConfigSetBootLoadingIcon(&conf, true);
		webConfigSetWhitelist(&conf, ".*");
		rc = webConfigShow(&conf, &out);
		if (R_FAILED(rc)) {
			showError("Error starting Browser\nLookup error code for more info", "", rc);
		}
	} else { // Running under applet
		showError("Running in applet mode\nPlease launch hbmenu by holding R on an APP (e.g. a game) NOT an applet (e.g. Gallery)", "", 0);
	}

}