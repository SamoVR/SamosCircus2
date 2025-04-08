#include "Scene.h"

Scene::Scene() {
}

void Scene::addObject(const Object& object) {
    objects.push_back(object);
}

std::vector<Object>& Scene::getObjects() {
    return objects;  // Non-const reference to allow modifications
}
