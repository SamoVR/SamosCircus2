#include "Scene.h"

Scene::Scene() {
}

void Scene::addObject(const Object& object) {
    objects.push_back(object);
}

const std::vector<Object>& Scene::getObjects() const {
    return objects;
}
