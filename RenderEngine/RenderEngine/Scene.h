#pragma once

#include "Object.h"
#include <vector>

class Scene {
public:
    Scene();

    void addObject(Object* object);  // store pointer
    void removeObject(size_t index);
    std::vector<Object*>& getObjects();  // return list of pointers
    void addInternalObject(Object* object);
    const std::vector<Object*>& getInternalObjects() const;
    

private:
    std::vector<Object*> objects;
    std::vector<Object*> internalObjects; // Not exposed to player logic

};

