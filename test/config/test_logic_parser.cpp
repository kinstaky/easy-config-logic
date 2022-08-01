#include "config/logic_parser.h"

#include <string>
#include <sstream>
#include <vector>

#include <gtest/gtest.h>

using namespace ecc;


const std::vector<std::string> kExpressions = {
	"A0 = B2",
	"A13 = A3 | A7",
	"B13 = A3 & A7",
	"A10 = (A3 | A7) & (B15 | B11)",
	"A14 = (C8 & C12) | C15 | C11"
};
const std::vector<std::string> kLeft = {
	"A0", "A13", "B13", "A10", "A14"
};
const std::vector<std::string> kRight = {
	" B2",
	" A3 | A7",
	" A3 & A7",
	" (A3 | A7) & (B15 | B11)",
	" (C8 & C12) | C15 | C11"
};
const std::vector<size_t> kOutputSize = {
	1, 2, 3, 4, 5
};
const std::vector<size_t> kOrGateSize = {
	0, 1, 1, 2, 4
};
const std::vector<size_t> kAndGateSize = {
	0, 0, 1, 2, 3
};
const std::vector<OutputInfo> kFrontOuputs = {
	{0, 18},
	{13, kOrGatesOffset + 0},
	{29, kAndGatesOffset + 0},
	{10, kAndGatesOffset + 1},
	{14, kAndGatesOffset + 2}
};
const std::vector<std::bitset<kOrBits>> kOrGates = {
	0x88,
	0x8800'0000,
	0x8900'0000'0000,
	0x9800'0000'0000
};
const std::vector<std::bitset<kAndBits>> kAndGates = {
	0x88,
	0x3'0000'0000'0000,
	0xc'0000'0000'0000
};
const std::vector<std::string> kClocks0 = {
	"clock_20MHz", "clock_5MHz", "clock_1Hz"
};
const std::vector<size_t> kClocks0Frequency = {
	20'000'000u, 5'000'000u, 1u
};
const std::vector<std::string> kClocks1 = {
	"clock_1MHz", "clock_1000kHz", "clock_1000000Hz"
};
const std::vector<size_t> kClocks1Frequency = {
	1'000'000u, 1'000'000u, 1'000'000u
};
const std::vector<std::string> kDividerGateExpressions = {
	"D0 | A2",
	"D0 & A2",
	"D1 | A5",
	"D0 | A2"
};
const std::vector<size_t> kDividerGateSize = {
	1, 2, 3, 3
};
const std::vector<DividerGateInfo> kDividerGateInfo = {
	{0, 2, kOperatorOr},
	{0, 2, kOperatorAnd},
	{1, 5, kOperatorOr}
};


// error input group
// syntax error input
const std::vector<std::string> kSyntaxErrorExpression = {
	// lex error
	"A3",
	"A2 & A3",
	"A5 = ",
	"A3 == A2",
	"A2 + A3",
	"A5 = A4 + A2",
	"A0 = D0 + A2"
	"100Hz",
	"D0 = ",
	"D1 = A2",
	"D2 = / 100",
	"D3 = A2 / 10 / 10",
	"D0 = A2 / 17.5",
	// identifier error
	"a6 = B2",
	"A4 = A4 | b13",
	"A3 = Back",
	"A4 = 1000khz",
	"A4 = 100Hz",
	"A5 = A1 & A37",
	"C4 = A35",
	"A4 = S5",
	"clock_100Hz = clock_100Hz",
	"Back = clock_20Hz",
	"back = A3",
	"A2 = A4 | A5 | S6",
	"S6 = clock_100Hz",
	"D0 = D1 | A1",
	"D0 = clock_100Hz / 100",
	"A1 = A2 | D1",
	"D2 = A2 / 0",
	"Back = A0 & D2",
	"A0 = D1 |",
	"A2 = D1 &"
};
// conflict input output group
const std::vector<std::vector<std::string>> kConflictExpression = {
	// multiple source of output
	{
		"A0 = A2 | A3",
		"A0 = A3"
	},
	{
		"Back = A0 & B5",
		"Back = A3"
	},
	{
		"C9 = A2",
		"C9 = clock_1MHz"
	},
	// multiple source of scaler
	{
		"S0 = A0 | A5",
		"S0 = A5"
	},
	// multiple source of divider
	{
		"D0 = A2 / 100",
		"D0 = A3 / 10"
	},
	// port is input and output at the same time
	{
		"A0 = A2 | A4",
		"A4 = A9"
	},
	{
		"A0 = A0 & A5"
	},
	{
		"Back = A9 | A10 | A11",
		"A11 = A12 & A13"
	},
	{
		"D0 = A0 / 100",
		"A0 = A3",
	},
	{
		"A0 = D0 | A2"
	},
	{
		"A0 = D0 & A2",
		"D0 = A3 / 100"
	},
	// input port defined as lemo and non-lemo
	{
		"A0 = A10",
		"A1 = A26"
	}
};

