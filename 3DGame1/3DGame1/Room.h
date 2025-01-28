#ifndef ROOM_H
#define ROOM_H

#include <GL/glew.h>

class Room {
public:
    Room();
    ~Room();

    void init();
    void render();

private:
    GLuint VAO, VBO;
};

#endif
