#include "Scene.h"
#include "Logger.h"
#include <fstream>
#include <unordered_map>
#include <algorithm>

Scene::Scene() {}

void Scene::addObject(std::unique_ptr<Object> object) {
    if (object) objects_.push_back(std::move(object));
}

void Scene::addInternalObject(std::unique_ptr<Object> object) {
    if (object) internalObjects_.push_back(std::move(object));
}

void Scene::detachFromHierarchy(Object* object) {
    if (!object) return;

    object->removeParent();

    // Children are detached, not deleted: silently destroying everything
    // inside a "group" as a side effect of deleting the group would be a
    // nasty surprise in an editor.
    for (Object* child : std::vector<Object*>(object->children)) { // copy: removeChild mutates the list
        object->removeChild(child);
    }
}

bool Scene::removeObjectAt(size_t index) {
    if (index >= objects_.size()) {
        LOG_WARN("Scene", "removeObjectAt: index " + std::to_string(index) + " out of range");
        return false;
    }
    detachFromHierarchy(objects_[index].get());
    objects_.erase(objects_.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

bool Scene::removeObject(Object* object) {
    if (!object) return false;

    auto it = std::find_if(objects_.begin(), objects_.end(),
        [object](const std::unique_ptr<Object>& p) { return p.get() == object; });

    if (it == objects_.end()) {
        LOG_WARN("Scene", "removeObject: object not found in scene");
        return false;
    }
    detachFromHierarchy(object);
    objects_.erase(it);
    return true;
}

void Scene::clear() {
    objects_.clear(); // unique_ptr destructors free everything
}

void Scene::clearAll() {
    objects_.clear();
    internalObjects_.clear();
}

bool Scene::contains(const Object* object) const {
    return indexOf(object) != npos;
}

size_t Scene::indexOf(const Object* object) const {
    for (size_t i = 0; i < objects_.size(); ++i) {
        if (objects_[i].get() == object) return i;
    }
    return npos;
}

std::unique_ptr<Object> Scene::extractObject(Object* object, size_t* outIndex) {
    const size_t index = indexOf(object);
    if (index == npos) return nullptr;
    if (outIndex) *outIndex = index;
    detachFromHierarchy(object);
    std::unique_ptr<Object> owned = std::move(objects_[index]);
    objects_.erase(objects_.begin() + static_cast<std::ptrdiff_t>(index));
    return owned;
}

void Scene::insertObjectAt(size_t index, std::unique_ptr<Object> object) {
    if (!object) return;
    index = std::min(index, objects_.size());
    objects_.insert(objects_.begin() + static_cast<std::ptrdiff_t>(index), std::move(object));
}

nlohmann::json Scene::buildJson() const {
    nlohmann::json j;
    j["objects"] = nlohmann::json::array();
    for (const auto& obj : objects_) j["objects"].push_back(obj->toJSON());
    return j;
}

std::string Scene::toJsonString() const {
    return buildJson().dump();
}

std::vector<Object*> Scene::getObjects() const {
    std::vector<Object*> view;
    view.reserve(objects_.size());
    for (const auto& ptr : objects_) view.push_back(ptr.get());
    return view;
}

std::vector<Object*> Scene::getInternalObjects() const {
    std::vector<Object*> view;
    view.reserve(internalObjects_.size());
    for (const auto& ptr : internalObjects_) view.push_back(ptr.get());
    return view;
}

Object* Scene::findByName(const std::string& name) const {
    for (const auto& ptr : objects_) {
        if (ptr->name == name) return ptr.get();
    }
    return nullptr;
}

bool Scene::saveToFile(const std::string& filename) const {
    const nlohmann::json j = buildJson();

    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        LOG_ERROR("Scene", "Failed to open file for saving: " + filename);
        return false;
    }
    outFile << j.dump(4);
    outFile.close();
    if (outFile.fail()) {
        LOG_ERROR("Scene", "Error while writing scene file: " + filename);
        return false;
    }
    LOG_INFO("Scene", "Saved " + std::to_string(objects_.size()) + " object(s) to " + filename);
    return true;
}

bool Scene::loadFromFile(const std::string& filename, Texture* sharedTexture) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        LOG_ERROR("Scene", "Failed to open file for loading: " + filename);
        return false;
    }

    nlohmann::json j;
    try {
        inFile >> j;
    } catch (const nlohmann::json::exception& e) {
        LOG_ERROR("Scene", "Failed to parse scene file '" + filename + "': " + e.what());
        return false;
    }

    if (!j.contains("objects") || !j["objects"].is_array()) {
        LOG_ERROR("Scene", "Scene file '" + filename + "' has no valid 'objects' array; nothing loaded.");
        return false;
    }

    // Build the whole new object list first and only swap it in once parsing
    // has succeeded, so a malformed file can no longer wipe the current scene.
    std::vector<std::unique_ptr<Object>> loaded;
    std::unordered_map<std::string, Object*> nameToObject;
    std::vector<std::pair<std::string, std::string>> pendingParents; // (child, parent)

    try {
        for (const auto& objJson : j["objects"]) {
            if (!objJson.contains("name")) {
                LOG_WARN("Scene", "Skipping an object entry with no 'name' in " + filename);
                continue;
            }

            // Two objects sharing a name used to silently overwrite each other
            // in the name map, making the first unreachable and sending any
            // parent reference to the wrong object. Duplicates are renamed.
            const std::string desiredName = objJson["name"].get<std::string>();
            std::string finalName = desiredName;
            for (int suffix = 1; nameToObject.count(finalName) != 0; ++suffix) {
                finalName = desiredName + "_" + std::to_string(suffix);
            }
            if (finalName != desiredName) {
                LOG_WARN("Scene", "Duplicate object name '" + desiredName + "' in " + filename
                    + " -- renamed to '" + finalName + "'.");
            }

            auto obj = std::make_unique<Object>(finalName, std::vector<Vertex>(), nullptr);
            obj->fromJSON(objJson);
            obj->setName(finalName); // fromJSON restored the original name

            // Shared (non-owned) texture: re-attach instead of reloading.
            if (objJson.contains("texture") && !objJson.value("textureOwned", true)) {
                const std::string savedPath = objJson["texture"].get<std::string>();
                if (sharedTexture && sharedTexture->isValid() && sharedTexture->getFilePath() == Texture::normalizePath(savedPath)) {
                    obj->setTexture(sharedTexture);
                } else {
                    auto own = std::make_unique<Texture>(savedPath);
                    obj->setOwnedTexture(std::move(own));
                }
            }

            nameToObject[finalName] = obj.get();
            if (objJson.contains("parent")) {
                pendingParents.emplace_back(finalName, objJson["parent"].get<std::string>());
            }
            loaded.push_back(std::move(obj));
        }
    } catch (const nlohmann::json::exception& e) {
        LOG_ERROR("Scene", "Malformed scene file '" + filename + "': " + e.what() + " -- current scene left untouched.");
        return false;
    }

    // Parents are resolved after every object exists (a child may appear
    // before its parent in the file).
    for (const auto& link : pendingParents) {
        auto childIt = nameToObject.find(link.first);
        auto parentIt = nameToObject.find(link.second);
        if (childIt == nameToObject.end()) continue;
        if (parentIt == nameToObject.end()) {
            LOG_WARN("Scene", "'" + link.first + "' references missing parent '" + link.second + "' -- left unparented.");
            continue;
        }
        childIt->second->setParent(parentIt->second); // rejects cycles
    }

    objects_ = std::move(loaded); // old objects freed here
    LOG_INFO("Scene", "Loaded " + std::to_string(objects_.size()) + " object(s) from " + filename);
    return true;
}
