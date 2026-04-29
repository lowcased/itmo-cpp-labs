#include "lexer.h"
#include "ast.h"
#include <cctype>
#include <cstdlib>

std::unordered_map<std::string, TokenType> keywords = {
    {"and", tok_and}, {"or", tok_or}, {"not", tok_not},
    {"function", tok_func}, {"end", tok_end}, {"while", tok_while},
    {"for", tok_for}, {"if", tok_if}, {"then", tok_then}, {"else", tok_else},
    {"in", tok_in}, {"break", tok_break}, {"continue", tok_continue},
    {"return", tok_ret}, {"true", tok_bool}, {"false", tok_bool},
    {"nil", tok_null}
};

std::ostream& operator<<(std::ostream& os, const NullType&) { return os << "nil"; }

bool isLetter(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}
bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool truthy(const Value& v) {
    if (std::holds_alternative<NullType>(v)) {
        return false;
    }
    if (std::holds_alternative<double>(v)) {
        return std::get<double>(v) != 0.0;
    }
    if (std::holds_alternative<bool>(v)) {
        return std::get<bool>(v);
    }
    if (std::holds_alternative<std::string>(v)) {
        return !std::get<std::string>(v).empty();
    }
    if (std::holds_alternative<std::shared_ptr<ListType>>(v)) {
        return !std::get<std::shared_ptr<ListType>>(v)->elems.empty();
    }
    return true;
}

Value* findVar(const std::string& n) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto f = (*it)->find(n);
        if (f != (*it)->end()) {
            return &f->second;
        }
    }
    return nullptr;
}

Value& getVar(const std::string& n) {
    if (auto* p = findVar(n)) {
        return *p;
    }
    return (*scopes.front())[n];
}

void setVar(const std::string& n, Value v) {
    if (auto* p = findVar(n)) {
        *p = std::move(v);
    }
    else {
        (*scopes.back())[n] = std::move(v);
    }
}

void LookupIdent(Token& token, std::string& identifier) {
    if (identifier == "true") {
        token.type = tok_bool;
        token.value = true;
    }
    else if (identifier == "false") {
        token.type = tok_bool;
        token.value = false;
    }
    else if (identifier == "nil") {
        token.type = tok_null;
        token.value = NullType{};
    }
    else if (keywords.contains(identifier)) {
        token.type = keywords[identifier];
    }
    else {
        token.type = tok_id;
        token.value = identifier;
    }
}

Lexer::Lexer(std::string& in) : input(in) {
    readChar();
}

