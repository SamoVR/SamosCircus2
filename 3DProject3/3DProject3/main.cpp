// Samo_VR
// Draws a rotating cube on screen using OpenGL
// Updated: Rendering a single face of a cube

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 mvp; // Combined Model-View-Projection matrix

void main()
{
    gl_Position = mvp * vec4(aPos, 1.0);
    vertexColor = aColor;
}

)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vertexColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(void)
{
    GLFWwindow* window;

    

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello Cube", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to init GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Vertex data for a quad (two triangles)
    float verticies[] = {
        // Positions           // Colors
        // Back face (Red)
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // Bottom-left
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // Bottom-right
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // Top-right
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // Top-left

        // Front face (Green)
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // Bottom-left
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // Bottom-right
         0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // Top-right
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // Top-left

        // Left face (Blue)
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, // Bottom-left
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // Bottom-right
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // Top-right
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, // Top-left

        // Right face (Yellow)
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, // Bottom-left
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // Bottom-right
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // Top-right
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, // Top-left

         // Bottom face (Magenta)
         -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, // Bottom-left
          0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, // Bottom-right
          0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f, // Top-right
         -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f, // Top-left

         // Top face (Cyan)
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f, // Bottom-left
          0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f, // Bottom-right
          0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f, // Top-right
         -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f  // Top-left
    };

    // Indices for the two triangles that make up the quad
    unsigned int indices[] = {
        // Back face
        0, 1, 2,
        2, 3, 0,

        // Front face
        4, 5, 6,
        6, 7, 4,

        // Left face
        8, 9, 10,
        10, 11, 8,

        // Right face
        12, 13, 14,
        14, 15, 12,

        // Bottom face
        16, 17, 18,
        18, 19, 16,

        // Top face
        20, 21, 22,
        22, 23, 20
    };

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Create the shader program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Delete the shaders as they're linked into our program now and no longer needed
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create a Vertex Buffer Object (VBO), Vertex Array Object (VAO), and Element Buffer Object (EBO)
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind the Vertex Array Object first, then bind and set the vertex buffer(s) and index buffer
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // Unbind the VAO
    glBindVertexArray(0);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) // MAIN LOOP
    {
        // Clear both color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = (float)width / (float)height;


        // Calculate the transformation matrices
        float time = (float)glfwGetTime();
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), time, glm::vec3(0.5f, 1.0f, 0.0f)); // Rotate over time
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));   // Move the scene back
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 640.0f / 480.0f, 0.1f, 100.0f);

        // Combine them into the MVP matrix
        glm::mat4 mvp = projection * view * model;

        // Send the MVP matrix to the shader
        unsigned int mvpLoc = glGetUniformLocation(shaderProgram, "mvp");
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

        // Render the cube
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }


    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
