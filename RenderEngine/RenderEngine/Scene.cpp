#include "Scene.h"

Scene::Scene() {
}

void Scene::addObject(Object* object) {
    objects.push_back(object);
}

void Scene::removeObject(size_t index) {
    if (index < objects.size()) {
        delete objects[index]; // Optional: if Scene owns memory
        objects.erase(objects.begin() + index);
    }
}

std::vector<Object*>& Scene::getObjects() {
    return objects;
}

void Scene::addInternalObject(Object* object) {
    internalObjects.push_back(object);
}

const std::vector<Object*>& Scene::getInternalObjects() const {
    return internalObjects;
}
