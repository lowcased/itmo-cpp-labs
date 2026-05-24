#pragma once

#include <memory>
#include <vector>
#include "lexer.h"
#include "ast.h"

class Parser {
public:
    explicit Parser(Lexer& lex);
    std::unique_ptr<AST> parse();

private:
    std::unique_ptr<AST> condition();
    std::unique_ptr<AST> while_cycle();
    std::unique_ptr<AST> for_cycle();
    std::unique_ptr<AST> list();
    std::unique_ptr<AST> func();
    std::unique_ptr<AST> compound_statement();
    std::vector<std::unique_ptr<AST>> statement_list(TokenType end1,
                                                    TokenType end2 = tok_end);
    std::unique_ptr<AST> statement();
    std::unique_ptr<AST> assignment();
    std::unique_ptr<AST> expr();
    std::unique_ptr<AST> parseOr();
    std::unique_ptr<AST> parseAnd();
    std::unique_ptr<AST> parseEq();
    std::unique_ptr<AST> parseCmp();
    std::unique_ptr<AST> parseAdd();
    std::unique_ptr<AST> parseMul();
    std::unique_ptr<AST> parsePow();
    std::unique_ptr<AST> postfix();
    std::unique_ptr<AST> factor();

    void eat(TokenType expected);

    Lexer& lexer;
    Token  cur_tok;
    Token  peek_tok;
};