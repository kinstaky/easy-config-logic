#ifndef __SCALER_DEFS_H__
#define __SCALER_DEFS_H__

const int kRefeshPeriod = 1;
const int kRecordPeriod = 1; 


const size_t kScalerNum = 32;
const size_t kScalersOffset = 156;
const size_t kMaxScalerArraySize = 256;
const size_t kBufferSize = 2048;


struct ScalerRequest {
	uint64_t client_time;
	uint64_t request_time;
};

struct ScalerResponse {
	uint32_t status;
};

struct ScalerData {
	uint64_t seconds;
	uint32_t scaler[kScalerNum];
};

enum LogLevel {
	kError = 0,
	kWarning,
	kInfo,
	kDebug
};

const char *log_level_name[] = {
	"error",
	"warning",
	"info",
	"debug"
};

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int SetLogLevel(char *level, LogLevel &log_level) {
	bool set_log_level = false;
	for (size_t i = 0; i < 4; ++i) {
		if (strcmp(level, log_level_name[i]) == 0) {
			log_level = LogLevel(kError+i);
			set_log_level = true;
			break;
		}
	}
	if (!set_log_level) {
		// this argument doesn't match any log level name
		fprintf(stderr, "Error: Invalid log level %s.", level);
		fprintf(stderr, "Log level should be one of following:\n"
			"error, warning, info, debug.\n");
		return -1;
	}
	return 0;
}


#endif 				// __SCALER_DEFS_H__