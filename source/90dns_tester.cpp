#include <stdio.h>
#include <switch.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "90dns_tester.hpp"
#include "main_util.h"

extern bool debug_enabled;

// List of hostnames to test
const char *host_names_list[] = {
    "nintendo.com",
    "nintendo.com",
    "nintendo.net",
    "nintendo.jp",
    "nintendo.co.jp",
    "nintendo.co.uk",
    "nintendo-europe.com",
    "nintendowifi.net",
    "nintendo.es",
    "nintendo.co.kr",
    "nintendo.tw",
    "nintendo.com.hk",
    "nintendo.com.au",
    "nintendo.co.nz",
    "nintendo.at",
    "nintendo.be",
    "nintendods.cz",
    "nintendo.dk",
    "nintendo.de",
    "nintendo.fi",
    "nintendo.fr",
    "nintendo.gr",
    "nintendo.hu",
    "nintendo.it",
    "nintendo.nl",
    "nintendo.no",
    "nintendo.pt",
    "nintendo.ru",
    "nintendo.co.za",
    "nintendo.se",
    "nintendo.ch",
    "nintendoswitch.cn",
    "nintendoswitch.com.cn",
    "nintendoswitch.com",
    "sun.hac.lp1.d4c.nintendo.net"
};

int checkHostnames() 
{
    if (debug_enabled) {
        debug_log_write("\nTest 90DNS.\n");
    }
    nifmInitialize(NifmServiceType_User);
    // Check if system says we're connected to a network (wifi or ethernet)
    Result net_rc = nifmGetInternetConnectionStatus(NULL, NULL, NULL);
    if (R_FAILED(net_rc)) {
        nifmExit();
        if (debug_enabled) {
            debug_log_write("Aucune connexion réseau.\n\n");
        }
        return 1;
    }

    // Iterate through hostnames array
    for (uint32_t i = 0; i < sizeof(host_names_list)/sizeof(host_names_list[0]); i++)
    {
        // Resolve the hostname
        int result = resolveHostname(host_names_list[i]);
        switch(result) {
            case DNS_BLOCKED:
                break;
            case DNS_RESOLVED:
                nifmExit();
                if (debug_enabled) {
                    debug_log_write("Console non protégée.\n\n");
                }
                return 2;
                break;
            case DNS_UNRESOLVED:
                break;
        }
    }

    nifmExit();
    if (debug_enabled) {
        debug_log_write("Protection active.\n\n");
    }
    return 0;
}

RESOLVER_STATUS resolveHostname(const char* hostname)
{
    struct hostent *he;
    struct in_addr a;
    // use gethostbyname to attempt hostname connection
    he = gethostbyname(hostname);
    if (he)
    {
        // go over all returned addresses for hostname
        while (*he->h_addr_list)
        {
            bcopy(*he->h_addr_list++, (char *) &a, sizeof(a));
            // succeed if any of them redirect to localhost
            if (strcmp(inet_ntoa(a), "127.0.0.1") == 0 || strcmp(inet_ntoa(a), "0.0.0.0") == 0)
            {
                return DNS_BLOCKED;
            }
        }
        return DNS_RESOLVED;
    }
    return DNS_UNRESOLVED;
}
