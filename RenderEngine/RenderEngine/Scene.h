#pragma once

#include "Object.h"
#include <vector>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

// OWNERSHIP: Scene is the single owner of every Object in it (unique_ptr), so
// "who deletes this" is enforced by the compiler instead of by convention.
//
// The original removeObject() had two overloads with DIFFERENT ownership
// semantics (index version deleted the object, pointer version only
// detached it). Interface.cpp used both in one delete operation, leaking
// children and leaving a still-selected child with a dangling parent pointer
// -> use-after-free in the gizmo. There is now exactly one way to remove an
// object, and it always detaches it from the hierarchy first.
//
// getObjects()/getInternalObjects() return non-owning snapshots of raw
// pointers for the rest of the codebase. They stay valid until the object is
// removed or the scene is cleared/loaded -- callers holding Object* across
// those operations (selection, animation items) must drop them first.
class Scene {
public:
    Scene();

    void addObject(std::unique_ptr<Object> object);
    void addInternalObject(std::unique_ptr<Object> object);

    // Destroys the object. Its children are detached (not deleted) first so
    // nothing is left pointing at freed memory. Returns false if not found.
    bool removeObjectAt(size_t index);
    bool removeObject(Object* object);

    // Destroys every user object (internal objects are kept).
    void clear();
    // Destroys user AND internal objects. Engine calls this during shutdown,
    // while the GL context is still alive (GL resources must not outlive it).
    void clearAll();

    bool contains(const Object* object) const;
    // Index of the object in the user list, or npos.
    static constexpr size_t npos = static_cast<size_t>(-1);
    size_t indexOf(const Object* object) const;

    // Removes the object from the scene WITHOUT destroying it and hands
    // ownership to the caller (used by undo/redo commands). The object is
    // detached from the hierarchy first, like removeObject().
    std::unique_ptr<Object> extractObject(Object* object, size_t* outIndex = nullptr);
    // Inserts at `index` (clamped), restoring list order on undo.
    void insertObjectAt(size_t index, std::unique_ptr<Object> object);

    // Compact JSON of the whole scene; used to detect unsaved changes.
    std::string toJsonString() const;

    std::vector<Object*> getObjects() const;
    std::vector<Object*> getInternalObjects() const;
    size_t objectCount() const { return objects_.size(); }
    Object* findByName(const std::string& name) const;

    // Both return false on failure (and log the reason).
    bool saveToFile(const std::string& filename) const;
    // `sharedTexture`: objects saved as using a shared texture with the same
    // file path get it re-attached instead of loading their own copy.
    bool loadFromFile(const std::string& filename, Texture* sharedTexture = nullptr);

private:
    std::vector<std::unique_ptr<Object>> objects_;
    std::vector<std::unique_ptr<Object>> internalObjects_; // not user-visible

    static void detachFromHierarchy(Object* object);
    nlohmann::json buildJson() const;
};
