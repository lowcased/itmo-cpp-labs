#pragma once
#include <cinttypes>
#include <iostream>


struct uint239_t {
	uint8_t data[35];
};

static_assert(sizeof(uint239_t) == 35, "Size of uint239_t must be no higher than 35 bytes");

uint239_t FromInt(uint32_t value, uint32_t shift);

uint239_t FromString(const char* str, uint32_t shift);

uint239_t operator+(const uint239_t& lhs, const uint239_t& rhs);

uint239_t operator-(const uint239_t& lhs, const uint239_t& rhs);

uint239_t operator*(const uint239_t& lhs, const uint239_t& rhs);

uint239_t operator/(const uint239_t& lhs, const uint239_t& rhs);

bool operator==(const uint239_t& lhs, const uint239_t& rhs);

bool operator!=(const uint239_t& lhs, const uint239_t& rhs);

std::ostream& operator<<(std::ostream& stream, const uint239_t& value);

uint64_t GetShift(const uint239_t& value);

uint239_t ConvertToNumWithoutShift(uint239_t number, uint32_t shift);

uint239_t ConvertToNumWithShift(uint239_t number, uint32_t shift);

bool IsZero(char* str);

char* DivideByTwo(const char* num);

void Convert239ToChar(uint239_t number, char* result);

uint239_t ConvertCharTo239(char* bits);

void SumCharBits(char* sum, char* lbits, char* rbits);

void DifChar(char* dif, char* lbits, char* rbits);

void ShiftChar(char* bits);

void SumChar(char* summa,char* num1, char* num2);

bool IsGreater(uint239_t num1, uint239_t num2);

void SetToZero(char a[], int n);
