#pragma once
#include "lexer.h"
#include <algorithm>
#include <random>
#include <sstream>
#include <cmath>
#include<unordered_map>

struct ListType {
    std::vector<Value> elems;
};

struct Function {
    std::vector<std::string> params;
    std::unique_ptr<AST>      body;
};

struct Return {
    Value value;
};

struct BreakThrow {};
struct ContinueThrow {};

class AST {
public:
    virtual ~AST() = default;
    virtual Value visit() = 0;
};

class Num : public AST {
public:
    Num(double num) : value(num) {}

    Value visit() {
        return value;
    }

private:
    double value;
};

class Bool : public AST {
public:
    Bool(bool b) : value(b) {}
    Value visit() {
        return value;
    }
private:
    bool value;
};

class Str : public AST {
public:
    Str(std::string str) : value(std::move(str)) {}

    Value visit() {
        return value;
    }

private:
    std::string value;
};

class If : public AST {
public:
    If(std::unique_ptr<AST> c, std::vector<std::unique_ptr<AST>> i, std::vector<std::unique_ptr<AST>> e = {}) 
    : condition(std::move(c)), if_body(std::move(i)), else_body(std::move(e)) {}

	Value visit() override {
		bool res = truthy(condition->visit());
		if (res) {
			for (auto& stmt : if_body) {
				stmt->visit();
			}
		}
		else {
			for (auto& stmt : else_body) {
				stmt->visit();
			}
		}

		return NullType{};
	}

private:
    std::unique_ptr<AST> condition;
    std::vector<std::unique_ptr<AST>> if_body, else_body;
};

class While : public AST {
public:
    While(std::unique_ptr<AST> c, std::vector<std::unique_ptr<AST>> b) : condition(std::move(c)), body(std::move(b)) {}

    Value visit() {
        while (truthy(condition->visit())) {
            try {
                for (std::unique_ptr<AST>& stmt : body) {
                    stmt->visit();
                }
            }
            catch (ContinueThrow&) {
                continue;
            }
            catch (BreakThrow&) {
                break;
            }
        }
        return NullType{};
    }

private:
    std::unique_ptr<AST> condition;
    std::vector<std::unique_ptr<AST>> body;
};

class For : public AST {
public:
    For(std::unique_ptr<AST> r, std::vector<std::unique_ptr<AST>> b, std::string it) : 
    range(std::move(r)), body(std::move(b)), iter_name(it) {}

    Value visit() {
        Value elems = range->visit();
        if (std::holds_alternative<std::string>(elems)) {
            for (char i : std::get<std::string>(elems)) {
                setVar(iter_name, std::string(1, i));
                try {
                    for (std::unique_ptr<AST>& i : body) {
                        i->visit();
                    }
                }
                catch (ContinueThrow) {
                    continue;
                }
                catch (BreakThrow) {
                    break;
                }
            }
        }
        else if (std::holds_alternative<std::shared_ptr<ListType>>(elems)) {
            for (Value i : std::get<std::shared_ptr<ListType>>(elems)->elems) {
                setVar(iter_name, i);
                try {
                    for (std::unique_ptr<AST>& i : body) {
                        i->visit();
                    }
                }
                catch (ContinueThrow) {
                    continue;
                }
                catch (BreakThrow) {
                    break;
                }
            }
        }
        return NullType{};
    }

private:
    std::unique_ptr<AST> range;
    std::vector<std::unique_ptr<AST>> body;
    std::string iter_name;
};

class Break : public AST {
public:
    Break() {}

    Value visit() {
        throw BreakThrow();
    }
};

class Continue : public AST {
public:
    Continue() {}

    Value visit() {
        throw ContinueThrow();
    }
};

class Bracket : public AST {
public:
    Bracket(std::unique_ptr<AST> collection, std::unique_ptr<AST> start, std::unique_ptr<AST> end, bool slice)
    : seq(std::move(collection)), start(std::move(start)), end(std::move(end)), isSlice(slice) {}

