// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include the main libnx system header, for Switch development
#include <switch.h>

extern PrintConsole logs_console;

const char *regions[] = {
    "Japan",
    "America",
    "Europe",
    "Australia/New Zealand",
    "Hong Kong/Taiwan/Korea",
    "China",
};

void rebootSystem(){
    spsmInitialize();
    spsmShutdown(true);
    spsmExit();
    printf("\033[0;31mSomething went wrong while rebooting!\033[0;37m\n");
	consoleUpdate(&logs_console);
}

bool set_90dns()
{
	Result res = 0;
	SetRegion region;
	u32 europeDns = 0xdb8daca3; // 163.172.141.219
	u32 americaDns = 0x4d79f6cf; // 207.246.121.77

	u32 primaryDns = 0;
	u32 secondaryDns = 0;
	res = setInitialize();
	if (res){
		printf("\033[0;31mEchec de l'initialisation de set! Err %x\033[0;37m\n", res);
		consoleUpdate(&logs_console);
		setExit();
		setsysExit();
		return false;
	}
	else {
		res = setsysInitialize();
		if (res){
			printf("\033[0;31mEchec de l'initialisation de setsys! Err %x\033[0;37m\n", res);
			consoleUpdate(&logs_console);
			setExit();
			setsysExit();
			return false;
		}
		else {
			res = setGetRegionCode(&region);
			if (res){
				printf("\033[0;31mEchec de la recupération de la region! Err %x\033[0;37m\n", res);
				consoleUpdate(&logs_console);
				setExit();
				setsysExit();
				return false;
			}
			else {
				if (region <= SetRegion_CHN){
					printf("Region %s detectee\n", regions[region]);
					consoleUpdate(&logs_console);
					if (region == SetRegion_USA){
						printf("\033[0;32mLe DNS americain sera utilise en tant que DNS primaire\033[0;37m\n");
						primaryDns = americaDns;
						secondaryDns = europeDns;
					}
					else {
						printf("\033[0;32mLe DNS europeen sera utilise en tant que DNS primaire\033[0;37m\n");
						primaryDns = europeDns;
						secondaryDns = americaDns;
					}
				}
				else {
					printf("\033[0;32mRegion inconnue? Le DNS americain sera utilise en tant que DNS primaire\033[0;37m\n");
					primaryDns = europeDns;
					secondaryDns = americaDns;
				}
				
			}
		}
	}

	printf("\nApplication de 90dns sur les reseaux wifi...\n");
	consoleUpdate(&logs_console);
	SetSysNetworkSettings* wifiSettings = (SetSysNetworkSettings*) malloc(sizeof(SetSysNetworkSettings) * 0x200);

	if (wifiSettings != NULL){
		s32 entryCount = 0;
		res = setsysGetNetworkSettings(&entryCount, wifiSettings, 0x200);
		if (res){
			printf("\033[0;31mEchec de la recupération des reseaux wifi! Err %x\033[0;37m\n", res);
			consoleUpdate(&logs_console);
			free(wifiSettings);
			setExit();
			setsysExit();
			return false;
		}
		else {
			printf("Reseau wifi trouve: %d\n", entryCount);
			consoleUpdate(&logs_console);
			for (int i = 0; i < entryCount; i++){
				wifiSettings[i].primary_dns = primaryDns;
				wifiSettings[i].secondary_dns = secondaryDns;
				wifiSettings[i].auto_settings &= ~SetSysAutoSettings_AutoDns;
			}

			if (entryCount){
				res = setsysSetNetworkSettings(wifiSettings, entryCount);
				if (res){
					printf("\033[0;31mEchec de la configuration du reseau wifi! Err %x\033[0;37m\n", res);
					consoleUpdate(&logs_console);
					free(wifiSettings);
					setExit();
					setsysExit();
					return false;
				}
				else {
					printf("\033[0;32mFini!\nLa console sera redemarree pour appliquer les changements\033[0;37m\n");
					consoleUpdate(&logs_console);
				}
			}
		}
	}
	free(wifiSettings);

	// Deinitialize and clean up resources used by the console (important!)
	setExit();
	setsysExit();
	return true;
}
