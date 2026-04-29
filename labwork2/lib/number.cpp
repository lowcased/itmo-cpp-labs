#include "number.h"

uint239_t FromInt(uint32_t value, uint32_t shift)
{
	uint239_t number;
	uint32_t bit = 1;
	for (int i = 0; i < 35; i++) {
		number.data[i] = 0;
	}
	for (uint32_t i = 0; i < 32; i++)
	{
		number.data[i / 7] |= (value & bit) >> 7 * (i / 7);
		bit <<= 1;
	}
	number = ConvertToNumWithShift(number, shift);
	return number;
}

uint239_t FromString(char* input, uint32_t shift)
{
	char* str = new char[sizeof(input)];
	for (int i = 0; i < sizeof(input); i++) {
		str[i] = input[i];
	}
	uint239_t number;
	for (int i = 0; i < 35; i++) {
		number.data[i] = 0;
	}
	uint32_t count = 0;
	while (!IsZero(str) && count < 239) {
		number.data[count / 7] |= ((str[strlen(str) - 1] - '0')%2) << count % 7;
		str = DivideByTwo(str);
		count++;
	}

	number = ConvertToNumWithShift(number, shift);
	return number;
}

uint239_t operator+(const uint239_t& lhs, const uint239_t& rhs)
{
	uint239_t lnum = lhs;
	uint239_t rnum = rhs;
	uint32_t lshift = GetShift(lhs);
	uint32_t rshift = GetShift(rhs);
	uint32_t sumshift = lshift + rshift;
	lnum = ConvertToNumWithoutShift(lnum, lshift);
	rnum = ConvertToNumWithoutShift(rnum, rshift);
	char lbits[239];
	char rbits[239];
	Convert239ToChar(lnum, lbits);
	Convert239ToChar(rnum, rbits);
	char sum[239];
	SumCharBits(sum, lbits, rbits);
	uint239_t result = ConvertCharTo239(sum);
	result = ConvertToNumWithShift(result, sumshift);
	return result;
}

uint239_t operator-(const uint239_t& lhs, const uint239_t& rhs)
{
	uint239_t lnum = lhs;
	uint239_t rnum = rhs;
	uint32_t lshift = GetShift(lhs);
	uint32_t rshift = GetShift(rhs);
	long long difshift;
	if (lshift >= rshift) {
		difshift = lshift - rshift;
	}
	else {
		difshift = ((long long)2 << 35 + lshift - rshift) % 245;
	}
	lnum = ConvertToNumWithoutShift(lnum, lshift);
	rnum = ConvertToNumWithoutShift(rnum, rshift);
	char lbits[239];
	char rbits[239];
	Convert239ToChar(lnum, lbits);
	Convert239ToChar(rnum, rbits);
	char dif[239];
	DifChar(dif, lbits, rbits);
	uint239_t result = ConvertCharTo239(dif);
	result = ConvertToNumWithShift(result, difshift);
	return result;
}

uint239_t operator*(const uint239_t& lhs, const uint239_t& rhs)
{
	uint239_t lnum = lhs;
	uint239_t rnum = rhs;
	uint32_t lshift = GetShift(lhs);
	uint32_t rshift = GetShift(rhs);
	uint32_t sumshift = lshift + rshift;
	lnum = ConvertToNumWithoutShift(lnum, lshift);
	rnum = ConvertToNumWithoutShift(rnum, rshift);
	char lbits[239];
	char rbits[239];
	Convert239ToChar(lnum, lbits);
	Convert239ToChar(rnum, rbits);
	char product[239];
	for (int i = 0; i < 239; i++) {
		product[i] = '0';
	}
	for (int i = 0; i < 239; i++) {
		if (rbits[i] == '1') {
			SumCharBits(product, product, lbits);
		}
		ShiftChar(lbits);
	}
	uint239_t result = ConvertCharTo239(product);
	result = ConvertToNumWithShift(result, sumshift);
	return result;
}

uint239_t operator/(const uint239_t& lhs, const uint239_t& rhs)
{
	uint239_t lnum = lhs;
	uint239_t rnum = rhs;
	long long lshift = GetShift(lhs);
	long long rshift = GetShift(rhs);
	long long difshift;
	if (lshift >= rshift) {
		difshift = lshift - rshift;
	}
	else {
		difshift = ((long long)2 << 35 + lshift - rshift) % 245;
	}
	uint239_t quotient;
	uint239_t one;
	for (int i = 0; i < 35; i++) {
		quotient.data[i] = 0;
		one.data[i] = 0;
	}
	one.data[0] = 1;
	while (IsGreater(lnum, rnum)) {
		lnum = lnum - rnum;
		quotient = quotient + one;
	}
	return quotient;
}

