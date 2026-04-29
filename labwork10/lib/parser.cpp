#include "parser.h"

Parser::Parser(Lexer& _lexer) : lexer(_lexer) {
    cur_tok = lexer.NextToken();
    peek_tok = lexer.NextToken();
}

void Parser::eat(TokenType expected) {
    if (cur_tok.type == expected) {
        cur_tok = peek_tok;
        peek_tok = lexer.NextToken();
    }
}

std::unique_ptr<AST> Parser::parse() {
    return compound_statement();
}

std::unique_ptr<AST> Parser::condition() {
    eat(tok_if);
    auto cond = expr();
    eat(tok_then);
    auto if_body = statement_list(tok_else, tok_end);

    if (cur_tok.type == tok_else && peek_tok.type == tok_if) {
        eat(tok_else);
        auto elseif = condition();
        std::vector<std::unique_ptr<AST>> elseif_vec;
        elseif_vec.emplace_back(std::move(elseif));
        return std::make_unique<If>(std::move(cond),std::move(if_body),std::move(elseif_vec));
    }
    if (cur_tok.type == tok_else) {
        eat(tok_else);
        auto else_body = statement_list(tok_end);
        eat(tok_end); eat(tok_if);
        return std::make_unique<If>(std::move(cond),
                                    std::move(if_body),
                                    std::move(else_body));
    }
    eat(tok_end); eat(tok_if);
    return std::make_unique<If>(std::move(cond),
                                std::move(if_body));
}

std::unique_ptr<AST> Parser::while_cycle() {
    eat(tok_while);
    auto cond = expr();
    auto body = statement_list(tok_end);
    eat(tok_end); eat(tok_while);
    return std::make_unique<While>(std::move(cond),
                                   std::move(body));
}

std::unique_ptr<AST> Parser::for_cycle() {
    eat(tok_for);
    std::string iter = std::get<std::string>(cur_tok.value);
    eat(tok_id); eat(tok_in);
    auto range = expr();
    auto body = statement_list(tok_end);
    eat(tok_end); eat(tok_for);
    return std::make_unique<For>(std::move(range),
                                 std::move(body),
                                 std::move(iter));
}

std::unique_ptr<AST> Parser::list() {
    eat(tok_lbracket);
    std::vector<std::unique_ptr<AST>> elems;
    if (cur_tok.type != tok_rbracket) {
        elems.push_back(expr());
        while (cur_tok.type == tok_comma) {
            eat(tok_comma);
            while (cur_tok.type == tok_eol) eat(tok_eol);
            if (cur_tok.type == tok_rbracket) break;
            elems.push_back(expr());
        }
    }
    while (cur_tok.type == tok_eol) eat(tok_eol);
    eat(tok_rbracket);
    return std::make_unique<List>(std::move(elems));
}

std::unique_ptr<AST> Parser::func() {
    eat(tok_func); eat(tok_lparen);
    std::vector<std::string> params;
    if (cur_tok.type != tok_rparen) {
        params.push_back(std::get<std::string>(cur_tok.value));
        eat(tok_id);
        while (cur_tok.type == tok_comma) {
            eat(tok_comma);
            params.push_back(std::get<std::string>(cur_tok.value));
            eat(tok_id);
        }
    }
    eat(tok_rparen);
    auto body = statement_list(tok_end);
    eat(tok_end); eat(tok_func);

    auto comp = std::make_unique<Compound>();
    for (auto& stmt : body) {
        comp->push(std::move(stmt));
    }
    return std::make_unique<Func>(std::move(params),
                                  std::move(comp));
}

std::unique_ptr<AST> Parser::compound_statement() {
    auto statements = statement_list(tok_eof);
    auto root = std::make_unique<Compound>();
    for (auto& stmt : statements) {
        root->push(std::move(stmt));
    }
    return root;
}

std::vector<std::unique_ptr<AST>> Parser::statement_list(TokenType end1,
                                                         TokenType end2) {
    std::vector<std::unique_ptr<AST>> out;
    while (cur_tok.type == tok_eol) {
        eat(tok_eol);
    }
    while (cur_tok.type != end1 && cur_tok.type != end2) {
        auto stmt = statement();
        if (stmt) {
            out.push_back(std::move(stmt));
        }
        while (cur_tok.type == tok_eol) {
            eat(tok_eol);
        }
    }
    return out;
}

