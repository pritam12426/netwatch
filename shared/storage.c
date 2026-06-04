#include "storage.h"
#include "log.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int storage_init(const char *path)
{
	FILE *fp = fopen(path, "rb");

	if (fp) {
		fclose(fp);
		return 0;
	}

	fp = fopen(path, "wb");
	if (!fp)
		return -1;

	NetmonHeader hdr = { 0 };

	memcpy(hdr.magic, NETMON_MAGIC, strlen(NETMON_MAGIC));
	hdr.version = NETMON_VERSION;

	fwrite(&hdr, sizeof(hdr), 1, fp);

	fclose(fp);

	return 0;
}


int storage_append(const char *path, const TrafficRecord *rec)
{
	FILE *fp = fopen(path, "ab");
	if (!fp)
		return -1;

	fwrite(rec, sizeof(*rec), 1, fp);

	fclose(fp);

	return 0;
}


int storage_dump(const char *path, int fd)
{
	FILE *fp = fopen(path, "rb");
	if (!fp)
		return -1;

	NetmonHeader hdr;

	if (fread(&hdr, sizeof(hdr), 1, fp) != 1) {
		fclose(fp);
		return -1;
	}

	if (memcmp(hdr.magic, NETMON_MAGIC, strlen(NETMON_MAGIC)) != 0) {
		fclose(fp);
		return -1;
	}

	if (hdr.version != NETMON_VERSION) {
		fclose(fp);
		return -1;
	}

	TrafficRecord rec;

	char out[512];

	while (fread(&rec, sizeof(rec), 1, fp) == 1) {
		snprintf(out,
		         sizeof(out),
		         "ts=%llu rx=%llu tx=%llu ip=%s\n",
		         (unsigned long long) rec.timestamp,
		         (unsigned long long) rec.rx_bytes,
		         (unsigned long long) rec.tx_bytes,
		         rec.ip);

		write(fd, out, strlen(out));
	}

	fclose(fp);

	return 0;
}
