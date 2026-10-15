#pragma once

#include "Object.h"
#include <vector>
#include <nlohmann/json.hpp>
#include <fstream>

class Scene {
public:
    Scene();

    void addObject(Object* object);  // store pointer
    void removeObject(size_t index, Object* object = nullptr);
    std::vector<Object*>& getObjects();  // return list of pointers
    void addInternalObject(Object* object);
    const std::vector<Object*>& getInternalObjects() const;
    
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);
    std::vector<Vertex> fromJSONToVertices(const nlohmann::json& jsonVertices);

private:
    std::vector<Object*> objects;
    std::vector<Object*> internalObjects; // Not exposed to player logic

};

