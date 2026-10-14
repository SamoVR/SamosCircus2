#pragma once

#include "Vertex.h"
#include <string>
#include <vector>

std::vector<Vertex> createCubeVertices();

std::vector<Vertex> createSphereVertices(float radius = 0.5f, int stacks = 16, int slices = 32);

std::vector<Vertex> createTriangle3DVertices();

std::vector<Vertex> createFullScreenQuadVertices();

// ---- scene-file mesh encoding (keeps scene JSON small)
// Name of the built-in primitive these vertices are ("cube", "sphere",
// "triangle"), or "" for any other mesh. Compares positions and UVs only.
const char* identifyPrimitive(const std::vector<Vertex>& vertices);
// Vertices of a built-in primitive by name; empty for an unknown name.
std::vector<Vertex> primitiveVertices(const std::string& name);
