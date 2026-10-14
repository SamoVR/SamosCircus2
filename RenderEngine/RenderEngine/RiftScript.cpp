#include "RiftScript.h"
#include <cctype>
#include <cmath>
#include <cstdio>
#include <random>
#include <stdexcept>

namespace rs {

std::string Value::toString() const {
    if (isString) return text;
    if (std::isfinite(number) && number == std::floor(number) && std::fabs(number) < 1e15) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(number));
        return buf;
    }
    char buf[48];
    std::snprintf(buf, sizeof(buf), "%.4g", number);
    return buf;
}

// ============================================================================
//  AST
// ============================================================================

struct Expr {
    enum class Kind { Number, String, Variable, Field, Call, Unary, Binary } kind;
    int line = 0;
    double number = 0.0;
    std::string text;      // string literal, variable / function / object name, operator
    std::string field;     // Field: object.field
    std::vector<ExprPtr> args;  // Call arguments, Unary/Binary operands
    explicit Expr(Kind k, int l) : kind(k), line(l) {}
};

struct Stmt {
    enum class Kind { Var, Assign, Command, If, Repeat, Return } kind;
    int line = 0;
    std::string name;
    std::vector<ExprPtr> exprs;                  // Var/Assign: [value]; Command: args; Repeat: [count]
    std::vector<std::pair<ExprPtr, Block>> branches; // If: (condition, body); else = (nullptr, body)
    Block body;                                  // Repeat
    explicit Stmt(Kind k, int l) : kind(k), line(l) {}
};

// ============================================================================
//  Tokenizer
// ============================================================================

namespace {

struct ScriptError : std::runtime_error {
    int line;
    ScriptError(int l, const std::string& m) : std::runtime_error(m), line(l) {}
};

struct Token {
    enum class Type { Number, String, Ident, Op, Newline, Eof } type;
    std::string text;
    double number = 0.0;
    int line = 0;
};

std::vector<Token> tokenize(const std::string& src) {
    std::vector<Token> out;
    int line = 1;
    size_t i = 0;
    auto push = [&](Token::Type t, std::string text, double n = 0.0) { out.push_back({ t, std::move(text), n, line }); };

    while (i < src.size()) {
        const char c = src[i];
        if (c == '\n') { push(Token::Type::Newline, "\\n"); ++line; ++i; continue; }
        if (c == ' ' || c == '\t' || c == '\r' || c == ';') { ++i; continue; }
        if (c == '-' && i + 1 < src.size() && src[i + 1] == '-') { // comment to end of line
            while (i < src.size() && src[i] != '\n') ++i;
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(c)) || (c == '.' && i + 1 < src.size() && std::isdigit(static_cast<unsigned char>(src[i + 1])))) {
            size_t j = i;
            // ".." after a number is the concat operator, not part of it
            while (j < src.size() && (std::isdigit(static_cast<unsigned char>(src[j])) ||
                   (src[j] == '.' && !(j + 1 < src.size() && src[j + 1] == '.')))) ++j;
            const std::string num = src.substr(i, j - i);
            try { push(Token::Type::Number, num, std::stod(num)); }
            catch (...) { throw ScriptError(line, "invalid number '" + num + "'"); }
            i = j;
            continue;
        }
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            size_t j = i;
            while (j < src.size() && (std::isalnum(static_cast<unsigned char>(src[j])) || src[j] == '_')) ++j;
            push(Token::Type::Ident, src.substr(i, j - i));
            i = j;
            continue;
        }
        if (c == '"' || c == '\'') {
            std::string s;
            size_t j = i + 1;
            while (j < src.size() && src[j] != c && src[j] != '\n') {
                if (src[j] == '\\' && j + 1 < src.size()) {
                    ++j;
                    s += src[j] == 'n' ? '\n' : src[j];
                } else {
                    s += src[j];
                }
                ++j;
            }
            if (j >= src.size() || src[j] != c) throw ScriptError(line, "unterminated string");
            push(Token::Type::String, s);
            i = j + 1;
            continue;
        }
        static const char* twoChar[] = { "==", "!=", "<=", ">=", ".." };
        bool matched = false;
        for (const char* op : twoChar) {
            if (src.compare(i, 2, op) == 0) { push(Token::Type::Op, op); i += 2; matched = true; break; }
        }
        if (matched) continue;
        if (std::string("+-*/%<>=(),.").find(c) != std::string::npos) {
            push(Token::Type::Op, std::string(1, c));
            ++i;
            continue;
        }
        throw ScriptError(line, std::string("unexpected character '") + c + "'");
    }
    push(Token::Type::Newline, "\\n");
    push(Token::Type::Eof, "<end of script>");
    return out;
}

