// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include the main libnx system header, for Switch development
#include <switch.h>

const char *regions[] = {
    "Japan",
    "America",
    "Europe",
    "Australia/New Zealand",
    "Hong Kong/Taiwan/Korea",
    "China",
};

int set_90dns()
{
	Result res = 0;
	SetRegion region;
	u32 europeDns = 0xdb8daca3; // 163.172.141.219
	u32 americaDns = 0x4d79f6cf; // 207.246.121.77

	u32 primaryDns = 0;
	u32 secondaryDns = 0;
	res = setInitialize();
	if (res){
		printf("Echec de l'initialisation de set! Err %x\n", res);
		consoleUpdate(NULL);
		return 1;
	}
	else {
		res = setsysInitialize();
		if (res){
			printf("Echec de l'initialisation de setsys! Err %x\n", res);
			consoleUpdate(NULL);
			return 1;
		}
		else {
			res = setGetRegionCode(&region);
			if (res){
				printf("Echec de la récupération de la région! Err %x\n", res);
				consoleUpdate(NULL);
				return 1;
			}
			else {
				if (region <= SetRegion_CHN){
					printf("Région %s détectée\n", regions[region]);
					consoleUpdate(NULL);
					if (region == SetRegion_USA){
						printf("American dns will be used as primary\n");
						primaryDns = americaDns;
						secondaryDns = europeDns;
					}
					else {
						printf("Europe dns will be used as primary\n");
						primaryDns = europeDns;
						secondaryDns = americaDns;
					}
				}
				else {
					printf("Unknown region? Europe dns will be used as primary\n");
					primaryDns = europeDns;
					secondaryDns = americaDns;
				}
				
			}
		}
	}

	printf("\nApplication de 90dns sur les réseaux wifi...\n");
	consoleUpdate(NULL);
	SetSysNetworkSettings* wifiSettings = malloc(sizeof(SetSysNetworkSettings) * 0x200);

	if (wifiSettings != NULL){
		s32 entryCount = 0;
		res = setsysGetNetworkSettings(&entryCount, wifiSettings, 0x200);
		if (res){
			printf("Echec de la récupération des réseaux wifi! Err %x\n", res);
			consoleUpdate(NULL);
		}
		else {
			printf("Réseau wifi trouvé: %d\n", entryCount);
			consoleUpdate(NULL);
			for (int i = 0; i < entryCount; i++){
				wifiSettings[i].primary_dns = primaryDns;
				wifiSettings[i].secondary_dns = secondaryDns;
				wifiSettings[i].auto_settings &= ~SetSysAutoSettings_AutoDns;
			}

			if (entryCount){
				res = setsysSetNetworkSettings(wifiSettings, entryCount);
				if (res){
					printf("Echec de la configuration du réseau wifi! Err %x\n", res);
					consoleUpdate(NULL);
				}
				else {
					printf("Terminé!\nLa console sera redémarrée pour appliquer les changements\n");
					consoleUpdate(NULL);
				}
			}
		}
	}
	free(wifiSettings);

	// Deinitialize and clean up resources used by the console (important!)
	setExit();
	setsysExit();
	return 0;
}
