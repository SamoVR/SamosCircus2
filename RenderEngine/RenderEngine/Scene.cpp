#include "Scene.h"

Scene::Scene() {
}

void Scene::addObject(Object* object) {
    objects.push_back(object);
}

void Scene::removeObject(size_t index,Object* object) {
    if (!object) {
        if (index < objects.size()) {
            delete objects[index]; // Optional: if Scene owns memory
            objects.erase(objects.begin() + index);
        }
    }
    else {
        auto it = std::find(objects.begin(), objects.end(), object);
        if (it != objects.end()) {
            objects.erase(it);
        }

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

void Scene::saveToFile(const std::string& filename) {
    nlohmann::json j;

    // Serialize all objects in the scene, including renamed ones
    for (const auto& obj : objects) {
        // Ensure the object is fully updated, including vertices and other properties
        j["objects"].push_back(obj->toJSON());
    }

    // Write JSON to file
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << j.dump(4);  // Pretty-print with 4 spaces indentation
        outFile.close();
    }
    else {
        std::cerr << "Failed to open file for saving: " << filename << std::endl;
    }
}

void Scene::loadFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open file for loading: " << filename << std::endl;
        return;
    }

    nlohmann::json j;
    inFile >> j;

    // Clear current objects
    objects.clear();

    // Deserialize objects and add them to the scene
    // Deserialize all objects first
    std::unordered_map<std::string, Object*> nameToObject;

    for (const auto& objJson : j["objects"]) {
        Object* obj = new Object(objJson["name"], std::vector<Vertex>(), nullptr);

        if (objJson.contains("vertices")) {
            std::vector<Vertex> vertices = fromJSONToVertices(objJson["vertices"]);
            obj->setVertices(vertices);
        }

        obj->fromJSON(objJson);
        nameToObject[obj->name] = obj;
        objects.push_back(obj);
    }

    // After all objects are created, set up parent-child relationships
    for (const auto& objJson : j["objects"]) {
        std::string objName = objJson["name"];
        if (objJson.contains("parent")) {
            std::string parentName = objJson["parent"];
            Object* obj = nameToObject[objName];
            Object* parentObj = nameToObject[parentName];

            if (obj && parentObj) {
                obj->setParent(parentObj);  // This should also add the child to the parent
            }
        }
    }
}

std::vector<Vertex> Scene::fromJSONToVertices(const nlohmann::json& jsonVertices) {
    std::vector<Vertex> vertices;

    for (const auto& v : jsonVertices) {
        Vertex vertex;
        vertex.position = glm::vec3(v["position"][0], v["position"][1], v["position"][2]);
        vertex.texCoord = glm::vec2(v["texCoord"][0], v["texCoord"][1]);  // Use texCoord instead of texCoords
        vertices.push_back(vertex);
    }

    return vertices;
}

