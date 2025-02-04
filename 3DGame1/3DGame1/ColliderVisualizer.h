#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ColliderVisualizer {
public:
    static GLuint VAO; // Only declare, no definition
    static GLuint VBO; // Only declare, no definition

    static void setup();
    void render(const glm::mat4& modelMatrix, GLuint shaderProgram, const glm::vec3& colliderMin, const glm::vec3& colliderMax);
};