// ============================================================================
//  Parser
// ============================================================================

class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : t_(std::move(tokens)) {}

    void parseProgram(Block& init, std::unordered_map<std::string, Block>& handlers) {
        skipNewlines();
        while (!at(Token::Type::Eof)) {
            if (isWord("on")) {
                const int line = next().line;
                const std::string event = expectIdent("event name after 'on'");
                expectNewline();
                if (handlers.count(event)) throw ScriptError(line, "duplicate handler 'on " + event + "'");
                handlers[event] = parseBlock();
                expectWord("end", "'end' to close 'on " + event + "'");
            } else {
                init.push_back(parseStatement());
            }
            skipNewlines();
        }
    }

private:
    std::vector<Token> t_;
    size_t p_ = 0;

    const Token& peek() const { return t_[p_]; }
    const Token& next() { return t_[p_ < t_.size() - 1 ? p_++ : p_]; }
    bool at(Token::Type type) const { return peek().type == type; }
    bool isWord(const char* w) const { return at(Token::Type::Ident) && peek().text == w; }
    bool isOp(const char* o) const { return at(Token::Type::Op) && peek().text == o; }
    void skipNewlines() { while (at(Token::Type::Newline)) ++p_; }

    [[noreturn]] void fail(const std::string& expected) const {
        throw ScriptError(peek().line, "expected " + expected + ", found '" + peek().text + "'");
    }
    std::string expectIdent(const std::string& what) {
        if (!at(Token::Type::Ident)) fail(what);
        return next().text;
    }
    void expectWord(const char* w, const std::string& what) { if (!isWord(w)) fail(what); ++p_; }
    void expectOp(const char* o) { if (!isOp(o)) fail(std::string("'") + o + "'"); ++p_; }
    void expectNewline() { if (!at(Token::Type::Newline)) fail("end of line"); skipNewlines(); }

    bool atBlockEnd() const {
        return at(Token::Type::Eof) || isWord("end") || isWord("else") || isWord("elseif");
    }

    Block parseBlock() {
        Block block;
        skipNewlines();
        while (!atBlockEnd()) {
            block.push_back(parseStatement());
            skipNewlines();
        }
        return block;
    }

    StmtPtr parseStatement() {
        const int line = peek().line;
        if (!at(Token::Type::Ident)) fail("a statement");

        if (isWord("on")) throw ScriptError(line, "'on' handlers must be at the top level");

        if (isWord("var")) {
            ++p_;
            auto s = std::make_unique<Stmt>(Stmt::Kind::Var, line);
            s->name = expectIdent("variable name");
            expectOp("=");
            s->exprs.push_back(parseExpr());
            expectNewline();
            return s;
        }
        if (isWord("if")) {
            ++p_;
            auto s = std::make_unique<Stmt>(Stmt::Kind::If, line);
            ExprPtr cond = parseExpr();
            expectWord("then", "'then'");
            s->branches.emplace_back(std::move(cond), parseBlock());
            while (isWord("elseif")) {
                ++p_;
                ExprPtr c = parseExpr();
                expectWord("then", "'then'");
                s->branches.emplace_back(std::move(c), parseBlock());
            }
            if (isWord("else")) {
                ++p_;
                s->branches.emplace_back(nullptr, parseBlock());
            }
            expectWord("end", "'end' to close 'if'");
            expectNewline();
            return s;
        }
        if (isWord("repeat")) {
            ++p_;
            auto s = std::make_unique<Stmt>(Stmt::Kind::Repeat, line);
            s->exprs.push_back(parseExpr());
            s->body = parseBlock();
            expectWord("end", "'end' to close 'repeat'");
            expectNewline();
            return s;
        }
        if (isWord("return")) {
            ++p_;
            expectNewline();
            return std::make_unique<Stmt>(Stmt::Kind::Return, line);
        }

        const std::string name = next().text;
        if (isOp("=")) {
            ++p_;
            auto s = std::make_unique<Stmt>(Stmt::Kind::Assign, line);
            s->name = name;
            s->exprs.push_back(parseExpr());
            expectNewline();
            return s;
        }

        auto s = std::make_unique<Stmt>(Stmt::Kind::Command, line);
        s->name = name;
        if (!at(Token::Type::Newline)) {
            s->exprs.push_back(parseExpr());
            while (isOp(",")) { ++p_; s->exprs.push_back(parseExpr()); }
        }
        expectNewline();
        return s;
    }

    ExprPtr binary(const std::string& op, ExprPtr l, ExprPtr r, int line) {
        auto e = std::make_unique<Expr>(Expr::Kind::Binary, line);
        e->text = op;
        e->args.push_back(std::move(l));
        e->args.push_back(std::move(r));
        return e;
    }

    ExprPtr parseExpr() { return parseOr(); }

    ExprPtr parseOr() {
        ExprPtr l = parseAnd();
        while (isWord("or")) { const int line = next().line; l = binary("or", std::move(l), parseAnd(), line); }
        return l;
    }
    ExprPtr parseAnd() {
        ExprPtr l = parseNot();
        while (isWord("and")) { const int line = next().line; l = binary("and", std::move(l), parseNot(), line); }
        return l;
    }
    ExprPtr parseNot() {
        if (isWord("not")) {
            const int line = next().line;
            auto e = std::make_unique<Expr>(Expr::Kind::Unary, line);
            e->text = "not";
            e->args.push_back(parseNot());
            return e;
        }
        return parseCompare();
    }
    ExprPtr parseCompare() {
        ExprPtr l = parseConcat();
        while (isOp("==") || isOp("!=") || isOp("<") || isOp(">") || isOp("<=") || isOp(">=")) {
            const Token op = next();
            l = binary(op.text, std::move(l), parseConcat(), op.line);
        }
        return l;
    }
    ExprPtr parseConcat() {
        ExprPtr l = parseAdd();
        while (isOp("..")) { const int line = next().line; l = binary("..", std::move(l), parseAdd(), line); }
        return l;
    }
    ExprPtr parseAdd() {
        ExprPtr l = parseMul();
        while (isOp("+") || isOp("-")) { const Token op = next(); l = binary(op.text, std::move(l), parseMul(), op.line); }
        return l;
    }
    ExprPtr parseMul() {
        ExprPtr l = parseUnary();
        while (isOp("*") || isOp("/") || isOp("%")) { const Token op = next(); l = binary(op.text, std::move(l), parseUnary(), op.line); }
        return l;
    }
    ExprPtr parseUnary() {
        if (isOp("-")) {
            const int line = next().line;
            auto e = std::make_unique<Expr>(Expr::Kind::Unary, line);
            e->text = "-";
            e->args.push_back(parseUnary());
            return e;
        }
        return parsePrimary();
    }
    ExprPtr parsePrimary() {
        const Token tok = peek();
        if (tok.type == Token::Type::Number) {
            ++p_;
            auto e = std::make_unique<Expr>(Expr::Kind::Number, tok.line);
            e->number = tok.number;
            return e;
        }
        if (tok.type == Token::Type::String) {
            ++p_;
            auto e = std::make_unique<Expr>(Expr::Kind::String, tok.line);
            e->text = tok.text;
            return e;
        }
        if (isOp("(")) {
            ++p_;
            ExprPtr e = parseExpr();
            expectOp(")");
            return e;
        }
        if (tok.type == Token::Type::Ident) {
            ++p_;
            if (tok.text == "true" || tok.text == "false") {
                auto e = std::make_unique<Expr>(Expr::Kind::Number, tok.line);
                e->number = tok.text == "true" ? 1.0 : 0.0;
                return e;
            }
            if (isOp("(")) {
                ++p_;
                auto e = std::make_unique<Expr>(Expr::Kind::Call, tok.line);
                e->text = tok.text;
                if (!isOp(")")) {
                    e->args.push_back(parseExpr());
                    while (isOp(",")) { ++p_; e->args.push_back(parseExpr()); }
                }
                expectOp(")");
                return e;
            }
            if (isOp(".")) {
                ++p_;
                auto e = std::make_unique<Expr>(Expr::Kind::Field, tok.line);
                e->text = tok.text;
                e->field = expectIdent("field name after '.'");
                return e;
            }
            auto e = std::make_unique<Expr>(Expr::Kind::Variable, tok.line);
            e->text = tok.text;
            return e;
        }
        fail("a value");
    }
};