// correct expressions group
const std::vector<std::vector<std::string>> kExpressionGroup = {
	{
		"A1 = A3 & A7",
		"A2 = (A3 | A7) & (A11 | A31)",
		"Back = (A3 | A7) & (A11 | A31)",
		"S0 = A3",
		"S1 = A7",
		"S2 = A11",
		"S3 = A31",
		"S4 = A1",
		"S5 = A3 | A7",
		"S6 = A11 | A31",
		"S7 = A2",
		"C9 = clock_1MHz",
		"D0 = (A3 & A7) / 1000",
		"B0 = D0 | B1",
		"B2 = D0 & (B3 | B4) & B5",
		"S8 = D0",
		"S9 = B1",
		"S10 = B0",
		"S11 = B3 | B4",
		"S12 = (B3 | B4) & B5",
		"S13 = B2"
	},
	{
		"Back = (A1 & A2)",
		"S0 = A1 & A2"
	},
	{
		"A2 = (C0 & C4) | C3 | C23",
		"B2 = (C0 & C4) | C3 | C23",
		"D0 = (C0 & C4) | C3 | C23 / 1250",
		"D1 = A0 / 17",
		"A6 = D0 | (C8 | C14)",
		"C2 = D0 & ((C23 & C11) | C15)",
		"C5 = D1",
		"S0 = C0 & C4",
		"S1 = B2",
		"S2 = D0",
		"S3 = C8 | C14",
		"S4 = A6",
		"S5 = (C23 & C11) | C15",
		"S8 = D0",
		"S9 = D1",
		"S6 = C2",
		"S7 = C2",
		"C9 = clock_1MHz",
		"C10 = clock_1000kHz",
		"C12 = clock_5kHz",
		"C13 = clock_1Hz"
	}
};
const std::vector<std::vector<OutputInfo>> kFrontOutputsGroup = {
	{
		{1, kAndGatesOffset + 0},
		{2, kAndGatesOffset + 1},
		{41, kClocksOffset + 1},
		{16, kDividerGatesOffset + 0},
		{18, kDividerGatesOffset + 1}
	},
	{
	},
	{
		{2, kAndGatesOffset + 0},
		{18, kAndGatesOffset + 0},
		{6, kDividerGatesOffset + 0},
		{34, kDividerGatesOffset + 1},
		{37, kDividersOffset + 1},
		{41, kClocksOffset + 1},
		{42, kClocksOffset + 1},
		{44, kClocksOffset + 2},
		{45, kClocksOffset + 0}
	}
};
const std::vector<size_t> kFrontInputGroup = {
	0x003f'888e,
	0x6,
	0xc99d'0004'0041
};
const std::vector<size_t> kFrontOutputGroup = {
	0x0200'0005'0006,
	0x0,
	0x3624'0004'0044
};
const std::vector<size_t> kFrontLemoGroup = {
	0x8000,
	0x0,
	0x0080'0000'0000
};
const std::vector<std::vector<std::bitset<kOrBits>>> kOrGatesGroup = {
	{0x88, 0x8800, 0x18'0000},
	{},
	{0x89'0000'0000, 0x98'0000'0000, 0x4100'0000'0000, 0x8080'0000'0000, 0x8800'0000'0000}
};
const std::vector<std::vector<std::bitset<kAndBits>>> kAndGatesGroup = {
	{0x88, 0x3'0000'0000'0000, 0x4'0000'0020'0000},
	{0x6},
	{0x3'0000'0000'0000, 0x18'0000'0000'0000, 0x11'0000'0000}
};
const std::vector<size_t> kBackOutputGroup = {
	kAndGatesOffset + 1, kAndGatesOffset + 0, -1ul
};
const std::vector<std::vector<OutputInfo>> kScalersGroup = {
	{
		{0, 3},
		{1, 7},
		{2, 11},
		{3, 15},
		{4, 1},
		{5, kOrGatesOffset + 0},
		{6, kOrGatesOffset + 1},
		{7, 2},
		{8, kDividersOffset + 0},
		{9, 17},
		{10, 16},
		{11, kOrGatesOffset + 2},
		{12, kAndGatesOffset + 2},
		{13, 18}
	},
	{
		{0, kAndGatesOffset + 0}
	},
	{
		{0, kAndGatesOffset + 2},
		{1, 18},
		{2, kDividersOffset + 0},
		{3, kOrGatesOffset + 2},
		{4, 6},
		{5, kAndGatesOffset + 1},
		{8, kDividersOffset + 0},
		{9, kDividersOffset + 1},
		{6, 34},
		{7, 34}
	}
};
const std::vector<std::vector<size_t>> kClockFrequencyGroup = {
	{1, 1'000'000},
	{1},
	{1, 1'000'000, 5'000}
};
const std::vector<std::vector<DividerInfo>> kDividerGroup = {
	{
		{0, kAndGatesOffset+0, 1000}
	},
	{
	},
	{
		{0, kAndGatesOffset+0, 1250},
		{1, 0, 17}
	}
};
const std::vector<std::vector<DividerGateInfo>> kDividerGateGroup = {
	{
		{0, 17, kOperatorOr},
		{0, kAndGatesOffset+2, kOperatorAnd}
	},
	{
	},
	{
		{0, kOrGatesOffset+2, kOperatorOr},
		{0, kAndGatesOffset+1, kOperatorAnd}
	}
};



TEST(LogicParserTest, Clear) {
	LogicParser parser;
	for (size_t i = 0; i < kExpressions.size(); ++i) {
		EXPECT_EQ(parser.Parse(kExpressions[i]), 0)
			<< "Error: Parse " << i;
	}

	parser.Clear();
	EXPECT_EQ(parser.FrontOutputSize(), 0)
		<< "Error: Front output size after clear";
	for (size_t i = 0; i < kFrontIoNum; ++i) {
		EXPECT_FALSE(parser.IsFrontInput(i))
			<< "Error: Front input after clear";
		EXPECT_FALSE(parser.IsFrontOutput(i))
			<< "Error: Front output after clear";
		EXPECT_FALSE(parser.IsFrontLemo(i))
			<< "Error: Front lemo after clear";
		EXPECT_FALSE(parser.IsFrontLogicOutput(i))
			<< "Error: Front logic output after clear";
	}
	EXPECT_EQ(parser.OrGateSize(), 0)
		<< "Error: Or gate size after clear";
	EXPECT_EQ(parser.AndGateSize(), 0)
		<< "Error: And gate size after clear";
	EXPECT_EQ(parser.ClockSize(), 1)
		<< "Error: Clock size after clear";
	EXPECT_EQ(parser.ScalerSize(), 0)
		<< "Error: Scaler size after clear";
	EXPECT_FALSE(parser.BackEnable())
		<< "Error: Back enable after clear";
	EXPECT_EQ(parser.DividerSize(), 0)
		<< "Error: Divider size after clear";
	EXPECT_EQ(parser.DividerGateSize(), 0)
		<< "Error: Divider gate size after clear";
}


TEST(LogicParserTest, LeftSideLex) {
	LogicParser parser;
	for (size_t i = 0; i < kExpressions.size(); ++i) {
		std::string left, right;
		EXPECT_EQ(parser.LeftSideLex(kExpressions[i], left, right), 0)
			<< "Error: LeftSideLex failed " << i;

		EXPECT_EQ(left, kLeft[i])
			<< "Error: Left side string not equal " << i;
		EXPECT_EQ(right, kRight[i])
			<< "Error: Right side string not equal " << i;
	}
}


TEST(LogicParserTest, FrontIoForm) {
	LogicParser parser;
	for (char head = 'A'; head <= 'C'; ++head) {
		// check number form
		for (size_t i = 0; i < 32; ++i) {
			std::stringstream ss;
			ss << head << i;

			EXPECT_TRUE(parser.IsFrontIo(ss.str()))
				<< "Error: Is not in FrontIo form " << ss.str();

			EXPECT_EQ(parser.IdentifierIndex(ss.str()), (head-'A')*16 + (i%16))
				<< "Error: Identifier index " << ss.str();
		}


		// check over size inputs
		for (size_t i = 0; i < 3; ++i) {
			std::stringstream ss;
			ss << head << std::string(3, ('0' +i));
			EXPECT_FALSE(parser.IsFrontIo(ss.str()))
				<< "Error: Is in FrontIo form " << ss.str();
		}


		// check over number input
		for (size_t i = 40; i <= 70; i+=10) {
			std::stringstream ss;
			ss << head << i;
			EXPECT_FALSE(parser.IsFrontIo(ss.str()))
				<< "Error: Is in FrontIo form " << ss.str();
		}
		for (size_t i = 32; i < 40; ++i) {
			std::stringstream ss;
			ss << head << i;
			EXPECT_FALSE(parser.IsFrontIo(ss.str()))
				<< "Error: Is in FrontIo form " << ss.str();
		}


		// check letter input
		for (char c = 'A'; c <= 'Z'; ++c) {
			std::stringstream ss;
			ss << head << c;
			EXPECT_FALSE(parser.IsFrontIo(ss.str()))
				<< "Error: Is in FrontIo form " << ss.str();
		}
		for (char c = 'a'; c <= 'z'; ++c) {
			std::stringstream ss;
			ss << head << c;
			EXPECT_FALSE(parser.IsFrontIo(ss.str()))
				<< "Error: Is in FrontIo form " << ss.str();
		}
	}


	// check over head input
	for (char head = 'D'; head < 'G'; ++head) {
		for (size_t i = 0; i < 9; ++i) {
			std::stringstream ss;
			ss << head << i;
			EXPECT_FALSE(parser.IsFrontIo(ss.str()))
				<< "Error: Is in FrontIo form " << ss.str();
		}
	}


	// check lowercase head input
	for (char head = 'a'; head <= 'c'; ++head) {
		for (size_t i = 0; i < 9; ++i) {
			std::stringstream ss;
			ss << head << i;
			EXPECT_FALSE(parser.IsFrontIo(ss.str()))
				<< "Error: Is in FrontIo form " << ss.str();
		}
	}
}


TEST(LogicParserTest, LemoForm) {
	LogicParser parser;
	for (char head = 'A'; head <= 'C'; ++head) {
		// check number form
		for (size_t i = 16; i < 32; ++i) {
			std::stringstream ss;
			ss << head << i;

			EXPECT_TRUE(parser.IsLemoIo(ss.str()))
				<< "Error: Is not in LemoIo form " << ss.str();

			EXPECT_EQ(parser.IdentifierIndex(ss.str()), (head-'A')*16 + i-16)
				<< "Error: Identifier index " << ss.str();
		}


		// check over size inputs
		for (size_t i = 0; i < 3; ++i) {
			std::stringstream ss;
			ss << head << std::string(3, ('0' +i));
			EXPECT_FALSE(parser.IsLemoIo(ss.str()))
				<< "Error: Is in LemoIo form " << ss.str();
		}


		// check over number input
		for (size_t i = 40; i <= 70; i+=10) {
			std::stringstream ss;
			ss << head << i;
			EXPECT_FALSE(parser.IsLemoIo(ss.str()))
				<< "Error: Is in LemoIo form " << ss.str();
		}
		for (size_t i = 32; i < 40; ++i) {
			std::stringstream ss;
			ss << head << i;
			EXPECT_FALSE(parser.IsLemoIo(ss.str()))
				<< "Error: Is in LemoIo form " << ss.str();
		}
		for (size_t i = 0; i < 16; ++i) {
			std::stringstream ss;
			ss << head << i;
			EXPECT_FALSE(parser.IsLemoIo(ss.str()))
				<< "Error: Is in LemoIo form " << ss.str();
		}


		// check letter input
		for (char c = 'A'; c <= 'Z'; ++c) {
			std::stringstream ss;
			ss << head << c;
			EXPECT_FALSE(parser.IsLemoIo(ss.str()))
				<< "Error: Is in LemoIo form " << ss.str();
		}
		for (char c = 'a'; c <= 'z'; ++c) {
			std::stringstream ss;
			ss << head << c;
			EXPECT_FALSE(parser.IsLemoIo(ss.str()))
				<< "Error: Is in LemoIo form " << ss.str();
		}
	}


	// check over head input
	for (char head = 'D'; head < 'G'; ++head) {
		for (size_t i = 16; i < 20; ++i) {
			std::stringstream ss;
			ss << head << i;
			EXPECT_FALSE(parser.IsLemoIo(ss.str()))
				<< "Error: Is in LemoIo form " << ss.str();
		}
	}


	// check lowercase head input
	for (char head = 'a'; head <= 'c'; ++head) {
		for (size_t i = 16; i < 20; ++i) {
			std::stringstream ss;
			ss << head << i;
			EXPECT_FALSE(parser.IsLemoIo(ss.str()))
				<< "Error: Is in LemoIo form " << ss.str();
		}
	}
}


TEST(LogicParserTest, ScalerForm) {
	LogicParser parser;
	// check correct form
	for (size_t i = 0; i < kMaxScalers; ++i) {
		std::stringstream ss;
		ss << "S" << i;

		EXPECT_TRUE(parser.IsScaler(ss.str()))
			<< "Error: Is not in scaler form " << ss.str();

		EXPECT_EQ(parser.IdentifierIndex(ss.str()), kScalersOffset+i)
			<< "Error: Identifier index of " << ss.str();
	}

	// check over size form
	for (size_t i = 100; i < 400; i += 100) {
		std::stringstream ss;
		ss << "S" << i;

		EXPECT_FALSE(parser.IsScaler(ss.str()))
			<< "Error: Is in scaler form " << ss.str();
	}

	// check over number input
	for (size_t i = 32; i < 50; ++i) {
		std::stringstream ss;
		ss << "S" << i;

		EXPECT_FALSE(parser.IsScaler(ss.str()))
			<< "Error: Is in scaler form " << ss.str();
	}

	// check FrontIo form
	for (size_t i = 0; i < kLeft.size(); ++i) {
		EXPECT_FALSE(parser.IsScaler(kLeft[i]))
			<< "Error: Is in scaler form " << kLeft[i];
	}

	// check clock form
	for (size_t i = 0; i < kClocks0.size(); ++i) {
		EXPECT_FALSE(parser.IsScaler(kClocks0[i]))
			<< "Error: Is in scaler form " << kClocks0[i];
	}
	for (size_t i = 0; i < kClocks1.size(); ++i) {
		EXPECT_FALSE(parser.IsScaler(kClocks1[i]))
			<< "Error: Is in scaler form " << kClocks1[i];
	}

}


TEST(LogicParserTest, ClockForm) {
	LogicParser parser;
	for (size_t i = 0; i < kClocks0.size(); ++i) {
		EXPECT_TRUE(parser.IsClock(kClocks0[i]))
			<< "Error: Is not in clock form " << kClocks0[i];

		EXPECT_EQ(parser.ParseFrequency(kClocks0[i]), kClocks0Frequency[i])
			<< "Error: Parse Frequency " << i;
	}

	for (size_t i = 0; i < kClocks1.size(); ++i) {
		EXPECT_TRUE(parser.IsClock(kClocks1[i]))
			<< "Error: Is not in clock form " << kClocks1[i];

		EXPECT_EQ(parser.ParseFrequency(kClocks1[i]), kClocks1Frequency[i])
			<< "Error: Parse Frequency " << i;
	}

	// check error form
	for (size_t i = 0; i < kLeft.size(); ++i) {
		EXPECT_FALSE(parser.IsClock(kLeft[i]))
			<< "Error: Is in clock form " << kLeft[i];
	}

	EXPECT_TRUE(parser.SecondClock() >= kClocksOffset && parser.SecondClock() < kClocksOffset + kMaxClocks)
		<< "Erro: Second clock not found";
}


TEST(LogicParserTest, DividerForm) {
	LogicParser parser;
	// check correct form
	for (size_t i = 0; i < kMaxDividers; ++i) {
		std::stringstream ss;
		ss << "D" << i;

		EXPECT_TRUE(parser.IsDivider(ss.str()))
			<< "Error: Is not in scaler form " << ss.str();

		EXPECT_EQ(parser.IdentifierIndex(ss.str()), kDividersOffset+i)
			<< "Error: Identifier index of " << ss.str();
	}

	// check over size form
	for (size_t i = 100; i < 400; i += 100) {
		std::stringstream ss;
		ss << "D" << i;

		EXPECT_FALSE(parser.IsDivider(ss.str()))
			<< "Error: Is in divider form " << ss.str();
	}

	// check over number input
	for (size_t i = 4; i < 20; ++i) {
		std::stringstream ss;
		ss << "D" << i;

		EXPECT_FALSE(parser.IsDivider(ss.str()))
			<< "Error: Is in divider form " << ss.str();
	}

	// check FrontIo form
	for (size_t i = 0; i < kLeft.size(); ++i) {
		EXPECT_FALSE(parser.IsDivider(kLeft[i]))
			<< "Error: Is in divider form " << kLeft[i];
	}

	// check clock form
	for (size_t i = 0; i < kClocks0.size(); ++i) {
		EXPECT_FALSE(parser.IsDivider(kClocks0[i]))
			<< "Error: Is in divider form " << kClocks0[i];
	}
	for (size_t i = 0; i < kClocks1.size(); ++i) {
		EXPECT_FALSE(parser.IsDivider(kClocks1[i]))
			<< "Error: Is in divider form " << kClocks1[i];
	}
}



TEST(LogicParserTest, CheckIdentifiers) {
	LogicParser parser;
	for (size_t i = 0; i < kExpressions.size(); ++i) {
		Lexer lexer;
		std::vector<TokenPtr> tokens;
		EXPECT_EQ(lexer.Analyse(kRight[i], tokens), 0)
			<< "Error: Lexer analyse failed " << i;

		EXPECT_TRUE(parser.CheckIdentifiers(kLeft[i], tokens))
			<< "Erorr: CheckIdentifiers failed " << i;
	}
}

TEST(LogicParserTest, GenerateGates) {
	LogicParser parser;
	for (size_t i = 0; i < kExpressions.size(); ++i) {
		EXPECT_EQ(parser.Parse(kExpressions[i]), 0)
			<< "Error: Parse " << i;

		EXPECT_EQ(parser.FrontOutputSize(), kOutputSize[i])
			<< "Error: output port pair size " << i;

		EXPECT_EQ(parser.OrGateSize(), kOrGateSize[i])
			<< "Error: Or gate size " << i;

		EXPECT_EQ(parser.AndGateSize(), kAndGateSize[i])
			<< "Error: And gate size " << i;
	}

	// check port outpus
	for (size_t i = 0; i < parser.FrontOutputSize(); ++i) {
		const auto [port, source] = parser.FrontOutput(i);
		// check output port index
		EXPECT_EQ(port, kFrontOuputs[i].port)
			<< "Error: output port index " << i;
		// check identifier/gate index
		EXPECT_EQ(source, kFrontOuputs[i].source)
			<< "Error: output identifier/gate index " << i;
	}

	// check or gates
	for (size_t i = 0; i < parser.OrGateSize(); ++i) {
		EXPECT_EQ(parser.OrGate(i), kOrGates[i])
			<< "Error: or gate " << i;
	}

	// check and gates
	for (size_t i = 0; i < parser.AndGateSize(); ++i) {
		EXPECT_EQ(parser.AndGate(i), kAndGates[i])
			<< "Error: and gate " << i;
	}
}


TEST(LogicParserTest, GenerateClocks) {
	LogicParser parser;

	// clock group 0
	for (size_t i = 0; i < kClocks0.size(); ++i) {
		size_t clock_index = parser.GenerateClock(kClocks0[i]);

		// expect index in clock range
		EXPECT_GE(clock_index, kClocksOffset)
			<< "Error: Generate clock " << i;
		EXPECT_LT(clock_index, kClocksOffset+kMaxClocks)
			<< "Error: Generate clock " << i;

		// expect clock frequency correct
		EXPECT_EQ(parser.ClockFrequency(clock_index-kClocksOffset), kClocks0Frequency[i])
			<< "Error: Clock frequency " << i;
	}
	EXPECT_EQ(parser.ClockSize(), 3)
		<< "Error: Clock size";

	// clock group 1
	parser.Clear();
	for (size_t i = 0; i < kClocks1.size(); ++i) {
		size_t clock_index = parser.GenerateClock(kClocks1[i]);

		// expect index in clock range
		EXPECT_GE(clock_index, kClocksOffset)
			<< "Error: Generate clock " << i;
		EXPECT_LT(clock_index, kClocksOffset+kMaxClocks)
			<< "Error: Generate clock " << i;

		// expect clock frequency correct
		EXPECT_EQ(parser.ClockFrequency(clock_index-kClocksOffset), kClocks1Frequency[i])
			<< "Error: Clock frequency " << i;
	}
	EXPECT_EQ(parser.ClockSize(), 2)
		<< "Error: Clock size";
}


TEST(LogicParserTest, GenerateDividerGates) {
	LogicParser parser;
	for (size_t i = 0; i < kDividerGateExpressions.size(); ++i) {
		Lexer lexer;
		std::vector<TokenPtr> tokens;
		ASSERT_EQ(lexer.Analyse(kDividerGateExpressions[i], tokens), 0)
			<< "Error: Lexer analyse failed " << i;

		std::vector<TokenPtr> divider_tokens(tokens.begin(), tokens.begin()+2);

		size_t gate_index = parser.GenerateDividerGate(divider_tokens, parser.IdentifierIndex(tokens[2]->Value()));

		EXPECT_GE(gate_index, kDividerGatesOffset + 0)
			<< "Error: Generate divider gate failed " << i;
		EXPECT_LT(gate_index, kDividerGatesOffset + kMaxDividerGates)
			<< "Error: Generate divider gate failed " << i;

		EXPECT_EQ(parser.DividerGateSize(), kDividerGateSize[i])
			<< "Error: Divider gate size " << i;
	}
	for (size_t i = 0; i < parser.DividerGateSize(); ++i) {
		EXPECT_EQ(parser.DividerGate(i).divider, kDividerGateInfo[i].divider)
			<< "Error: Divider source index of divider gate " << i;

		EXPECT_EQ(parser.DividerGate(i).source, kDividerGateInfo[i].source)
			<< "Error: Other source index of divider gate " << i;

		EXPECT_EQ(parser.DividerGate(i).op_type, kDividerGateInfo[i].op_type)
			<< "Error; Operator type of divider gate " << i;
	}
}




TEST(LogicParserTest, ParseError) {
	for (const std::string &expr : kSyntaxErrorExpression) {
		LogicParser parser;
		EXPECT_EQ(parser.Parse(expr), -1)
			<< "Error: Parse success: " << expr;

		// std::cout << "Finish test case " << expr << std::endl;
	}
}


TEST(LogicParserTest, IoPortConflict) {
	for (size_t i = 0; i < kConflictExpression.size(); ++i) {
		bool success = true;
		LogicParser parser;
		for (const auto &expr : kConflictExpression[i]) {
			if (parser.Parse(expr) != 0) {
				success = false;
				break;
			}
		}
		EXPECT_FALSE(success) << "Error: Parse success " << i;
	}
}



TEST(LogicParserTest, Parse) {
	for (size_t group = 0; group < kExpressionGroup.size(); ++group) {
		LogicParser parser;
		for (const auto &expr : kExpressionGroup[group]) {
			ASSERT_EQ(parser.Parse(expr), 0)
				<< "Error: Parse group " << group << " expression " << expr;
		}


		// check results

		// front output
		EXPECT_EQ(parser.FrontOutputSize(), kFrontOutputsGroup[group].size())
			<< "Error: Front output size, group " << group;
		for (size_t i = 0; i < parser.FrontOutputSize(); ++i) {
			EXPECT_EQ(parser.FrontOutput(i).port, kFrontOutputsGroup[group][i].port)
				<< "Error: Front output port of group " << group << ", index " << i;
			EXPECT_EQ(parser.FrontOutput(i).source, kFrontOutputsGroup[group][i].source)
				<< "Error: Front output source of group " << group << ", index " << i;
		}
		for (size_t i = 0; i < kFrontIoNum; ++i) {
			EXPECT_EQ(parser.IsFrontInput(i), bool(kFrontInputGroup[group] & (1ul << i)))
				<< "Error: Front input of group " << group << ", index " << i;
			EXPECT_EQ(parser.IsFrontOutput(i), bool(kFrontOutputGroup[group] & (1ul << i)))
				<< "Error: Front output of group " << group << ", index " << i;
			EXPECT_EQ(parser.IsFrontLemo(i), bool(kFrontLemoGroup[group] & (1ul << i)))
				<< "Error: Front lemo of group " << group << ", index " << i;
		}

		// back output
		EXPECT_EQ(parser.BackEnable(), (kBackOutputGroup[group] != -1ul))
			<< "Error: Back output enable of group " << group;
		EXPECT_EQ(parser.BackSource(), kBackOutputGroup[group])
			<< "Error: Back output of group " << group;

		// or gates
		EXPECT_EQ(parser.OrGateSize(), kOrGatesGroup[group].size())
			<< "Error: Or gate size of group " << group;
		for (size_t i = 0; i < parser.OrGateSize(); ++i) {
			EXPECT_EQ(parser.OrGate(i), kOrGatesGroup[group][i])
				<< "Error: Or gate bit of group " << group << ", index " << i;
		}

		// and gates
		EXPECT_EQ(parser.AndGateSize(), kAndGatesGroup[group].size())
			<< "Error: And gate size of group " << group;
		for (size_t i = 0; i < parser.AndGateSize(); ++i) {
			EXPECT_EQ(parser.AndGate(i), kAndGatesGroup[group][i])
				<< "Error: And gate bit of group " << group << ", index " << i;
		}

		// clocks
		EXPECT_EQ(parser.ClockSize(), kClockFrequencyGroup[group].size())
			<< "Error: Clock size of group " << group;
		for (size_t i = 0; i < parser.ClockSize(); ++i) {
			EXPECT_EQ(parser.ClockFrequency(i), kClockFrequencyGroup[group][i])
				<< "Error: Clock frequency of group " << group << ", index " << i;
		}

		// scalers
		EXPECT_EQ(parser.ScalerSize(), kScalersGroup[group].size())
			<< "Error: Scaler size of group " << group;
		for (size_t i = 0; i < parser.ScalerSize(); ++i) {
			EXPECT_EQ(parser.Scaler(i).port, kScalersGroup[group][i].port)
				<< "Error: Scaler port of group " << group << ", index " << i;
			EXPECT_EQ(parser.Scaler(i).source, kScalersGroup[group][i].source)
				<< "Error: Scaler source of group " << group << ", index " << i;
		}


		// dividers
		EXPECT_EQ(parser.DividerSize(), kDividerGroup[group].size())
			<< "Error: Divider size of group " << group;
		for (size_t i = 0; i < parser.DividerSize(); ++i) {
			EXPECT_EQ(parser.Divider(i).port, kDividerGroup[group][i].port)
				<< "Error: Divider port of group " << group << ", index " << i;
			EXPECT_EQ(parser.Divider(i).source, kDividerGroup[group][i].source)
				<< "Error: Divider source of group " << group << ", index " << i;
			EXPECT_EQ(parser.Divider(i).divisor, kDividerGroup[group][i].divisor)
				<< "Error: Divider divisor of group " << group << ", index " << i;
		}


		// divider gates
		EXPECT_EQ(parser.DividerGateSize(), kDividerGateGroup[group].size())
			<< "Error: Divider gate size of group " << group;
		for (size_t i = 0; i < parser.DividerGateSize(); ++i) {
			EXPECT_EQ(parser.DividerGate(i).divider, kDividerGateGroup[group][i].divider)
				<< "Error: Divider gate's divider source of group " << group << " index " << i;
			EXPECT_EQ(parser.DividerGate(i).source, kDividerGateGroup[group][i].source)
				<< "Error: Divider gate's other source of group " << group << ", index " << i;
			EXPECT_EQ(parser.DividerGate(i).op_type, kDividerGateGroup[group][i].op_type)
				<< "Error: Dvider gate's operator of group " << group << ", index " << i;
		}
	}
}