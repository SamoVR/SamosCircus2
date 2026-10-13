#pragma once
//
// CommandHistory.h -- owns the undo/redo stacks.
// Executing a new command after an undo clears the redo stack (standard
// editor semantics). Depth is capped so a long session can't grow forever.
//
#include "Command.h"
#include "Logger.h"
#include <vector>
#include <memory>
#include <string>

class CommandHistory {
public:
    static constexpr size_t kMaxHistory = 200;

    // Executes the command and records it. Callers should not call
    // command->execute() themselves.
    void execute(std::unique_ptr<Command> command) {
        command->execute();
        undoStack_.push_back(std::move(command));
        if (undoStack_.size() > kMaxHistory) undoStack_.erase(undoStack_.begin());
        redoStack_.clear();
    }

    bool canUndo() const { return !undoStack_.empty(); }
    bool canRedo() const { return !redoStack_.empty(); }

    void undo() {
        if (undoStack_.empty()) return;
        std::unique_ptr<Command> command = std::move(undoStack_.back());
        undoStack_.pop_back();
        LOG_INFO("CommandHistory", "Undo: " + command->description());
        command->undo();
        redoStack_.push_back(std::move(command));
    }

    void redo() {
        if (redoStack_.empty()) return;
        std::unique_ptr<Command> command = std::move(redoStack_.back());
        redoStack_.pop_back();
        LOG_INFO("CommandHistory", "Redo: " + command->description());
        command->execute();
        undoStack_.push_back(std::move(command));
    }

    std::string peekUndoDescription() const { return undoStack_.empty() ? "" : undoStack_.back()->description(); }
    std::string peekRedoDescription() const { return redoStack_.empty() ? "" : redoStack_.back()->description(); }

    void clear() { undoStack_.clear(); redoStack_.clear(); }

private:
    std::vector<std::unique_ptr<Command>> undoStack_;
    std::vector<std::unique_ptr<Command>> redoStack_;
};