    Value visit() {
        Value cv = seq->visit();
        if (std::holds_alternative<std::string>(cv)) {
            std::string str = std::get<std::string>(cv);
            int n = int(str.size());
            int i = 0, j = n;
            if (start) {
                int si = int(std::get<double>(start->visit()));
                if (si < 0) {
                    si += n;
                }
                i = std::clamp(si, 0, n);
            }
            if (isSlice) {
                if (end) {
                    int ei = int(std::get<double>(end->visit()));
                    if (ei < 0) {
                        ei += n;
                    }
                    j = std::clamp(ei, 0, n);
                }
                return str.substr(i, std::max(0, j - i));
            }
            else {
                if (i < n) {
                    return std::string(1, str[i]);
                }
                return std::string{};
            }
        }
        if (std::holds_alternative<std::shared_ptr<ListType>>(cv)) {
            std::shared_ptr<ListType> lst = std::get<std::shared_ptr<ListType>>(cv);
            int n = int(lst->elems.size());
            int i = 0, j = n;
            if (start) {
                int si = int(std::get<double>(start->visit()));
                if (si < 0) {
                    si += n;
                }
                i = std::clamp(si, 0, n);
            }
            if (isSlice) {
                if (end) {
                    int ei = int(std::get<double>(end->visit()));
                    if (ei < 0) {
                        ei += n;
                    }
                    j = std::clamp(ei, 0, n);
                }
                std::shared_ptr<ListType> out = std::make_shared<ListType>();
                out->elems.insert(out->elems.end(), lst->elems.begin() + i, lst->elems.begin() + j);
                return out;
            }
            else {
                if (i < n) {
                    return lst->elems[i];
                }
                return NullType{};
            }
        }
        return NullType{};
    }

private:
    std::unique_ptr<AST> seq;
    std::unique_ptr<AST> start;
    std::unique_ptr<AST> end;
    bool isSlice;
};

class UnaryOp : public AST {
public:
    UnaryOp(TokenType token, std::unique_ptr<AST> val) : tok(token), value(std::move(val)) {}
    Value visit() {
        if (tok == tok_plus) {
            return std::get<double>(value->visit());
        }
        if (tok == tok_minus) {
            return -std::get<double>(value->visit());
        }
        if (tok == tok_not) {
            return !truthy(value->visit());
        }
        return NullType{};
    }
private:
    TokenType tok;
    std::unique_ptr<AST> value;
};

class BinOp : public AST {
public:
    BinOp(std::unique_ptr<AST> lhs, TokenType token, std::unique_ptr<AST> rhs) 
    : left(std::move(lhs)), tok(token), right(std::move(rhs)) {}

