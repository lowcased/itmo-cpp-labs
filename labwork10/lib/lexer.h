#pragma once
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>

enum TokenType {
	tok_ill, tok_eof, tok_eol, tok_id, tok_num, tok_str, tok_list, tok_func,
	tok_ret, tok_end, tok_null, tok_plus, tok_minus, tok_mult, tok_slash,
	tok_rem, tok_pow, tok_eq, tok_not_eq, tok_less, tok_greater,
	tok_less_or_eq, tok_greater_or_eq, tok_and, tok_or, tok_not, tok_assign,
	tok_plus_assign, tok_minus_assign, tok_mult_assign, tok_div_assign,
	tok_rem_assign, tok_pow_assign, tok_comma, tok_lparen, tok_rparen,
	tok_lbracket, tok_rbracket, tok_while, tok_for, tok_if, tok_then, tok_else,
	tok_in, tok_break, tok_continue, tok_comment, tok_bool, tok_colon
};

struct NullType {};
std::ostream& operator<<(std::ostream& os, const NullType&);

struct Function;
struct ListType;
class  AST;

using Value = std::variant<double, bool, std::string, std::shared_ptr<ListType>, std::shared_ptr<Function>, NullType>;

inline std::ostream* itmo_out = &std::cout;

struct Token {
	TokenType type;
	Value     value;
};

extern std::unordered_map<std::string, TokenType> keywords;

extern std::vector<std::string> call_stack;
extern std::unordered_map<std::string, Value> variables;
extern std::vector<std::unordered_map<std::string, Value>*> scopes;

bool truthy(const Value& v);
Value* findVar(const std::string& n);
Value& getVar(const std::string& n);
void setVar(const std::string& n, Value v);

bool isLetter(char ch);
bool isDigit(char ch);
void LookupIdent(Token& token, std::string& identifier);

class Lexer {
public:
	explicit Lexer(std::string& in);
	Token NextToken();
private:
	std::string input;
	int  position = 0;
	int  read_position = 0;
	char ch = 0;

	void readChar();
	char peekChar();
	double readNumber();
	std::string readIdentifier();
	std::string readString();
	void skipWhitespace();
};