Token Lexer::NextToken() {
    Token token;
    while (true) {
        skipWhitespace();
        if (ch == '/' && peekChar() == '/') {
            while (ch && ch != '\n') {
                readChar();
            }
            if (ch == '\n') {
                token.type = tok_eol;
                readChar();
                return token;
            }
            continue;
        }
        break;
    }

    switch (ch) {
    case '=':
        if (peekChar() == '=') {
            readChar();
            token.type = tok_eq;
        }
        else {
            token.type = tok_assign;
        }
        readChar();
        break;
    case '!':
        if (peekChar() == '=') {
            readChar();
            token.type = tok_not_eq;
        }
        else {
            token.type = tok_ill;
        }
        readChar();
        break;
    case '+':
        if (peekChar() == '=') {
            readChar();
            token.type = tok_plus_assign;
        }
        else {
            token.type = tok_plus;
        }
        readChar();
        break;
    case '-':
        if (peekChar() == '=') {
            readChar();
            token.type = tok_minus_assign;
        }
        else {
            token.type = tok_minus;
        }
        readChar();
        break;
    case '*':
        if (peekChar() == '=') {
            readChar();
            token.type = tok_mult_assign;
        }
        else {
            token.type = tok_mult;
        }
        readChar();
        break;
    case '/':
        if (peekChar() == '=') {
            readChar(); token.type = tok_div_assign;
        }
        else {
            token.type = tok_slash;
        }
        readChar();
        break;
    case '%':
        if (peekChar() == '=') {
            readChar(); token.type = tok_rem_assign;
        }
        else {
            token.type = tok_rem;
        }
        readChar();
        break;
    case '^':
        if (peekChar() == '=') {
            readChar();
            token.type = tok_pow_assign;
        }
        else {
            token.type = tok_pow;
        }
        readChar();
        break;
    case '>':
        if (peekChar() == '=') {
            readChar();
            token.type = tok_greater_or_eq;
        }
        else {
            token.type = tok_greater;
        }
        readChar();
        break;
    case '<':
        if (peekChar() == '=') {
            readChar();
            token.type = tok_less_or_eq;
        }
        else {
            token.type = tok_less;
        }
        readChar();
        break;
    case ',':
        token.type = tok_comma;
        readChar();
        break;
    case ':':
        token.type = tok_colon;
        readChar();
        break;
    case '(':
        token.type = tok_lparen;
        readChar();
        break;
    case ')':
        token.type = tok_rparen;
        readChar();
        break;
    case '[':
        token.type = tok_lbracket;
        readChar();
        break;
    case ']':
        token.type = tok_rbracket;
        readChar();
        break;
    case '\n':
        token.type = tok_eol;
        readChar();
        break;
    case '\"':
        token.type = tok_str;
        token.value = readString();
        break;
    case 0:
        token.type = tok_eof;
        break;
    default:
        if (isLetter(ch)) {
            std::string identifier = readIdentifier();
            LookupIdent(token, identifier);
        }
        else if (isDigit(ch)) {
            token.type = tok_num;
            token.value = readNumber();
        }
        else {
            token.type = tok_ill;
            readChar();
        }
    }
    return token;
}

void Lexer::readChar() {
    if (read_position >= input.length()) {
        ch = 0;
    }
    else {
        ch = input[read_position];
    }
    position = read_position;
    ++read_position;
}

char Lexer::peekChar() {
    if (read_position >= input.length()) {
        return 0;
    }
    else {
        return input[read_position];
    }
}

double Lexer::readNumber() {
    int begin = position;     
    readChar();                
    while (isDigit(ch))        
        readChar();
    if (ch == '.') {         
        readChar();
        while (isDigit(ch))
            readChar();
    }
    if (ch == 'e' || ch == 'E') {           
        char next = peekChar();
        if (next == '+' || next == '-' || isDigit(next)) {
            readChar();                    
            if (ch == '+' || ch == '-')     
                readChar();
            while (isDigit(ch))
                readChar();
        }
    }
    return std::stod(input.substr(begin, position - begin));
}

std::string Lexer::readIdentifier() {
    int begin = position;  
    readChar();          
    while (isLetter(ch) || isDigit(ch))
        readChar();         
    return input.substr(begin, position - begin);
}

std::string Lexer::readString() {
    readChar();
    std::string str = "";
    while (ch != '\"' || input[position - 1] == '\\') {
        if (ch == '\\') {
            char peek = peekChar();
            switch (peek) {
            case '\'':
                readChar();
                ch = '\'';
                break;
            case '\"':
                readChar();
                ch = '\"';
                break;
            case '\?':
                readChar();
                ch = '\?';
                break;
            case '\\':
                readChar();
                break;
            case 'a':
                readChar();
                ch = '\a';
                break;
            case 'b':
                readChar();
                ch = '\b';
                break;
            case 'f':
                readChar();
                ch = '\f';
                break;
            case 'n':
                readChar();
                ch = '\n';
                break;
            case 'r':
                readChar();
                ch = '\r';
                break;
            case 't':
                readChar();
                ch = '\t';
                break;
            case 'v':
                readChar();
                ch = '\v';
                break;
            }
        }
        str += ch;
        readChar();
    }
    readChar();
    return str;
}

void Lexer::skipWhitespace() {
    while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
        readChar();
}