    Value visit() {
        Value l = left->visit();

        if (tok == tok_and) {
            if (!truthy(l)) return false;
            return truthy(right->visit());
        }
        if (tok == tok_or) {
            if (truthy(l))  return true;
            return truthy(right->visit());
        }

        Value r = right->visit();

        switch (tok) {
        case tok_plus:
            if (std::holds_alternative<double>(l) && std::holds_alternative<double>(r))
                return std::get<double>(l) + std::get<double>(r);
            if (std::holds_alternative<std::string>(l) && std::holds_alternative<std::string>(r))
                return std::get<std::string>(l) + std::get<std::string>(r);
            if (std::holds_alternative<std::shared_ptr<ListType>>(l) && std::holds_alternative<std::shared_ptr<ListType>>(r)) {
                auto out = std::make_shared<ListType>();
                auto ll = std::get<std::shared_ptr<ListType>>(l);
                auto rr = std::get<std::shared_ptr<ListType>>(r);
                out->elems = ll->elems;
                out->elems.insert(out->elems.end(), rr->elems.begin(), rr->elems.end());
                return out;
            }
            break;

        case tok_minus:
            if (std::holds_alternative<double>(l) && std::holds_alternative<double>(r))
                return std::get<double>(l) - std::get<double>(r);
            if (std::holds_alternative<std::string>(l) && std::holds_alternative<std::string>(r)) {
                std::string ls = std::get<std::string>(l);
                std::string rs = std::get<std::string>(r);
                if (ls.size() >= rs.size() && ls.compare(ls.size() - rs.size(), rs.size(), rs) == 0)
                    return ls.substr(0, ls.size() - rs.size());
                return ls;
            }
            break;

        case tok_mult:
            if (std::holds_alternative<std::string>(l) && std::holds_alternative<double>(r)) {
                std::string s = std::get<std::string>(l);
                int n = std::max(0, int(std::floor(std::get<double>(r))));
                std::string out; out.reserve(s.size() * n);
                while (n--) out += s;
                return out;
            }
            if (std::holds_alternative<double>(l) && std::holds_alternative<std::string>(r)) {
                std::string s = std::get<std::string>(r);
                int n = std::max(0, int(std::floor(std::get<double>(l))));
                std::string out; out.reserve(s.size() * n);
                while (n--) out += s;
                return out;
            }
            if (std::holds_alternative<std::shared_ptr<ListType>>(l) && std::holds_alternative<double>(r)) {
                auto lst = std::get<std::shared_ptr<ListType>>(l);
                int n = std::max(0, int(std::floor(std::get<double>(r))));
                auto out = std::make_shared<ListType>();
                while (n--) out->elems.insert(out->elems.end(), lst->elems.begin(), lst->elems.end());
                return out;
            }
            if (std::holds_alternative<double>(l) && std::holds_alternative<std::shared_ptr<ListType>>(r)) {
                auto lst = std::get<std::shared_ptr<ListType>>(r);
                int n = std::max(0, int(std::floor(std::get<double>(l))));
                auto out = std::make_shared<ListType>();
                while (n--) out->elems.insert(out->elems.end(), lst->elems.begin(), lst->elems.end());
                return out;
            }
            if (std::holds_alternative<double>(l) && std::holds_alternative<double>(r))
                return std::get<double>(l) * std::get<double>(r);
            break;

        case tok_slash:
        {
            double d = std::get<double>(r);
            if (d == 0.0) return NullType{};
            return std::get<double>(l) / d;
        }

        case tok_rem:
        {
            int d = int(std::get<double>(r));
            if (d == 0) return NullType{};
            return double(int(std::get<double>(l)) % d);
        }

        case tok_pow:
            return std::pow(std::get<double>(l), std::get<double>(r));

        case tok_less:
            if (std::holds_alternative<double>(l) && std::holds_alternative<double>(r))
                return std::get<double>(l) < std::get<double>(r);
            if (std::holds_alternative<std::string>(l) && std::holds_alternative<std::string>(r))
                return std::get<std::string>(l) < std::get<std::string>(r);
            break;

        case tok_greater:
            if (std::holds_alternative<double>(l) && std::holds_alternative<double>(r))
                return std::get<double>(l) > std::get<double>(r);
            if (std::holds_alternative<std::string>(l) && std::holds_alternative<std::string>(r))
                return std::get<std::string>(l) > std::get<std::string>(r);
            break;

        case tok_less_or_eq:
            if (std::holds_alternative<double>(l) && std::holds_alternative<double>(r))
                return std::get<double>(l) <= std::get<double>(r);
            if (std::holds_alternative<std::string>(l) && std::holds_alternative<std::string>(r))
                return std::get<std::string>(l) <= std::get<std::string>(r);
            break;

        case tok_greater_or_eq:
            if (std::holds_alternative<double>(l) && std::holds_alternative<double>(r))
                return std::get<double>(l) >= std::get<double>(r);
            if (std::holds_alternative<std::string>(l) && std::holds_alternative<std::string>(r))
                return std::get<std::string>(l) >= std::get<std::string>(r);
            break;

        case tok_eq:
            if (std::holds_alternative<NullType>(l) || std::holds_alternative<NullType>(r))
                return std::holds_alternative<NullType>(l) && std::holds_alternative<NullType>(r);
            if (l.index() != r.index()) return false;
            if (std::holds_alternative<double>(l))   return std::get<double>(l) == std::get<double>(r);
            if (std::holds_alternative<bool>(l))     return std::get<bool>(l) == std::get<bool>(r);
            if (std::holds_alternative<std::string>(l)) return std::get<std::string>(l) == std::get<std::string>(r);
            return false;

        case tok_not_eq:
            return !compare_eq(l, r);

        default:
            break;
        }
        return NullType{};
    }
private:
    TokenType tok;
    std::unique_ptr<AST> left;
    std::unique_ptr<AST> right;