// ============================================================================
//  Interpreter
// ============================================================================

enum class Flow { Normal, Return };

std::mt19937& rng() {
    static std::mt19937 gen{ std::random_device{}() };
    return gen;
}

double toNumber(const Value& v, int line) {
    if (!v.isString) return v.number;
    try { size_t used = 0; const double d = std::stod(v.text, &used); if (used == v.text.size()) return d; }
    catch (...) {}
    throw ScriptError(line, "'" + v.text + "' is not a number");
}

class Interpreter {
public:
    Interpreter(std::unordered_map<std::string, Value>& globals, Host& host) : globals_(globals), host_(host) {}

    Flow run(const Block& block) {
        for (const auto& s : block) {
            if (exec(*s) == Flow::Return) return Flow::Return;
        }
        return Flow::Normal;
    }

private:
    std::unordered_map<std::string, Value>& globals_;
    Host& host_;
    long long budget_ = 200000; // statements per event: an endless repeat can't freeze the editor

    Flow exec(const Stmt& s) {
        if (--budget_ < 0) throw ScriptError(s.line, "script ran too long in one frame (endless loop?)");
        switch (s.kind) {
        case Stmt::Kind::Var:
            globals_[s.name] = eval(*s.exprs[0]);
            return Flow::Normal;
        case Stmt::Kind::Assign:
            if (!globals_.count(s.name)) throw ScriptError(s.line, "unknown variable '" + s.name + "' (declare it with 'var')");
            globals_[s.name] = eval(*s.exprs[0]);
            return Flow::Normal;
        case Stmt::Kind::Return:
            return Flow::Return;
        case Stmt::Kind::If:
            for (const auto& [cond, body] : s.branches) {
                if (!cond || eval(*cond).truthy()) return run(body);
            }
            return Flow::Normal;
        case Stmt::Kind::Repeat: {
            const double n = toNumber(eval(*s.exprs[0]), s.line);
            for (long long i = 0; i < static_cast<long long>(n); ++i) {
                if (run(s.body) == Flow::Return) return Flow::Return;
            }
            return Flow::Normal;
        }
        case Stmt::Kind::Command: {
            std::vector<Value> args;
            for (const auto& e : s.exprs) args.push_back(eval(*e));
            if (s.name == "print") {
                std::string line;
                for (size_t i = 0; i < args.size(); ++i) line += (i ? " " : "") + args[i].toString();
                host_.print(line);
                return Flow::Normal;
            }
            std::string error;
            if (!host_.command(s.name, args, error)) throw ScriptError(s.line, error);
            return Flow::Normal;
        }
        }
        return Flow::Normal;
    }

