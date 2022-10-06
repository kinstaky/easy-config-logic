#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

const size_t kScalerNum = 32;
const size_t kScalersOffset = 156;
const unsigned int pclkFrequency = 100000000;

void SigIntHandler(int) {
	if (system("tput rmcup") != 0) {
		fprintf(stderr, "Error: bash command tput rmcup failed.\n");
		fprintf(stderr, "Please type `tput rmcup` or printf '\e[2J\e[?47l\e8'`"
			"by yourself to switch back to the primary screen.\n");
		exit(-1);
	}
	// printf("You press Ctrl+C to quit.\n");
	exit(0);
}


int main() {
    const int memory_size = 4096;

    // open file
    int fd = open("/dev/uio0", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Error: Failed to open /dev/uio0.\n");
        return -1;
    }
    // lock the address space
    if (flock(fd, LOCK_EX | LOCK_NB)) {
        fprintf(stderr, "Error: Failed to get file lock on /dev/uio0\n");
        return -2;
    }
	// map
    void *map_addr = mmap(NULL, memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_addr == MAP_FAILED) {
        fprintf(stderr, "Error: Failed to mmap\n");
        return -3;
    }
    volatile uint32_t *mapped = (unsigned int *)map_addr;

	if (system("tput smcup") != 0) {
		fprintf(stderr, "Error: bash command tput smcup failed.\n");
	}
	signal(SIGINT, SigIntHandler);
	uint32_t scaler_value[kScalerNum];
	while (1) {
		for (size_t i = 0; i < kScalerNum; i++) {
			scaler_value[i] = (mapped[kScalersOffset+i] >> 12) & 0xfffff;
			scaler_value[i] = scaler_value[i] > 0 ? scaler_value[i] - 1 : 0;
		}
		if (system("clear") != 0) {
			fprintf(stderr, "Error: bash command clear failed\n");
			return -1;
		}
		printf("scalar      counts\n");
		for (size_t i = 0; i < kScalerNum; i++) {
			printf("%2u%15d\n", (uint32_t)i, scaler_value[i]);
		}
		usleep(500000);
	}

	return 0;
}