    bool  compare_eq(const Value& l, const Value& r) {
        if (std::holds_alternative<NullType>(l) || std::holds_alternative<NullType>(r))
            return std::holds_alternative<NullType>(l) && std::holds_alternative<NullType>(r);
        if (l.index() != r.index()) return false;
        if (auto pd = std::get_if<double>(&l))
            return *pd == std::get<double>(r);
        if (auto pb = std::get_if<bool>(&l))
            return *pb == std::get<bool>(r);
        if (auto ps = std::get_if<std::string>(&l))
            return *ps == std::get<std::string>(r);
        return false;
    }
};

class Compound : public AST {
public:
    void push(std::unique_ptr<AST> stmt) {
        if (stmt) statements.emplace_back(std::move(stmt));
    }
    Value visit() override {
        for (auto& s : statements) {
            try {
                s->visit();
            }
            catch (Return& r) {
                return r.value;
            }
        }
        return Value{NullType{}};
    }
private:
    std::vector<std::unique_ptr<AST>> statements;
};

class Null : public AST {
public:
    Null(Token) {}

    Value visit() {
        return NullType{};
    }
};

class Var : public AST {
public:
    Var(Token tok) : name(std::get<std::string>(tok.value)) {}

    Value visit() {
        return getVar(name);
    }

    std::string getName() {
        return name;
    }

private:
    std::string name;
};

class Assign : public AST {
public:
	Assign(std::unique_ptr<Var> left, std::unique_ptr<AST> right)
		: left(std::move(left)), right(std::move(right))
	{}

	Value visit() override {
		Value v = right->visit();
		setVar(left->getName(), v);
		return v;
	}

private:
	std::unique_ptr<Var> left;
	std::unique_ptr<AST> right;
};

class List : public AST {
public:
    List(std::vector<std::unique_ptr<AST>> elems)
        : elems_(std::move(elems))
    {}

    Value visit() override {
        ListType lst;
        for (auto& element : elems_) {
            lst.elems.push_back(element->visit());
        }
        return std::make_shared<ListType>(std::move(lst));
    }

private:
    std::vector<std::unique_ptr<AST>> elems_;
};

class Func : public AST {
public:
    Func(std::vector<std::string> params, std::unique_ptr<AST> body)
        : func{ std::move(params), std::move(body) }
    {}

    Value visit() override {
        return std::make_shared<Function>(std::move(func));
    }

private:
    Function func;
};

class Ret : public AST {
public:
    explicit Ret(std::unique_ptr<AST> e)
        : expr(std::move(e))
    {}

    Value visit() override {
        if (expr) {
            throw Return{ expr->visit() };
        }
        throw Return{ NullType{} };
    }

private:
    std::unique_ptr<AST> expr;
};

class Call : public AST {
public:
    Call(std::string n, std::vector<std::unique_ptr<AST>> a) : name(std::move(n)), args(std::move(a)) {}

