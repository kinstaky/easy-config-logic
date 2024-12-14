#include "config/memory_config.h"

#include <fstream>
#include <string>

#include "gtest/gtest.h"

#include "config/config_parser.h"
#include "memory.h"

#ifndef TEST_DATA_DIRECTORY
#define TEST_DATA_DIRECTORY ""
#endif

using namespace ecl;

const std::string kTestDataDir = TEST_DATA_DIRECTORY;


TEST(MemoryConfigTest, FileReadWrite) {
	MemoryConfig config0;
	ASSERT_EQ(config0.Read((kTestDataDir + "register_config_0_origin.txt").c_str()), 0)
		<< "Error: config0 read failed.";
	ASSERT_EQ(config0.Write((kTestDataDir + "register_config_0_generate.txt").c_str()), 0)
		<< "Error: config0 write failed.";
	const Memory *memory0 = config0.GetMemory();

	MemoryConfig config1;
	ASSERT_EQ(config1.Read((kTestDataDir + "register_config_0_generate.txt").c_str()), 0)
		<< "Error: config1 read failed.";
	const Memory *memory1 = config1.GetMemory();

	EXPECT_EQ(memcmp(memory0, memory1, sizeof(Memory)), 0);
}


TEST(MemoryConfigTest, ReadLogicExpression) {
	ConfigParser parser;
	ASSERT_EQ(parser.Read(kTestDataDir + "logic_config_1.txt"), 0)
		<< "Error: Parser read file " << kTestDataDir << "logic_config_1.txt" << " failed.";

	// read config from parser
	MemoryConfig config0;
	ASSERT_EQ(config0.Read(&parser), 0)
		<< "Error: config0 read failed.";
	EXPECT_EQ(config0.Write((kTestDataDir + "register_config_1_generate.txt").c_str()), 0)
		<< "Error: config0 write failed.";
	const Memory *memory0 = config0.GetMemory();

	// read config from file
	MemoryConfig config1;
	ASSERT_EQ(config1.Read((kTestDataDir + "register_config_1.txt").c_str()), 0)
		<< "Error: config1 read failed.";
	const Memory *memory1 = config1.GetMemory();

	EXPECT_EQ(memcmp(memory0, memory1, sizeof(Memory)), 0);
}