    Value eval(const Expr& e) {
        switch (e.kind) {
        case Expr::Kind::Number: return Value(e.number);
        case Expr::Kind::String: return Value(e.text);
        case Expr::Kind::Variable: {
            auto it = globals_.find(e.text);
            if (it == globals_.end()) throw ScriptError(e.line, "unknown variable '" + e.text + "'");
            return it->second;
        }
        case Expr::Kind::Field: {
            Value out;
            std::string error;
            if (!host_.getField(e.text, e.field, out, error)) throw ScriptError(e.line, error);
            return out;
        }
        case Expr::Kind::Unary: {
            const Value v = eval(*e.args[0]);
            if (e.text == "not") return Value(v.truthy() ? 0.0 : 1.0);
            return Value(-toNumber(v, e.line));
        }
        case Expr::Kind::Binary: return binary(e);
        case Expr::Kind::Call: return call(e);
        }
        return Value();
    }

    Value binary(const Expr& e) {
        const std::string& op = e.text;
        if (op == "and") { const Value l = eval(*e.args[0]); return l.truthy() ? eval(*e.args[1]) : l; }
        if (op == "or")  { const Value l = eval(*e.args[0]); return l.truthy() ? l : eval(*e.args[1]); }

        const Value l = eval(*e.args[0]);
        const Value r = eval(*e.args[1]);
        if (op == "..") return Value(l.toString() + r.toString());
        if (op == "==" || op == "!=") {
            bool eq;
            if (l.isString || r.isString) eq = l.toString() == r.toString();
            else eq = l.number == r.number;
            return Value((op == "==") == eq ? 1.0 : 0.0);
        }
        if (op == "+" && l.isString && r.isString) return Value(l.text + r.text);

        const double a = toNumber(l, e.line), b = toNumber(r, e.line);
        if (op == "+") return Value(a + b);
        if (op == "-") return Value(a - b);
        if (op == "*") return Value(a * b);
        if (op == "/") { if (b == 0.0) throw ScriptError(e.line, "division by zero"); return Value(a / b); }
        if (op == "%") { if (b == 0.0) throw ScriptError(e.line, "division by zero"); return Value(std::fmod(a, b)); }
        if (op == "<") return Value(a < b ? 1.0 : 0.0);
        if (op == ">") return Value(a > b ? 1.0 : 0.0);
        if (op == "<=") return Value(a <= b ? 1.0 : 0.0);
        if (op == ">=") return Value(a >= b ? 1.0 : 0.0);
        throw ScriptError(e.line, "unknown operator " + op);
    }