    Value visit() {
        if (name == "abs") {
            return std::abs(std::get<double>(args[0]->visit()));
        }

        if (name == "ceil") {
            return ceil(std::get<double>(args[0]->visit()));
        }

        if (name == "floor") {
            return floor(std::get<double>(args[0]->visit()));
        }

        if (name == "round") {
            return round(std::get<double>(args[0]->visit()));
        }

        if (name == "sqrt") {
            return sqrt(std::get<double>(args[0]->visit()));
        }

        if (name == "rnd") {
            if (args.empty()) {
                return 0.0;
            }
            Value v = args[0]->visit();
            int n = static_cast<int>(std::get<double>(v));
            if (n <= 0) {
                return 0.0;
            }
            static std::mt19937 engine{ std::random_device{}() };
            std::uniform_int_distribution<int> dist(0, n - 1);
            return static_cast<double>(dist(engine));
        }

        if (name == "parse_num") {
            if (args.empty()) {
                return NullType{};
            }
            Value v = args[0]->visit();
            if (!std::holds_alternative<std::string>(v)) {
                return NullType{};
            }
            std::string s = std::get<std::string>(v);
            try {
                double d = std::stod(s);
                return d;
            }
            catch (std::invalid_argument) {
                return NullType{};
            }
            catch (std::out_of_range) {
                return NullType{};
            }
        }

        if (name == "to_string") {
            if (args.empty()) {
                return NullType{};
            }
            Value v = args[0]->visit();
            if (!std::holds_alternative<double>(v)) {
                return NullType{};
            }
            double d = std::get<double>(v);
            return std::to_string(d);
        }

        if (name == "len") {
            if (args.empty()) {
                return 0.0;
            }
            Value v = args[0]->visit();
            if (std::holds_alternative<std::string>(v)) {
                return static_cast<double>(std::get<std::string>(v).size());
            }
            if (std::holds_alternative<std::shared_ptr<ListType>>(v)) {
                return static_cast<double>(std::get<std::shared_ptr<ListType>>(v)->elems.size());
            }
            return 0.0;
        }

        if (name == "lower") {
            if (args.empty()) {
                return std::string{};
            }
            Value v = args[0]->visit();
            std::string s = std::get<std::string>(v);
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        }

        if (name == "upper") {
            if (args.empty()) {
                return std::string{};
            }
            Value v = args[0]->visit();
            std::string s = std::get<std::string>(v);
            std::transform(s.begin(), s.end(), s.begin(), ::toupper);
            return s;
        }

        if (name == "split") {
            if (args.size() < 2) {
                return std::make_shared<ListType>();
            }
            Value v0 = args[0]->visit();
            Value v1 = args[1]->visit();
            std::string s = std::get<std::string>(v0);
            std::string d = std::get<std::string>(v1);
            std::shared_ptr<ListType> out = std::make_shared<ListType>();
            int pos = 0, n;
            while ((n = s.find(d, pos)) != std::string::npos) {
                out->elems.push_back(s.substr(pos, n - pos));
                pos = n + d.size();
            }
            out->elems.push_back(s.substr(pos));
            return out;
        }

        if (name == "join") {
            if (args.size() < 2) {
                return std::string{};
            }
            Value v0 = args[0]->visit();
            Value v1 = args[1]->visit();
            std::shared_ptr<ListType> lst = std::get<std::shared_ptr<ListType>>(v0);
            std::string d = std::get<std::string>(v1);
            std::string result;
            for (int i = 0; i < lst->elems.size(); ++i) {
                result += std::get<std::string>(lst->elems[i]);
                if (i + 1 < lst->elems.size()) {
                    result += d;
                }
            }
            return result;
        }

        if (name == "replace") {
            if (args.size() < 3) {
                return std::string{};
            }
            Value v0 = args[0]->visit();
            Value v1 = args[1]->visit();
            Value v2 = args[2]->visit();
            std::string s = std::get<std::string>(v0);
            std::string oldv = std::get<std::string>(v1);
            std::string newv = std::get<std::string>(v2);
            int pos = 0;
            while ((pos = s.find(oldv, pos)) != std::string::npos) {
                s.replace(pos, oldv.size(), newv);
                pos += newv.size();
            }
            return s;
        }

        if (name == "range") {
            if (args.size() < 2) {
                return std::make_shared<ListType>();
            }
            Value v0 = args[0]->visit();
            Value v1 = args[1]->visit();
            int start = static_cast<int>(std::get<double>(v0));
            int end = static_cast<int>(std::get<double>(v1));
            int step = 1;
            if (args.size() >= 3) {
                step = static_cast<int>(std::get<double>(args[2]->visit()));
            }
            auto out = std::make_shared<ListType>();
            if (step > 0) {
                for (int i = start; i < end; i += step) {
                    out->elems.push_back(static_cast<double>(i));
                }
            }
            else if (step < 0) {
                for (int i = start; i > end; i += step) {
                    out->elems.push_back(static_cast<double>(i));
                }
            }
            return out;
        }

        if (name == "push") {
            if (args.size() < 2) {
                return NullType{};
            }
            std::shared_ptr<ListType> lst = std::get<std::shared_ptr<ListType>>(args[0]->visit());
            Value v = args[1]->visit();
            lst->elems.push_back(v);
            return lst;
        }

        if (name == "pop") {
            if (args.empty()) {
                return NullType{};
            }
            std::shared_ptr<ListType> lst = std::get<std::shared_ptr<ListType>>(args[0]->visit());
            if (lst->elems.empty()) {
                return NullType{};
            }
            Value val = lst->elems.back();
            lst->elems.pop_back();
            return val;
        }

        if (name == "insert") {
            if (args.size() < 3) {
                return NullType{};
            }
            std::shared_ptr<ListType> lst = std::get<std::shared_ptr<ListType>>(args[0]->visit());
            int idx = static_cast<int>(std::get<double>(args[1]->visit()));
            Value v = args[2]->visit();
            if (idx < 0) {
                idx = 0;
            }
            if (idx > static_cast<int>(lst->elems.size())) {
                idx = lst->elems.size();
            }
            lst->elems.insert(lst->elems.begin() + idx, v);
            return lst;
        }

        if (name == "remove") {
            if (args.size() < 2) {
                return NullType{};
            }
            std::shared_ptr<ListType> lst = std::get<std::shared_ptr<ListType>>(args[0]->visit());
            int idx = static_cast<int>(std::get<double>(args[1]->visit()));
            if (idx < 0 || idx >= static_cast<int>(lst->elems.size())) {
                return NullType{};
            }
            Value val = lst->elems[idx];
            lst->elems.erase(lst->elems.begin() + idx);
            return val;
        }

        if (name == "sort") {
            if (args.empty()) {
                return std::make_shared<ListType>();
            }
            Value v0 = args[0]->visit();
            std::shared_ptr<ListType> lst = std::get<std::shared_ptr<ListType>>(v0);
            std::vector<Value>& elems = lst->elems;
            std::stable_sort(elems.begin(), elems.end(), [](const Value& a, const Value& b) {
                if (a.index() != b.index()) {
                    return a.index() < b.index();
                }
                switch (a.index()) {
                case 0:
                    return std::get<double>(a) < std::get<double>(b);
                case 1:
                    return std::get<std::string>(a) < std::get<std::string>(b);
                default:
                    return false;
                }
                });
            return lst;
        }

        if (name == "print" || name == "println") {
            for (int i = 0; i < args.size(); ++i) {
                Value v = args[i]->visit();

                std::visit([&](auto&& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        if (val.find_first_of(" \t") != std::string::npos)
                            *itmo_out << '"' << val << '"';
                        else
                            *itmo_out << val;
                    }
                    else {
                        *itmo_out << val;
                    }
                    }, v);

                if (i + 1 != args.size()) *itmo_out << ' ';
            }
            if (name == "println") {
                *itmo_out << '\n';
            }
            return NullType{};
        }

