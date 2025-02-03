#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include "Camera.h"
#include "World.h"

class Player {
public:
    Player(Camera* camera, World* world);

    void Update(float deltaTime);
    void PlaceBlock();
    void MineBlock();

private:
    Camera* camera;
    World* world;
};

#endif
