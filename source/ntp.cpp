#include <string>
#include <utility>
#include <vector>

#include <arpa/inet.h>
#include <errno.h>
#include <exception>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <switch.h>

#include "main_util.h"
#include "ntp.hpp"

typedef std::vector<std::pair<std::string, std::string>> vector_pair;

std::vector<std::string> vectorPairKeys(vector_pair pair) {
	std::vector<std::string> keys;
	for (auto const& [key, val] : pair) {
		keys.push_back(key);
	}
	return keys;
}

std::vector<std::string> vectorPairValues(vector_pair pair) {
	std::vector<std::string> values;
	for (auto const& [key, val] : pair) {
		values.push_back(val);
	}
	return values;
}

vector_pair NTPSERVERS = {
	{"NTP Pool - Main", "pool.ntp.org"},
	{"Cloudflare", "time.cloudflare.com"},
	{"Google", "time.google.com"},
	{"NIST - Main", "time.nist.gov"},
	{"NTP Pool - Africa", "africa.pool.ntp.org"},
	{"NTP Pool - Asia", "asia.pool.ntp.org"},
	{"NTP Pool - Europe", "europe.pool.ntp.org"},
	{"NTP Pool - North America", "north-america.pool.ntp.org"},
	{"NTP Pool - Oceania", "oceania.pool.ntp.org"},
	{"NTP Pool - South America", "south-america.pool.ntp.org"},
};

#define UNIX_OFFSET 2208988800L

#define NTP_DEFAULT_PORT "123"
#define DEFAULT_TIMEOUT 3

// Flags 00|100|011 for li=0, vn=4, mode=3
#define NTP_FLAGS 0x23

typedef struct {
	uint8_t flags;
	uint8_t stratum;
	uint8_t poll;
	uint8_t precision;
	uint32_t root_delay;
	uint32_t root_dispersion;
	uint8_t referenceID[4];
	uint32_t ref_ts_secs;
	uint32_t ref_ts_frac;
	uint32_t origin_ts_secs;
	uint32_t origin_ts_frac;
	uint32_t recv_ts_secs;
	uint32_t recv_ts_fracs;
	uint32_t transmit_ts_secs;
	uint32_t transmit_ts_frac;

} ntp_packet;

struct NtpException : public std::exception {
protected:
	int m_code;
	std::string m_message;

public:
	NtpException(int code, std::string message) {
		m_code = code;
		m_message = message;
	}

	std::string what() noexcept {
		return m_message;
	}
};

class NTPClient {
private:
	int m_timeout;
	const char* m_port;
	const char* m_server;

public:
	NTPClient(const char* server, const char* port, int timeout) {
		m_server = server;
		m_port = port;
		m_timeout = timeout;
	}

	NTPClient(const char* server, const char* port) : NTPClient(server, port, DEFAULT_TIMEOUT) {}
	NTPClient(const char* server) : NTPClient(server, NTP_DEFAULT_PORT, DEFAULT_TIMEOUT) {}
	NTPClient() : NTPClient("pool.ntp.org", NTP_DEFAULT_PORT, DEFAULT_TIMEOUT) {}

	void setTimeout(int timeout) {
		m_timeout = timeout;
	}

	time_t getTime() noexcept(false) {
		int server_sock, status;
		struct addrinfo hints, *servinfo, *ap;
		socklen_t addrlen = sizeof(struct sockaddr_storage);
		ntp_packet packet = {.flags = NTP_FLAGS};

		hints = (struct addrinfo){.ai_family = AF_INET, .ai_socktype = SOCK_DGRAM};

		if ((status = getaddrinfo(m_server, m_port, &hints, &servinfo)) != 0) {
			throw NtpException(1, "Unable to get address info (" + std::string(gai_strerror(status)) + ")");
		}

		for (ap = servinfo; ap != NULL; ap = ap->ai_next) {
			server_sock = socket(ap->ai_family, ap->ai_socktype, ap->ai_protocol);
			if (server_sock != -1)
				break;
		}

		if (ap == NULL) {
			throw NtpException(2, "Unable to create the socket");
		}

		struct timeval timeout = {.tv_sec = m_timeout, .tv_usec = 0};

		if (setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
			throw NtpException(3, "Unable to set RCV timeout");
		}

		if (setsockopt(server_sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
			throw NtpException(4, "Unable to set SND timeout");
		}

		if ((status = sendto(server_sock, &packet, sizeof(packet), 0, ap->ai_addr, ap->ai_addrlen)) == -1) {
			throw NtpException(5, "Unable to send packet");
		}

		if ((status = recvfrom(server_sock, &packet, sizeof(packet), 0, ap->ai_addr, &addrlen)) == -1) {
			if (errno == 11 || errno == 35) { // NX: 11, OTH: 35
				throw NtpException(6, "Connection timeout, retry. (" + std::to_string(m_timeout) + "s)");
			} else {
				throw NtpException(7, "Unable to receive packet (" + std::to_string(errno) + ")");
			}
		}

		freeaddrinfo(servinfo);
		close(server_sock);

		packet.recv_ts_secs = ntohl(packet.recv_ts_secs);

		return packet.recv_ts_secs - UNIX_OFFSET;
	}

	long getTimeOffset(time_t currentTime) noexcept(false) {
		time_t ntpTime = getTime();
		return currentTime - ntpTime;
	}
};

TimeServiceType __nx_time_service_type = TimeServiceType_System;
std::string Message = "";
int currentServer = 0;
std::vector<std::string> serverAddresses;
std::vector<std::string> serverNames;