std::unique_ptr<AST> Parser::statement() {
    while (cur_tok.type == tok_eol) {
        eat(tok_eol);
    }
    if (cur_tok.type == tok_break) {
        eat(tok_break);
        return std::make_unique<Break>();
    }
    if (cur_tok.type == tok_continue) {
        eat(tok_continue);
        return std::make_unique<Continue>();
    }
    if (cur_tok.type == tok_ret) {
        eat(tok_ret);
        std::unique_ptr<AST> ex = nullptr;
        if (cur_tok.type != tok_eol && cur_tok.type != tok_end) {
            ex = expr();
        }
        return std::make_unique<Ret>(std::move(ex));
    }
    if (cur_tok.type == tok_id && (
        peek_tok.type == tok_assign ||
        peek_tok.type == tok_plus_assign ||
        peek_tok.type == tok_minus_assign ||
        peek_tok.type == tok_mult_assign ||
        peek_tok.type == tok_div_assign ||
        peek_tok.type == tok_rem_assign ||
        peek_tok.type == tok_pow_assign)) {
        return assignment();
    }
    return expr();
}

std::unique_ptr<AST> Parser::assignment() {
    auto left = std::make_unique<Var>(cur_tok);
    eat(tok_id);
    TokenType op = cur_tok.type;
    eat(op);
    auto right = expr();

    if (op != tok_assign) {
        TokenType binop = tok_ill;
        switch (op) {
            case tok_plus_assign:  binop = tok_plus;  break;
            case tok_minus_assign: binop = tok_minus; break;
            case tok_mult_assign:  binop = tok_mult;  break;
            case tok_div_assign:   binop = tok_slash; break;
            case tok_rem_assign:   binop = tok_rem;   break;
            case tok_pow_assign:   binop = tok_pow;   break;
            default: break;
        }
        auto copy_left = std::make_unique<Var>(Token{tok_id, left->getName()});
        right = std::make_unique<BinOp>(std::move(copy_left),
                                        binop,
                                        std::move(right));
    }
    return std::make_unique<Assign>(std::move(left),
                                    std::move(right));
}

std::unique_ptr<AST> Parser::expr() {
    return parseOr();
}

std::unique_ptr<AST> Parser::parseOr() {
    auto node = parseAnd();
    while (cur_tok.type == tok_or) {
        Token op = cur_tok;
        eat(tok_or);
        node = std::make_unique<BinOp>(std::move(node),
                                       op.type,
                                       parseAnd());
    }
    return node;
}

std::unique_ptr<AST> Parser::parseAnd() {
    auto node = parseEq();
    while (cur_tok.type == tok_and) {
        Token op = cur_tok;
        eat(tok_and);
        node = std::make_unique<BinOp>(std::move(node),
                                       op.type,
                                       parseEq());
    }
    return node;
}

std::unique_ptr<AST> Parser::parseEq() {
    auto node = parseCmp();
    while (cur_tok.type == tok_eq || cur_tok.type == tok_not_eq) {
        Token op = cur_tok;
        eat(op.type);
        node = std::make_unique<BinOp>(std::move(node),
                                       op.type,
                                       parseCmp());
    }
    return node;
}

std::unique_ptr<AST> Parser::parseCmp() {
    auto node = parseAdd();
    while (cur_tok.type == tok_less ||
           cur_tok.type == tok_greater ||
           cur_tok.type == tok_less_or_eq ||
           cur_tok.type == tok_greater_or_eq) {
        Token op = cur_tok;
        eat(op.type);
        node = std::make_unique<BinOp>(std::move(node),
                                       op.type,
                                       parseAdd());
    }
    return node;
}

std::unique_ptr<AST> Parser::parseAdd() {
    auto node = parseMul();
    while (cur_tok.type == tok_plus || cur_tok.type == tok_minus) {
        Token op = cur_tok;
        eat(op.type);
        node = std::make_unique<BinOp>(std::move(node),
                                       op.type,
                                       parseMul());
    }
    return node;
}

