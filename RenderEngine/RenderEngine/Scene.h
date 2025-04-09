#pragma once

#include "Object.h"
#include <vector>

class Scene {
public:
    Scene();

    void addObject(Object* object);  // store pointer
    void removeObject(size_t index);
    std::vector<Object*>& getObjects();  // return list of pointers

private:
    std::vector<Object*> objects;
};

