#include "syntax/logic_comparer.h"

#include <vector>
#include <map>

#include <gtest/gtest.h>

using namespace ecc;


// expressions
const std::vector<std::vector<std::string>> kExpressions = {
	{"A", "A"},
	{"B", "A"},
	{"A|B", "B|A"},
	{"A&B", "B & A"},
	{"A|A", "A"},
	{"A&A", "A|A"},
	{"A & B", "A | B"},
	{"(A & B)", "A & B"},
	{"(A & B) | C", "(A | C) & (B | C)"},
	{"(A | B) & (C | D)", "(A&C) | (A&D) | (B&C) | (B&D)"},
	{"(A|B)&B", "B"},
	{"(A&B)|B", "B"}
};
// compare result
const std::vector<bool> kResult = {
	true,
	false,
	true,
	true,
	true,
	true,
	false,
	true,
	true,
	true,
	true,
	true
};



const std::vector<std::pair<std::string, std::string>> kTimeExpressions = {
	{
		"(A|B|C) & (C|D|E) & (A|F|G) & H", 
		"(A&C&H) | (A&D&H) | (A&E&H) | (C&F&H) | (B&D&F&H) | (B&E&F&H) | (C&G&H) | (B&D&G&H) | (B&E&G&H)"
	},
	{
		"((A|B|C)&(D|E|F)&(G|H)) | ((I|J)&(K&L))",
		"(A | B | C | I | J) & (D | E | F | I | J) & (G | H | I | J) & (A | B | C | K)"
		"& (D | E | F | K) & (G | H | K) & (A | B | C | L) & (D | E | F | L) & (G | H | L)"
	},
	{
		"( ((A&B&C)|D|(E&F)) & (G|H) & ((I&J&K)|(L&M)) ) | ((P|Q)&R)",
		"(A | D | E | P | Q) & (B | D | E | P | Q) & (C | D | E | P | Q) & (A | D | F | P | Q)"
		"& (B | D | F | P | Q) & (C | D | F | P | Q) & (G | H | P | Q) & (I | L | P | Q)"
		"& (J | L | P | Q) & (K | L | P | Q) & (I | M | P | Q) & (J | M | P | Q) & (K | M | P | Q)"
		"& (A | D | E | R) & (B | D | E | R) & (C | D | E | R) & (A | D | F | R) & (B | D | F | R)"
		"& (C | D | F | R) & (G | H | R) & (I | L | R) & (J | L | R) & (K | L | R) & (I | M | R)"
		"& (J | M | R) & (K | M | R)"
	},
	{
		"(A0&A3) & ( (A4|A7|A8|A12|A13|A15) | (B0|B3|B4|B7|B8|B11) | (C0|C3) | (((C7&C11)|C4|C8) & C12) )",
		"(A4 | A7 | A8 | A12 | A13 | A15 | B0 | B3 | B4 | B7 | B8 | B11 | C0 | C3 | C7 | C4 | C8)"
		"& (A4 | A7 | A8 | A12 | A13 | A15 | B0 | B3 | B4 | B7 | B8 | B11 | C0 | C3 | C11 | C4 | C8)"
		"& (A4 | A7 | A8 | A12 | A13 | A15 | B0 | B3 | B4 | B7 | B8 | B11 | C0 | C3 | C12) & A0 & A3"
	},
	{
		"( ((A&B&C)|D|(E&F)) & (G|H) & ((I&J&K)|(L&M)) ) | ( ((N&O)|(P&Q&R)) & S & T)",
		"(A | D | E | N | P) & (B | D | E | N | P) & (C | D | E | N | P) & (A | D | F | N | P)"
		"& (B | D | F | N | P) & (C | D | F | N | P) & (G | H | N | P) & (I | L | N | P)"
		"& (J | L | N | P) & (K | L | N | P) & (I | M | N | P) & (J | M | N | P) & (K | M | N | P)"
		"& (A | D | E | O | P) & (B | D | E | O | P) & (C | D | E | O | P) & (A | D | F | O | P)"
		"& (B | D | F | O | P) & (C | D | F | O | P) & (G | H | O | P) & (I | L | O | P)"
		"& (J | L | O | P) & (K | L | O | P) & (I | M | O | P) & (J | M | O | P) & (K | M | O | P)"
		"& (A | D | E | N | Q) & (B | D | E | N | Q) & (C | D | E | N | Q) & (A | D | F | N | Q)"
		"& (B | D | F | N | Q) & (C | D | F | N | Q) & (G | H | N | Q) & (I | L | N | Q)"
		"& (J | L | N | Q) & (K | L | N | Q) & (I | M | N | Q) & (J | M | N | Q)"
		"& (K | M | N | Q) & (A | D | E | O | Q) & (B | D | E | O | Q) & (C | D | E | O | Q)"
		"& (A | D | F | O | Q) & (B | D | F | O | Q) & (C | D | F | O | Q) & (G | H | O | Q)"
		"& (I | L | O | Q) & (J | L | O | Q) & (K | L | O | Q) & (I | M | O | Q) & (J | M | O | Q)"
		"& (K | M | O | Q) & (A | D | E | N | R) & (B | D | E | N | R) & (C | D | E | N | R)"
		"& (A | D | F | N | R) & (B | D | F | N | R) & (C | D | F | N | R) & (G | H | N | R)"
		"& (I | L | N | R) & (J | L | N | R) & (K | L | N | R) & (I | M | N | R) & (J | M | N | R)"
		"& (K | M | N | R) & (A | D | E | O | R) & (B | D | E | O | R) & (C | D | E | O | R)"
		"& (A | D | F | O | R) & (B | D | F | O | R) & (C | D | F | O | R) & (G | H | O | R)"
		"& (I | L | O | R) & (J | L | O | R) & (K | L | O | R) & (I | M | O | R) & (J | M | O | R)"
		"& (K | M | O | R) & (A | D | E | S) & (B | D | E | S) & (C | D | E | S) & (A | D | F | S)"
		"& (B | D | F | S) & (C | D | F | S) & (G | H | S) & (I | L | S) & (J | L | S) & (K | L | S)"
		"& (I | M | S) & (J | M | S) & (K | M | S) & (A | D | E | T) & (B | D | E | T) & (C | D | E | T)"
		"& (A | D | F | T) & (B | D | F | T) & (C | D | F | T) & (G | H | T) & (I | L | T) & (J | L | T)"
		"& (K | L | T) & (I | M | T) & (J | M | T) & (K | M | T)"
	}
};


TEST(LogicComparerTest, Compare) {
	for (size_t i = 0; i < kExpressions.size(); ++i) {
		std::cout << "-----------"<< i << "----------" << std::endl;
		LogicComparer comparer;
		EXPECT_EQ(comparer.Compare(kExpressions[i][0], kExpressions[i][1]), kResult[i])
			<< "index " << i;
	}
}


TEST(LogicComparerTest, Time) {
	std::vector<double> time_cost;
	size_t index = 0;
	for (const auto &expr_pair : kTimeExpressions) {
		LogicComparer comparer;

		auto start = std::chrono::high_resolution_clock::now();
		EXPECT_TRUE(comparer.Compare(expr_pair.first, expr_pair.second))
			<< "Error: not euqal in case " << index;
		auto stop = std::chrono::high_resolution_clock::now();

		std::cout << "finish case " << index << std::endl;
		time_cost.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count());
		++index;
	}

	for (size_t i = 0; i < time_cost.size(); ++i) {
		std::cout << "case " << i << " cost " << time_cost[i] << " ms" << std::endl;
	}
}