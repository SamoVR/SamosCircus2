#include "Player.h"
#include <iostream>

Player::Player(Camera* camera, World* world)
    : camera(camera), world(world) {
    if (!camera || !world) {
        throw std::invalid_argument("Camera or World cannot be null");
    }
}


void Player::PlaceBlock() {
    glm::ivec3 targetBlock, placePos;
    // Get the block that the player is looking at and the position where we can place the new block
    if (camera->GetBlockLookingAt(world, targetBlock, placePos)) {
        // Retrieve the type of block at the target position
        Block* block = world->getChunkAt(placePos.x, placePos.z)->getBlock(placePos.x, placePos.y, placePos.z);
        std::cout << placePos.x, placePos.y, placePos.z;

        if (block != nullptr) {
            BlockType blockType = block->getType();  // Assuming Block has a getType() method to return the BlockType

            // If you want to place a new block, do it here
            world->placeBlock(targetBlock.x, targetBlock.y, targetBlock.z, blockType);
            std::cout << "Placed block at: " << placePos.x << ", " << placePos.y << ", " << placePos.z << std::endl;
        }
    }
}

void Player::MineBlock() {
    glm::ivec3 targetBlock, placePos;
    // Get the block that the player is looking at
    if (camera->GetBlockLookingAt(world, targetBlock, placePos)) {
        Block* block = world->getChunkAt(targetBlock.x, targetBlock.z)->getBlock(targetBlock.x, targetBlock.y, targetBlock.z);

        if (block != nullptr) {
            BlockType blockType = block->getType(); // Extract the block type
            // Mine the block (replace it with air)
            world->placeBlock(targetBlock.x, targetBlock.y, targetBlock.z, blockType);
            std::cout << "Mined block at: " << targetBlock.x << ", " << targetBlock.y << ", " << targetBlock.z << std::endl;
        }
    }
}