bool operator==(const uint239_t& lhs, const uint239_t& rhs)
{
	uint239_t lnum = ConvertToNumWithoutShift(lnum, GetShift(lnum));
	uint239_t rnum = ConvertToNumWithoutShift(rnum, GetShift(rnum));
	for (int i = 34; i >= 0; i--) {
		if (lhs.data[i] != rhs.data[i]) {
			return false;
		}
	}
	return true;
}

bool operator!=(const uint239_t& lhs, const uint239_t& rhs)
{
	if (lhs == rhs)
		return false;
	return true;
}

std::ostream& operator<<(std::ostream& stream, const uint239_t& value)
{
	uint239_t number = ConvertToNumWithoutShift(value, GetShift(value));
	char* decimal=new char[72];
	char* bits=new char[239];
	SetToZero(decimal,72);
	SetToZero(bits,239);
	Convert239ToChar(number, bits);
	char* poww=new char[72];
	SetToZero(poww, 72);
	poww[0] = '1';
	char* summa=new char[72];
	SetToZero(summa,72);
	for (int i = 0; i < 239; i++) {
		if (bits[i] == '1') {
			SumChar(summa,decimal, poww);
			for (int i = 0; i < 72; i++) {
				decimal[i] = summa[i];
			}
		}
		SumChar(summa,poww, poww);
		for (int i = 0; i < 72; i++) {
			poww[i] = summa[i];
		}
	}
	int i = 71;
	while (i>0 && decimal[i] == '0') {
		i--;
	}
	char res[73];
	SetToZero(res,73);
	for (int j=0; j<=i; j++) {
		res[j] = decimal[i-j];
	}
	res[i+1] = '\0';
	return stream << res;
}

uint64_t GetShift(const uint239_t& value)
{
	uint64_t shift = 0;
	for (int i = 0; i < 35; i++) {
		shift += ((value.data[i] >> 7) & 1) * (1 << i);
	}
	return shift;
}

uint239_t ConvertToNumWithoutShift(uint239_t number, uint32_t shift) {
	shift %= 245;
	uint32_t byteShift = shift / 7;
	uint32_t bitShift = shift % 7;
	uint239_t result;
	for (int i = 0; i < 35; i++) {
		number.data[i] &= 0b01111111;
	}
	result.data[0] = number.data[byteShift] >> bitShift;
	for (int i = 1; i < 35; i++) {
		result.data[i] = number.data[(i + byteShift) % 35] >> bitShift;
		result.data[i - 1] |= number.data[(i + byteShift) % 35] << (7 - bitShift);
	}
	return result;
}

uint239_t ConvertToNumWithShift(uint239_t number, uint32_t shift) {
	shift %= 245;
	uint32_t byteShift = shift / 7;
	uint32_t bitShift = shift % 7;
	uint239_t result;
	for (int i = 0; i < 35; i++) {
		result.data[i] = 0;
	}
	for (int i = 0; i < 35; i++) {
		result.data[(i + byteShift) % 35] |= (number.data[i] << bitShift) & 0b01111111;
		result.data[(i + 1 + byteShift) % 35] |= (number.data[i] >> (7 - bitShift));
	}
	uint32_t bit = 1;
	for (int i = 0; i < 32; i++) {
		result.data[i] |= ((shift & bit) >> i) << 7;
		bit <<= 1;
	}
	return result;
}

char* DivideByTwo(const char* num) {
	uint8_t buf = 0;
	char* quotient = new char[strlen(num)+1];
	quotient[strlen(num)] = '\0';
	uint32_t length = 0;
	for (int i = 0; i < strlen(quotient); i++) {
		buf = buf * 10 + num[i] - '0';
		if (buf >= 2) {
			quotient[length] = buf / 2+'0';
			buf %= 2;
			length++;
		}
		else {
			quotient[length] = '0';
			length++;
		}
	}
	if (quotient[0] == '0' && strlen(quotient)!=1) {
		for (int i = 0; i < strlen(quotient)-1; i++) {
			quotient[i] = quotient[i + 1];
		}
		quotient[strlen(quotient) - 1] = '\0';
	}
	if (quotient[strlen(num) - 1] < '0' || quotient[strlen(num) - 1]>'9') {
		quotient[strlen(num) - 1] = '\0';
	}
	return quotient;
}

