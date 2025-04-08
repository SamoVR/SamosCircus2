#pragma once

#include "Object.h"
#include <vector>

class Scene {
public:
    Scene();

    void addObject(const Object& object);
    std::vector<Object>& getObjects();  // Return non-const reference

private:
    std::vector<Object> objects;
};
