// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include the main libnx system header, for Switch development
#include <switch.h>

#include "90dns_setter.hpp"
#include "main_util.h"
#include "translate.hpp"

const char *regions[] = {
    "Japan",
    "America",
    "Europe",
    "Australia/New Zealand",
    "Hong Kong/Taiwan/Korea",
    "China",
};

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
		printf("\033[0;31m");
		printf(language_vars["lng_dns_install_set_init_error"].c_str(), res);
		printf("\033[0;37m\n");
		consoleUpdate(&logs_console);		return false;
	}
	else {
		res = setsysInitialize();
		if (res){
			printf("\033[0;31m");
			printf(language_vars["lng_dns_install_setsys_init_error"].c_str(), res);
			printf("\033[0;37m\n");
			consoleUpdate(&logs_console);
			setExit();
			return false;
		}
		else {
			res = setGetRegionCode(&region);
			if (res){
				printf("\033[0;31m");
				printf(language_vars["lng_dns_install_get_region_code_error"].c_str(), res);
				printf("\033[0;37m\n");
				consoleUpdate(&logs_console);
				setExit();
				setsysExit();
				return false;
			}
			else {
				if (region <= SetRegion_CHN){
					printf(language_vars["lng_dns_install_display_region_code"].c_str(), regions[region]);
					printf("\n");
					consoleUpdate(&logs_console);
					if (region == SetRegion_USA){
						printf("\033[0;32m");
						printf(language_vars["lng_dns_install_use_us_dns"].c_str());
						printf("\033[0;37m\n");
						primaryDns = americaDns;
						secondaryDns = europeDns;
					}
					else {
						printf("\033[0;32m");
						printf(language_vars["lng_dns_install_use_eu_dns"].c_str());
						printf("\033[0;37m\n");
						primaryDns = europeDns;
						secondaryDns = americaDns;
					}
				}
				else {
					printf("\033[0;32m");
					printf(language_vars["lng_dns_install_use_unknown_dns"].c_str());
					printf("\033[0;37m\n");
					primaryDns = europeDns;
					secondaryDns = americaDns;
				}
				
			}
		}
	}

	printf("\n");
	printf(language_vars["lng_dns_install_aplying"].c_str());
	printf("\n");
	consoleUpdate(&logs_console);
	SetSysNetworkSettings* wifiSettings = (SetSysNetworkSettings*) malloc(sizeof(SetSysNetworkSettings) * 0x200);

	if (wifiSettings != NULL){
		s32 entryCount = 0;
		res = setsysGetNetworkSettings(&entryCount, wifiSettings, 0x200);
		if (res){
			printf("\033[0;31m");
			printf(language_vars["lng_dns_install_get_wifi_networks_error"].c_str(), res);
			printf("\033[0;37m\n");
			consoleUpdate(&logs_console);
			free(wifiSettings);
			setExit();
			setsysExit();
			return false;
		}
		else {
			printf(language_vars["lng_dns_install_networs_count"].c_str(), entryCount);
			printf("\n");
			consoleUpdate(&logs_console);
			for (int i = 0; i < entryCount; i++){
				wifiSettings[i].primary_dns = primaryDns;
				wifiSettings[i].secondary_dns = secondaryDns;
				wifiSettings[i].auto_settings &= ~SetSysAutoSettings_AutoDns;
			}

			if (entryCount){
				res = setsysSetNetworkSettings(wifiSettings, entryCount);
				if (res){
					printf("\033[0;31m");
					printf(language_vars["lng_dns_install_wifi_network_config_error"].c_str(), res);
					printf("\033[0;37m\n");
					consoleUpdate(&logs_console);
					free(wifiSettings);
					setExit();
					setsysExit();
					return false;
				}
				else {
					printf("\033[0;32m");
					printf(language_vars["lng_dns_install_success"].c_str());
					printf("\033[0;37m\n");
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