    Value call(const Expr& e) {
        std::vector<Value> args;
        for (const auto& a : e.args) args.push_back(eval(*a));
        const std::string& f = e.text;
        auto num = [&](size_t i) {
            if (i >= args.size()) throw ScriptError(e.line, f + "() needs " + std::to_string(i + 1) + " argument(s)");
            return toNumber(args[i], e.line);
        };
        constexpr double kDeg = 3.14159265358979323846 / 180.0;
        if (f == "sin") return Value(std::sin(num(0) * kDeg)); // degrees, like the rotation fields
        if (f == "cos") return Value(std::cos(num(0) * kDeg));
        if (f == "abs") return Value(std::fabs(num(0)));
        if (f == "sqrt") return Value(std::sqrt(std::fmax(0.0, num(0))));
        if (f == "floor") return Value(std::floor(num(0)));
        if (f == "round") return Value(std::round(num(0)));
        if (f == "min") return Value(std::fmin(num(0), num(1)));
        if (f == "max") return Value(std::fmax(num(0), num(1)));
        if (f == "clamp") return Value(std::fmin(std::fmax(num(0), num(1)), num(2)));
        if (f == "random") {
            const double lo = args.empty() ? 0.0 : num(0), hi = args.size() < 2 ? 1.0 : num(1);
            return Value(std::uniform_real_distribution<double>(lo, std::fmax(lo, hi))(rng()));
        }
        if (f == "str") { if (args.empty()) return Value(std::string()); return Value(args[0].toString()); }

        Value out;
        std::string error;
        if (!host_.callFunction(f, args, out, error)) throw ScriptError(e.line, error);
        return out;
    }
};

} // namespace

// ============================================================================
//  Script
// ============================================================================

Script::Script() = default;
Script::~Script() = default;
Script::Script(Script&&) noexcept = default;
Script& Script::operator=(Script&&) noexcept = default;

bool Script::compile(const std::string& source) {
    init_.clear();
    handlers_.clear();
    globals_.clear();
    failed_ = false;
    error_.clear();
    try {
        Parser parser(tokenize(source));
        parser.parseProgram(init_, handlers_);
        return true;
    } catch (const ScriptError& e) {
        failed_ = true;
        error_ = "line " + std::to_string(e.line) + ": " + e.what();
        return false;
    }
}

bool Script::execute(const Block& block, Host& host) {
    if (failed_) return false;
    try {
        Interpreter(globals_, host).run(block);
        return true;
    } catch (const ScriptError& e) {
        failed_ = true;
        error_ = "line " + std::to_string(e.line) + ": " + e.what();
        return false;
    }
}

bool Script::runInit(Host& host) { return execute(init_, host); }

bool Script::fire(const std::string& event, Host& host) {
    auto it = handlers_.find(event);
    if (it == handlers_.end()) return true;
    return execute(it->second, host);
}

bool Script::hasHandler(const std::string& event) const { return handlers_.count(event) != 0; }

const char* languageReference() {
    return
        "EVENTS        on start | on update | on touch  ... end\n"
        "VARIABLES     var speed = 5      speed = speed + 1\n"
        "BUILT-IN VARS dt (frame time), time (seconds since Play), other (name of the touching part)\n"
        "FLOW          if a > 1 and b then ... elseif ... else ... end\n"
        "              repeat 3 ... end      return\n"
        "OPERATORS     + - * / %  == != < > <= >=  and or not  .. (join text)\n"
        "FIELDS        self.x .y .z  .rx .ry .rz  .sx .sy .sz  .vx .vy .vz  .name .anchored\n"
        "              player.x .y .z     other.x ... (inside 'on touch')\n"
        "FUNCTIONS     sin(deg) cos(deg) abs sqrt floor round min max clamp random(a,b) str(x)\n"
        "              key(\"W\") -> 1 while held    distance(\"PartName\") -> to player\n"
        "COMMANDS      print a, b            message \"text\", seconds\n"
        "              move x,y,z   rotate x,y,z   setpos x,y,z   setrot x,y,z   setscale x,y,z\n"
        "              color r,g,b   velocity x,y,z   push x,y,z   anchored 0/1\n"
        "              hide   show   collide 0/1   win \"text\"   lose \"text\"\n"
        "PLAYER        teleport x,y,z   respawn   launch x,y,z (sets the player's velocity)\n";
}

} // namespace rs
