#ifndef _90dns_tester_HPP_
#define _90dns_tester_HPP_

typedef enum RESOLVER_STATUS {
    DNS_BLOCKED,
    DNS_RESOLVED,
    DNS_UNRESOLVED
} RESOLVER_STATUS;

RESOLVER_STATUS resolveHostname(const char* hostname);
int checkHostnames();

#endif