bool IsZero(char* str) {
	for (int i = strlen(str) - 1; i >= 0; i--) {
		if (str[i] != '0') return false;
	}
	return true;
}

uint8_t ReadNextBit(uint239_t number, int& cur_byte, int& cur_bit) {
	cur_bit--;
	if (cur_bit < 0) {
		cur_byte--;
		if (cur_byte < 0) {
			return 2; //конец числа
		}
	}
	return (number.data[cur_byte] >> cur_bit) & 1;
}

void Convert239ToChar(uint239_t number, char* result) {
	for (int i = 0; i < 34; i++) {
		int bit = 1;
		for (int j = 0; j < 7; j++) {
			result[i*7 + j] = '0'+((number.data[i] & bit) >> j);
			bit <<= 1;
		}
	}
	result[238] = '0' + ((number.data[34] & 0b00000001));
}

uint239_t ConvertCharTo239(char* bits) {
	uint239_t number;
	for (int i = 0; i < 35; i++) {
		number.data[i] = 0;
	}
	for (int i = 0; i < 34; i++) {
		for (int j = 0; j < 7; j++) {
			number.data[i] |= (bits[7 * i + j] - '0') << j;
		}
	}
	number.data[34] |= (bits[238] - '0') & 1;
	return number;
}

void SumCharBits(char* sum, char* lbits, char* rbits) {
	char overflow = '0';
	for (int i = 0; i < 239; i++) {
		if (lbits[i] == '0' && rbits[i] == '0') {
			sum[i] = overflow;
			overflow = '0';
		}
		else if (lbits[i] == '0' && rbits[i] == '1' && overflow == '0') {
			sum[i] = '1';
		}
		else if (lbits[i] == '1' && rbits[i] == '0' && overflow == '0') {
			sum[i] = '1';
		}
		else if (lbits[i] == '0' && rbits[i] == '1' && overflow == '1') {
			sum[i] = '0';
		}
		else if (lbits[i] == '1' && rbits[i] == '0' && overflow == '1') {
			sum[i] = '0';
		}
		else if (lbits[i] == '1' && rbits[i] == '1' && overflow == '0') {
			sum[i] = '0';
			overflow = '1';
		}
		else if (lbits[i] == '1' && rbits[i] == '1' && overflow == '1') {
			sum[i] = '1';
		}
	}
}

void DifChar(char* dif, char* lbits, char* rbits) {
	for (int i = 0; i < 239; i++) {
		if (lbits[i] == '0' && rbits[i] == '0') {
			dif[i] = '0';
		}
		else if (lbits[i] == '0' && rbits[i] == '1') {
			dif[i] = '1';
			int j = i + 1;
			while (j < 238 && lbits[j] == '0') {
				lbits[j] = '1';
				j++;
			}
			lbits[j] = '0';
		}
		else if (lbits[i] == '1' && rbits[i] == '0') {
			dif[i] = '1';
		}
		else if (lbits[i] == '1' && rbits[i] == '1') {
			dif[i] = '0';
		}
	}
}

void ShiftChar(char* bits) {
	char temp1 = bits[0], temp2;
	for (int i = 0; i < 238; i++) {
		temp2 = bits[i + 1];
		bits[i + 1] = temp1;
		temp1 = temp2;
	}
}

bool IsGreater(uint239_t num1, uint239_t num2) {
	for (int i = 34; i >= 0; i--) {
		if (num1.data[i] > num2.data[i]) {
			return true;
		}
	}
	return false;
}

void SumChar(char* sum, char* num1, char* num2) {
	uint8_t overflow = 0;
	for (int i = 0; i < 72; i++) {
		sum[i] = char(int((num1[i] - '0' + num2[i] - '0' + overflow) % 10) + '0');
		if (num1[i] - '0' + num2[i] - '0' + overflow >= 10) {
			overflow = 1;
		}
		else {
			overflow = 0;
		}
	}
}

void SetToZero(char a[], int n) {
	for (int i = 0; i < n; i++) {
		a[i] = '0';
	}
}
