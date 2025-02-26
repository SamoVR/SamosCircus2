#include "Player.h"
#include "Camera.h"
#include "World.h"
#include "Block.h"
#include <iostream>

glm::ivec3 blockPos;
glm::vec3 hitPoint;

Player::Player(Camera* camera, World* world)
    : camera(camera), world(world) {
    if (!camera || !world) {
        throw std::invalid_argument("Camera or World cannot be null");
    }
}

void Player::Update() {

    /*
    if (camera->getBlockLookingAt(world, blockPos, hitPoint)) {
        std::cout << "Looking at block: " << blockPos.x << ", " << blockPos.y << ", " << blockPos.z << std::endl;
    }
    else {
        std::cout << "Not looking at any block." << std::endl;
    }
    */
}

void Player::PlaceBlock() {

    if (camera->getBlockLookingAt(world, blockPos, hitPoint)) {  // Updated function to return hitPoint
        // Compute normal of the block face being looked at
        glm::vec3 blockCenter = blockPos + glm::ivec3(1); // Center of the block
        glm::vec3 faceNormal = glm::normalize(hitPoint - blockCenter); // Direction from center to hitPoint

        // Determine the face based on the largest absolute component of faceNormal
        glm::ivec3 placeOffset(0);
        if (fabs(faceNormal.x) > fabs(faceNormal.y) && fabs(faceNormal.x) > fabs(faceNormal.z)) {
            placeOffset.x = (faceNormal.x > 0) ? 1 : -1;
        }
        else if (fabs(faceNormal.y) > fabs(faceNormal.x) && fabs(faceNormal.y) > fabs(faceNormal.z)) {
            placeOffset.y = (faceNormal.y > 0) ? 1 : -1;
        }
        else {
            placeOffset.z = (faceNormal.z > 0) ? 1 : -1;
        }

        glm::ivec3 placePos = blockPos + placeOffset;

        if (world->getBlockAt(placePos.x, placePos.y, placePos.z) == nullptr) {
            world->placeBlock(placePos.x, placePos.y, placePos.z, BlockType::GRASS);
            std::cout << "Placed block at: " << placePos.x << ", " << placePos.y << ", " << placePos.z << std::endl;
        }
        else {
            std::cout << "Cannot place block, position is occupied." << std::endl;
        }
    }
    else {
        std::cout << "No valid block to place against." << std::endl;
    }
}


void Player::MineBlock() {

    if (camera->getBlockLookingAt(world, blockPos, hitPoint)) {
        Block* block = world->getBlockAt(blockPos.x, blockPos.y, blockPos.z);
        if (block != nullptr && block->isSolid && block->breakTime >= 0) {
            world->placeBlock(blockPos.x, blockPos.y, blockPos.z, BlockType::AIR);
            std::cout << "Mined block at: " << blockPos.x << ", " << blockPos.y << ", " << blockPos.z << std::endl;
        }
    }
    else {
        std::cout << "No block to mine." << std::endl;
    }
}
