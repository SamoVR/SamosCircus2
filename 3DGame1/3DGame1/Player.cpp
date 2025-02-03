#include "Player.h"
#include <iostream>

Player::Player(Camera* camera, World* world)
    : camera(camera), world(world) {
    if (!camera || !world) {
        throw std::invalid_argument("Camera or World cannot be null");
    }
}

void Player::Update() {
    glm::ivec3 blockPos;
    Block* blockPtr = nullptr;

    // Use -> because camera is a pointer
    if (camera->getTargetBlock(world, blockPos, blockPtr)) {
        std::cout << "Looking at block at: " << blockPos.x << ", " << blockPos.y << ", " << blockPos.z << std::endl;
    }
    else {
        std::cout << "No block in sight!" << std::endl;
    }
}

void Player::PlaceBlock() {
    // Implementation needed
}

void Player::MineBlock() {
    // Implementation needed
}
