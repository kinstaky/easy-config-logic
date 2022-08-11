#include "config/memory_config.h"

#include <fstream>
#include <string>

#include "gtest/gtest.h"

#include "config/logic_parser.h"


#ifndef TEST_DATA_DIRECTORY
#define TEST_DATA_DIRECTORY ""
#endif

using namespace ecl;

const std::string kTestDataDir = TEST_DATA_DIRECTORY;

void CompareMomory(const MemoryConfig::Memory *memory0, const MemoryConfig::Memory *memory1) {
	// compare front io
	for (size_t i = 0; i < MemoryConfig::kFrontIoGroupNum; ++i) {
		EXPECT_EQ(memory0->rj45_enable[i], memory1->rj45_enable[i])
			<< "Error: rj45_enable " << i;

		EXPECT_EQ(memory0->pl_out_enable[i], memory1->pl_out_enable[i])
			<< "Error: pl_out_enable " << i;
	
		EXPECT_EQ(memory0->front_input_inverse[i], memory1->front_input_inverse[i])
			<< "Error: front_input_inverse " << i;
		
		EXPECT_EQ(memory0->front_output_inverse[i], memory1->front_output_inverse[i])
			<< "Error: front_output_inverse " << i;
	}
	for (size_t i = 0; i < MemoryConfig::kFrontIoNum; ++i) {
		EXPECT_EQ(memory0->front_io_source[i], memory1->front_io_source[i])
			<< "Error: front_io_source " << i;
	}

	// compare multi
	for (size_t i = 0; i < MemoryConfig::kMultiNum; ++i) {
		for (size_t j = 0; j < MemoryConfig::kFrontIoGroupNum; ++j) {
			EXPECT_EQ(memory0->multi_front_selection[i][j], memory1->multi_front_selection[i][j])
				<< "Error: multi_front_selection " << i << ", " << j;
		}
		EXPECT_EQ(memory0->multi_threshold[i], memory1->multi_threshold[i])
			<< "Error: multi_threshold " << i;
	}

	// compare or gatess
	for (size_t i = 0; i < MemoryConfig::kOrGateNum; ++i) {
		for (size_t j = 0; j < MemoryConfig::kFrontIoGroupNum; ++j) {
			EXPECT_EQ(memory0->or_front_selection[i][j], memory1->or_front_selection[i][j])
				<< "Error: or_front_selection " << i << ", " << j;
		}
		EXPECT_EQ(memory0->or_multi_selection[i], memory1->or_multi_selection[i])
			<< "Error: or_multi_selection " << i;
	}
	
	// compare and gates
	for (size_t i = 0; i < MemoryConfig::kAndGateNum; ++i) {
		for (size_t j = 0; j < MemoryConfig::kFrontIoGroupNum; ++j) {
			EXPECT_EQ(memory0->and_front_selection[i][j], memory1->and_front_selection[i][j])
				<< "Error: and_front_selection " << i << ", " << j;
		}
		EXPECT_EQ(memory0->and_multi_selection[i], memory1->and_multi_selection[i])
			<< "Error: and_multi_selection " << i;
		EXPECT_EQ(memory0->and_or_selection[i], memory1->and_or_selection[i])
			<< "Error: and_or_selection " << i;
	}

	// compare back
	EXPECT_EQ(memory0->back_enable, memory1->back_enable)
		<< "Error: back_enable";
	EXPECT_EQ(memory0->back_source, memory1->back_source)
		<< "Error: back source";

	// compare divider
	for (size_t i = 0; i < MemoryConfig::kDividerNum; ++i) {
		EXPECT_EQ(memory0->divider_source[i], memory1->divider_source[i])
			<< "Error: divider_source " << i;
		EXPECT_EQ(memory0->divider_divisor[i], memory1->divider_divisor[i])
			<< "Error: divider_divisor " << i;
	}

	// compare divider gates
	for (size_t i = 0; i < MemoryConfig::kDividerGateNum; ++i) {
		EXPECT_EQ(memory0->divider_gate_operator_type[i], memory1->divider_gate_operator_type[i])
			<< "Error: divider_gate_operator_type " << i;
		EXPECT_EQ(memory0->divider_gate_divider_source[i], memory1->divider_gate_divider_source[i])
			<< "Error: divider_gate_divider_source " << i;
		EXPECT_EQ(memory0->divider_gate_other_source[i], memory1->divider_gate_other_source[i])
			<< "Error: divider_gate_other_source " << i;
	}

	// compare scalers
	for (size_t i = 0; i < MemoryConfig::kScalerNum; ++i) {
		EXPECT_EQ(memory0->scaler_source[i], memory1->scaler_source[i])
			<< "Error: scaler_source " << i;
		EXPECT_EQ(memory0->scaler_clock_source[i], memory1->scaler_clock_source[i])
			<< "Error: scaler_clock_source " << i;
	}
}


TEST(MemoryConfigTest, FileReadWrite) {
	MemoryConfig config0;
	ASSERT_EQ(config0.Read((kTestDataDir + "register_config_0_origin.txt").c_str()), 0)
		<< "Error: config0 read failed.";
	ASSERT_EQ(config0.Write((kTestDataDir + "register_config_0_generate.txt").c_str()), 0)
		<< "Error: config0 write failed.";
	const MemoryConfig::Memory *memory0 = config0.GetMemory();

	MemoryConfig config1;
	ASSERT_EQ(config1.Read((kTestDataDir + "register_config_0_generate.txt").c_str()), 0)
		<< "Error: config1 read failed.";
	const MemoryConfig::Memory *memory1 = config1.GetMemory();

	CompareMomory(memory0, memory1);
}


TEST(MemoryConfigTest, ReadLogicExpression) {
	LogicParser parser;
	ASSERT_EQ(parser.Read(kTestDataDir + "logic_config_1.txt"), 0)
		<< "Error: Parser read file " << kTestDataDir << "logic_config_1.txt" << " failed.";

	// read config from parser
	MemoryConfig config0;
	ASSERT_EQ(config0.Read(&parser), 0)
		<< "Error: config0 read failed.";
	EXPECT_EQ(config0.Write((kTestDataDir + "register_config_1_generate.txt").c_str()), 0)
		<< "Error: config0 write failed.";
	const MemoryConfig::Memory *memory0 = config0.GetMemory();

	// read config from file
	MemoryConfig config1;
	ASSERT_EQ(config1.Read((kTestDataDir + "register_config_1.txt").c_str()), 0)
		<< "Error: config1 read failed.";
	const MemoryConfig::Memory *memory1 = config1.GetMemory();

	CompareMomory(memory0, memory1);
}