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
    for (const auto& objJson : j["objects"]) {
        Object* obj = new Object(objJson["name"], std::vector<Vertex>(), nullptr);  // Pass an empty vertex list and nullptr for texture

        // Check if the "vertices" key exists, and if so, deserialize it
        if (objJson.contains("vertices")) {
            std::vector<Vertex> vertices = fromJSONToVertices(objJson["vertices"]);
            obj->setVertices(vertices); // Assuming Object has a `setVertices` method
        }

        // Deserialize other object properties
        obj->fromJSON(objJson);
        objects.push_back(obj);
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

