#pragma once
//
// Command.h -- base interface for undo/redo (see CommandHistory.h).
// Edit > Undo / Redo were empty menu stubs in the original code.
//
#include <string>

class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;   // do (and redo) the action
    virtual void undo() = 0;      // fully reverse execute()
    virtual std::string description() const = 0; // e.g. "Move Cube", for menu labels
};
