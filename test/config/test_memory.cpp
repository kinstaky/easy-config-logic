#include "config/memory.h"

#include <string>
#include <sstream>
#include <vector>

#include <gtest/gtest.h>

using namespace ecl;

TEST(MemoryTest, Size) {
	ASSERT_EQ(sizeof(Memory), 232*4);
}