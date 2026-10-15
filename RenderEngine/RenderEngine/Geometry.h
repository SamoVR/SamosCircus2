#pragma once

#include "Vertex.h"
#include <vector>

std::vector<Vertex> createCubeVertices();

std::vector<Vertex> createSphereVertices(float radius = 0.5f, int stacks = 16, int slices = 32);

std::vector<Vertex> createTriangle3DVertices();

std::vector<Vertex> createFullScreenQuadVertices();