        if (name == "read") {
            std::string in;
            std::getline(std::cin, in);
            return in;
        }

        if (name == "stacktrace") {
            std::shared_ptr<ListType> stacktrace = std::make_shared<ListType>();
            for (std::string call : call_stack) {
                stacktrace->elems.push_back(call);
            }
            return stacktrace;
        }

        auto fn = std::get<std::shared_ptr<Function>>(getVar(name));
        std::unordered_map<std::string, Value> locals;
        scopes.push_back(&locals);
        call_stack.push_back(name);

        for (int i = 0; i < fn->params.size(); ++i) {
            if (i < args.size()) {
                locals[fn->params[i]] = args[i]->visit();
            }
            else {
                locals[fn->params[i]] = NullType{};
            }
        }

        Value ret = NullType{};
        try {
            ret = fn->body->visit();
        }
        catch (Return const& r) {
            ret = r.value;
        }
        scopes.pop_back();
        call_stack.pop_back();
        return ret;
    }

private:
    std::string name;
    std::vector<std::unique_ptr<AST>> args;
};

class CallExpr : public AST {
public:
    CallExpr(std::unique_ptr<AST> callee,
             std::vector<std::unique_ptr<AST>> arguments)
      : fn(std::move(callee)),
        args(std::move(arguments))
    {}

    Value visit() override {
        Value fv = fn->visit();
        auto fptr = std::get<std::shared_ptr<Function>>(fv);

        std::unordered_map<std::string, Value> locals;
        scopes.push_back(&locals);
        call_stack.push_back("<lambda>");

        for (size_t i = 0; i < fptr->params.size(); ++i) {
            Value argVal = (i < args.size())
                         ? args[i]->visit()
                         : NullType{};
            locals[fptr->params[i]] = std::move(argVal);
        }

        Value ret = NullType{};
        try {
            ret = fptr->body->visit();
        }
        catch (Return const& r) {
            ret = r.value;
        }

        scopes.pop_back();
        call_stack.pop_back();
        return ret;
    }

private:
    std::unique_ptr<AST> fn;
    std::vector<std::unique_ptr<AST>> args;
};