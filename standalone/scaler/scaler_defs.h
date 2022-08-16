#ifndef __SCALER_DEFS_H__
#define __SCALER_DEFS_H__

const int kRefeshPeriod = 1;
const int kRecordPeriod = 4; 


const size_t kScalerNum = 32;
const size_t kScalersOffset = 156;
const size_t kScalerStackSize = 8;
const size_t kMaxScalerArraySize = kScalerNum * kScalerStackSize * 4;
const size_t kBufferSize = 2048;


const uint32_t kTypeDateRequest = 1;
const uint32_t kTypeScalerRequest = 2;


struct RequestHeader {
	uint32_t type;
};

struct DateRequest {
	struct RequestHeader header;
};


struct DateResponse {
	uint64_t seconds;
};

struct ScalerRequest {
	struct RequestHeader header;
	uint32_t size;
	uint64_t seconds;
};

struct ScalerData {
	uint64_t seconds;
	uint32_t scaler[kScalerNum];
};

struct ScalerResponse {
	uint32_t status;
	uint32_t size;
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



#endif 				// __SCALER_DEFS_H__