	std::string getCurrentServerAddress() {
		return serverAddresses[currentServer];
	}

	bool setNetworkSystemClock(time_t time) {
		Result rs = timeSetCurrentTime(TimeType_NetworkSystemClock, (uint64_t)time);
		if (R_FAILED(rs)) {
			return false;
		}
		return true;
	}

	bool setTime() {
		std::string srv = getCurrentServerAddress();
		NTPClient* client = new NTPClient(srv.c_str());

		try {
			time_t ntpTime = client->getTime();

			if (setNetworkSystemClock(ntpTime)) {
				Message = "Synced with " + srv;
				delete client;
				return true;
			} else {
				Message = "Unable to set network clock.";
				delete client;
				return false;
			}
		} catch (NtpException& e) {
			Message = "Error: " + e.what();
		}

		delete client;
		return false;
	}

	bool setNetworkTimeAsUser() {
		time_t userTime, netTime;

		Result rs = timeGetCurrentTime(TimeType_UserSystemClock, (u64*)&userTime);
		if (R_FAILED(rs)) {
			Message = "GetTimeUser " + std::to_string(rs);
			return false;
		}

		std::string usr = "User time!";
		std::string gr8 = "";
		rs = timeGetCurrentTime(TimeType_NetworkSystemClock, (u64*)&netTime);
		if (!R_FAILED(rs) && netTime < userTime) {
			gr8 = " Great Scott!";
		}

		if (setNetworkSystemClock(userTime)) {
			Message = usr.append(gr8);
			return true;
		} else {
			Message = "Unable to set network clock.";
			return false;
		}
	}

	bool getOffset(time_t *ntpTimeOffset) {
		time_t currentTime;
		Result rs = timeGetCurrentTime(TimeType_NetworkSystemClock, (u64*)&currentTime);
		if (R_FAILED(rs)) {
			Message = "GetTimeNetwork " + std::to_string(rs);
			return false;
		}

		std::string srv = getCurrentServerAddress();
		NTPClient* client = new NTPClient(srv.c_str());

		try {
			time_t ntpTimeOffset = client->getTimeOffset(currentTime);
			Message = "Offset: " + std::to_string(ntpTimeOffset) + "s";
			delete client;
			return true;
		} catch (NtpException& e) {
			Message = "Error: " + e.what();
		}

		delete client;
		return false;
	}

bool launch_sync_time() {
	nifmInitialize(NifmServiceType_User);
	timeInitialize();
	smInitialize();

	bool success = false;

		// if (internet_is_connected()) {
		serverAddresses = vectorPairValues(NTPSERVERS); // Récupérer la liste des serveurs NTP

		// Vérifier l'écart entre l'heure utilisateur et l'heure réseau
		time_t currentConsoleTime;
		time_t time_diff = 31;
		timeGetCurrentTime(TimeType_UserSystemClock, (u64*)&currentConsoleTime);
		for (size_t i = 0; i < serverAddresses.size(); ++i) {
			currentServer = i;
			if (getOffset(&time_diff)) {
				if (time_diff >= 30) {
					success = setTime();
					if (success) break;
				} else {
					Message = "No synchronization needed. Offset: " + std::to_string(time_diff) + "s";
					success = true;
					break;
				}
			}
		}
	// }

	if (!success) {
		setNetworkTimeAsUser();
		Message = "Fallback: Synchronized using user time.";
	}

	nifmExit();
	timeExit();
	smExit();

	return success;
}