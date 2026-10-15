#pragma once
//
// Commands.h -- concrete undo/redo commands for the editor (see Command.h and
// CommandHistory.h). Header-only so no project file changes are needed.
//
// Ownership rule that keeps raw Object* pointers in commands valid: any
// operation that removes an Object from the Scene is itself a command that
// takes ownership of the Object while it sits in the history. Commands are
// only ever undone/redone in strict order, so a pointer a command holds is
// always alive when that command runs.
//
#include "Command.h"
#include "Scene.h"
#include "AnimationSequencer.h"
#include <memory>
#include <string>
#include <vector>

// The user-editable state of an Object (what the Properties panel and the
// gizmo change). Colour is written directly, NOT through setColor(), which
// would also drop the texture.
struct ObjectState {
    std::string name;
    glm::vec3 position{0.0f}, rotation{0.0f}, scale{1.0f}, color{1.0f};

    static ObjectState capture(const Object& o) {
        return { o.name, o.position, o.rotation, o.scale, o.color };
    }
    void applyTo(Object& o) const {
        o.setName(name);
        o.position = position;
        o.rotation = rotation;
        o.scale = scale;
        o.color = color;
    }
    bool operator==(const ObjectState& r) const {
        return name == r.name && position == r.position && rotation == r.rotation && scale == r.scale && color == r.color;
    }
    bool operator!=(const ObjectState& r) const { return !(*this == r); }
};

class ModifyObjectCommand : public Command {
public:
    ModifyObjectCommand(Object* object, ObjectState before, ObjectState after, std::string description)
        : object_(object), before_(std::move(before)), after_(std::move(after)), description_(std::move(description)) {}
    void execute() override { after_.applyTo(*object_); }
    void undo() override { before_.applyTo(*object_); }
    std::string description() const override { return description_; }
private:
    Object* object_;
    ObjectState before_, after_;
    std::string description_;
};

// Several commands as one undo step (e.g. a gizmo drag over a multi-selection).
class CompositeCommand : public Command {
public:
    CompositeCommand(std::string description, std::vector<std::unique_ptr<Command>> commands)
        : description_(std::move(description)), commands_(std::move(commands)) {}
    void execute() override { for (auto& c : commands_) c->execute(); }
    void undo() override { for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) (*it)->undo(); }
    std::string description() const override { return description_; }
private:
    std::string description_;
    std::vector<std::unique_ptr<Command>> commands_;
};

class AddObjectCommand : public Command {
public:
    AddObjectCommand(Scene& scene, std::unique_ptr<Object> object, std::string description)
        : scene_(scene), owned_(std::move(object)), raw_(owned_.get()), description_(std::move(description)) {}
    void execute() override { scene_.addObject(std::move(owned_)); }
    void undo() override { owned_ = scene_.extractObject(raw_); }
    std::string description() const override { return description_; }
    Object* object() const { return raw_; }
private:
    Scene& scene_;
    std::unique_ptr<Object> owned_; // non-null only while the object is NOT in the scene
    Object* raw_;
    std::string description_;
};

class RemoveObjectCommand : public Command {
public:
    RemoveObjectCommand(Scene& scene, AnimationSequencer& animator, Object* object)
        : scene_(scene), animator_(animator), raw_(object) {}

    void execute() override {
        parent_ = raw_->parent;
        children_ = raw_->children;
        removedItems_.clear();
        for (const auto& item : animator_.items) {
            if (item.animatedObject == raw_) removedItems_.push_back(item);
        }
        animator_.removeItemsForObject(raw_);
        owned_ = scene_.extractObject(raw_, &index_); // detaches it from parent and children
    }

    void undo() override {
        if (!owned_) return;
        scene_.insertObjectAt(index_, std::move(owned_));
        if (parent_ && scene_.contains(parent_)) raw_->setParent(parent_);
        for (Object* child : children_) {
            if (scene_.contains(child)) child->setParent(raw_);
        }
        for (const auto& item : removedItems_) animator_.items.push_back(item);
    }

    std::string description() const override { return "Delete " + (raw_ ? name_() : std::string("Object")); }

private:
    std::string name_() const { return owned_ ? owned_->name : raw_->name; }

    Scene& scene_;
    AnimationSequencer& animator_;
    Object* raw_;
    std::unique_ptr<Object> owned_;
    size_t index_ = 0;
    Object* parent_ = nullptr;
    std::vector<Object*> children_;
    std::vector<AnimationSequencer::Item> removedItems_;
};

// Re-parenting that keeps the object where it is in the world: `before` and
// `after` are its local transforms for the old and new parent.
class ReparentCommand : public Command {
public:
    ReparentCommand(Object* object, Object* oldParent, Object* newParent, ObjectState before, ObjectState after)
        : object_(object), oldParent_(oldParent), newParent_(newParent), before_(std::move(before)), after_(std::move(after)) {}
    void execute() override { object_->setParent(newParent_); after_.applyTo(*object_); }
    void undo() override { object_->setParent(oldParent_); before_.applyTo(*object_); }
    std::string description() const override { return "Reparent " + object_->name; }
private:
    Object* object_;
    Object* oldParent_;
    Object* newParent_;
    ObjectState before_, after_;
};
