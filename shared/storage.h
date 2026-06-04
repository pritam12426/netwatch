#ifndef _STORAGE_H_
#define _STORAGE_H_


#include <stdint.h>

#define NETMON_MAGIC   "NETMON"
#define NETMON_VERSION 1

typedef struct {
	char     magic[8];
	uint32_t version;
	uint32_t reserved;
} NetmonHeader;

typedef struct {
	uint64_t timestamp;
	uint64_t rx_bytes;
	uint64_t tx_bytes;
	char     ip[60];
} TrafficRecord;

int storage_init(const char *path);
int storage_append(const char *path, const TrafficRecord *rec);
int storage_dump(const char *path, int fd);


#endif  // _STORAGE_H_