std::unique_ptr<AST> Parser::parseMul() {
    auto node = parsePow();
    while (cur_tok.type == tok_mult ||
           cur_tok.type == tok_slash ||
           cur_tok.type == tok_rem) {
        Token op = cur_tok;
        eat(op.type);
        node = std::make_unique<BinOp>(std::move(node),
                                       op.type,
                                       parsePow());
    }
    return node;
}

std::unique_ptr<AST> Parser::parsePow() {
    auto node = postfix();
    if (cur_tok.type == tok_pow) {
        Token op = cur_tok;
        eat(tok_pow);
        node = std::make_unique<BinOp>(std::move(node),
                                       op.type,
                                       parsePow());
    }
    return node;
}

std::unique_ptr<AST> Parser::postfix() {
    auto node = factor();
    while (true) {
        if (cur_tok.type == tok_lbracket) {
            eat(tok_lbracket);
            std::unique_ptr<AST> start = nullptr;
            std::unique_ptr<AST> end = nullptr;
            bool slice = false;
            if (cur_tok.type != tok_colon && cur_tok.type != tok_rbracket) {
                start = expr();
            }
            if (cur_tok.type == tok_colon) {
                eat(tok_colon);
                slice = true;
                if (cur_tok.type != tok_rbracket) {
                    end = expr();
                }
            }
            eat(tok_rbracket);
            node = std::make_unique<Bracket>(std::move(node),
                                             std::move(start),
                                             std::move(end),
                                             slice);
            continue;
        }
        if (cur_tok.type == tok_lparen) {
            eat(tok_lparen);
            std::vector<std::unique_ptr<AST>> args;
            if (cur_tok.type != tok_rparen) {
                args.push_back(expr());
                while (cur_tok.type == tok_comma) {
                    eat(tok_comma);
                    args.push_back(expr());
                }
            }
            eat(tok_rparen);
            node = std::make_unique<CallExpr>(std::move(node),
                                              std::move(args));
            continue;
        }
        break;
    }
    return node;
}

std::unique_ptr<AST> Parser::factor() {
    Token tok = cur_tok;
    if (tok.type == tok_not) {
        eat(tok_not);
        return std::make_unique<UnaryOp>(tok_not, factor());
    }
    if (tok.type == tok_plus) {
        eat(tok_plus);
        return std::make_unique<UnaryOp>(tok_plus, factor());
    }
    if (tok.type == tok_minus) {
        eat(tok_minus);
        return std::make_unique<UnaryOp>(tok_minus, factor());
    }
    if (tok.type == tok_num) {
        eat(tok_num);
        return std::make_unique<Num>(std::get<double>(tok.value));
    }
    if (tok.type == tok_bool) {
        eat(tok_bool);
        return std::make_unique<Bool>(std::get<bool>(tok.value));
    }
    if (tok.type == tok_str) {
        eat(tok_str);
        return std::make_unique<Str>(std::get<std::string>(tok.value));
    }
    if (tok.type == tok_null) {
        eat(tok_null);
        return std::make_unique<Null>(tok);
    }
    if (tok.type == tok_if) {
        return condition();
    }
    if (tok.type == tok_while) {
        return while_cycle();
    }
    if (tok.type == tok_for) {
        return for_cycle();
    }
    if (tok.type == tok_lparen) {
        eat(tok_lparen);
        auto sub = parse();
        eat(tok_rparen);
        return sub;
    }
    if (tok.type == tok_id) {
        eat(tok_id);
        if (cur_tok.type == tok_lparen) {
            eat(tok_lparen);
            std::vector<std::unique_ptr<AST>> args;
            if (cur_tok.type != tok_rparen) {
                args.push_back(expr());
                while (cur_tok.type == tok_comma) {
                    eat(tok_comma);
                    args.push_back(expr());
                }
            }
            eat(tok_rparen);
            return std::make_unique<Call>(std::get<std::string>(tok.value),
                                          std::move(args));
        }
        return std::make_unique<Var>(tok);
    }
    if (tok.type == tok_lbracket) {
        return list();
    }
    if (tok.type == tok_func) {
        return func();
    }
    return nullptr;
}