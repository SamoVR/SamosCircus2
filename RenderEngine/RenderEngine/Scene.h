#pragma once

#include "Object.h"
#include <vector>

class Scene {
public:
    Scene();

    void addObject(const Object& object);
    const std::vector<Object>& getObjects() const;

private:
    std::vector<Object> objects;
};
