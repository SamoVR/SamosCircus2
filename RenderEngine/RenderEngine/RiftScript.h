#pragma once
//
// RiftScript.h -- the small scripting language attached to objects in play
// mode (the RIFT equivalent of a Roblox Script). Self-contained: tokenizer,
// recursive-descent parser to an AST, tree-walking interpreter.
//
//   -- comment
//   var speed = 90
//   on start
//       print "Hello from " .. self.name
//   end
//   on update
//       rotate 0, speed * dt, 0
//       if key("E") and self.y < 5 then
//           move 0, 2 * dt, 0
//       end
//   end
//   on touch
//       if other == "Player" then
//           color 1, 0, 0
//           message "You touched " .. self.name, 2
//       end
//   end
//
// Values are numbers or strings. Commands (move, color, ...) and object
// fields (self.x, player.y, ...) are provided by the host (GameRuntime), so
// the language core knows nothing about the engine.
//
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace rs {

struct Value {
    bool isString = false;
    double number = 0.0;
    std::string text;

    Value() = default;
    Value(double n) : number(n) {}
    Value(std::string s) : isString(true), text(std::move(s)) {}

    bool truthy() const { return isString ? !text.empty() : number != 0.0; }
    std::string toString() const;
};

// Engine-side services a script can use. Every method returns false (and
// fills `error`) for unknown names, which stops the script with a message.
class Host {
public:
    virtual ~Host() = default;
    virtual bool command(const std::string& name, const std::vector<Value>& args, std::string& error) = 0;
    virtual bool getField(const std::string& object, const std::string& field, Value& out, std::string& error) = 0;
    virtual bool callFunction(const std::string& name, const std::vector<Value>& args, Value& out, std::string& error) = 0;
    virtual void print(const std::string& text) = 0;
};

struct Expr;
struct Stmt;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using Block = std::vector<StmtPtr>;

class Script {
public:
    Script();
    ~Script();
    Script(Script&&) noexcept;
    Script& operator=(Script&&) noexcept;

    // Parses the source. On failure returns false; error() holds "line N: ...".
    bool compile(const std::string& source);

    // Runs the top-level statements (var declarations) once.
    bool runInit(Host& host);
    // Runs an `on <event>` handler if the script has one. Returns false on a
    // runtime error (the script then stays failed until recompiled).
    bool fire(const std::string& event, Host& host);
    bool hasHandler(const std::string& event) const;

    void setVariable(const std::string& name, Value value) { globals_[name] = std::move(value); }

    bool failed() const { return failed_; }
    const std::string& error() const { return error_; }

private:
    Block init_;
    std::unordered_map<std::string, Block> handlers_;
    std::unordered_map<std::string, Value> globals_;
    bool failed_ = false;
    std::string error_;
    bool execute(const Block& block, Host& host);
};

// Short reference shown in the editor's script window.
const char* languageReference();

} // namespace rs
