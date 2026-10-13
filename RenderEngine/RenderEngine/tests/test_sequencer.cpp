#include "AnimationSequencer.h"
#include "Geometry.h"
#include <cassert>
#include <cstdio>
#include <set>

// GL stubs (Object construction needs them; same approach as test_core)
static GLuint n1 = 1;
static void APIENTRY gv(GLsizei n, GLuint* a) { for (GLsizei i = 0; i < n; ++i) a[i] = n1++; }
static void APIENTRY dv(GLsizei, const GLuint*) {}
static void APIENTRY bv(GLuint) {}
static void APIENTRY bb(GLenum, GLuint) {}
static void APIENTRY bd(GLenum, GLsizeiptr, const void*, GLenum) {}
static void APIENTRY ev(GLuint) {}
static void APIENTRY ap(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) {}
PFNGLGENVERTEXARRAYSPROC __glewGenVertexArrays = gv;
PFNGLDELETEVERTEXARRAYSPROC __glewDeleteVertexArrays = dv;
PFNGLBINDVERTEXARRAYPROC __glewBindVertexArray = bv;
PFNGLGENBUFFERSPROC __glewGenBuffers = gv;
PFNGLDELETEBUFFERSPROC __glewDeleteBuffers = dv;
PFNGLBINDBUFFERPROC __glewBindBuffer = bb;
PFNGLBUFFERDATAPROC __glewBufferData = bd;
PFNGLENABLEVERTEXATTRIBARRAYPROC __glewEnableVertexAttribArray = ev;
PFNGLVERTEXATTRIBPOINTERPROC __glewVertexAttribPointer = ap;

#define CHECK(cond, msg) do { if (!(cond)) { std::printf("FAIL: %s  (line %d)\n", msg, __LINE__); return 1; } else std::printf("[PASS] %s\n", msg); } while (0)
static bool near3(const glm::vec3& a, const glm::vec3& b) { return glm::length(a - b) < 1e-4f; }

int main() {
    using AS = AnimationSequencer;
    auto cube = createCubeVertices();
    Object obj("Cube", cube);
    obj.position = {1, 2, 3};
    obj.rotation = {10, 20, 30};
    obj.scale = {2, 2, 2};

    AS seq;
    CHECK(seq.sequencer.GetItemTypeCount() == 1 && std::string(seq.sequencer.GetItemTypeName(0)) == "Object",
          "item type count is derived from the real name table");
    CHECK(std::string(seq.sequencer.GetItemTypeName(4)) == "Unknown" && std::string(seq.sequencer.GetItemTypeName(-1)) == "Unknown",
          "out-of-range type index is bounds-checked (was an out-of-bounds read)");

    int idx = seq.addItemForObject(&obj, 10, 50);
    CHECK(near3(seq.items[idx].curvesByMode[2].evaluate(25), {2, 2, 2}), "Scale curves are seeded at the object's scale, not 0");
    CHECK(near3(seq.items[idx].curvesByMode[0].evaluate(25), {1, 2, 3}), "Position curves seeded at the object's position");

    // --- animation applies without any UI / expansion (was only applied while the row was expanded) ---
    AS::Item& item = seq.items[idx];
    item.mode = AS::TransformMode::Position;
    auto& pos = item.curvesByMode[0];
    pos.mPts[0][0] = ImVec2(10, 0);  pos.mPts[0][1] = ImVec2(50, 40);  // X: 0 -> 40 over frames 10..50
    pos.mDirty = true;
    seq.currentFrame = 30;
    seq.applyToObjects();
    CHECK(std::abs(obj.position.x - 20.0f) < 1e-4f, "applyToObjects drives the object with NO row expanded (frame 30 -> x=20)");

    // --- start/end frame respected ---
    obj.position = {7, 7, 7};
    seq.currentFrame = 5;  seq.applyToObjects();
    CHECK(obj.position == glm::vec3(7, 7, 7), "frame before the item's start leaves the object untouched");
    seq.currentFrame = 60; seq.applyToObjects();
    CHECK(obj.position == glm::vec3(7, 7, 7), "frame after the item's end leaves the object untouched");

    // --- does not fight manual edits while the frame is unchanged ---
    seq.currentFrame = 30; seq.applyToObjects();
    obj.position = {99, 99, 99};
    seq.applyToObjects();
    CHECK(obj.position == glm::vec3(99, 99, 99), "no re-apply on the same frame (manual gizmo edits survive)");
    seq.currentFrame = 31; seq.applyToObjects();
    CHECK(obj.position != glm::vec3(99, 99, 99), "changing frame re-applies the animation");

    // --- hold behaviour + degenerate curves ---
    AS::RampEdit r(0, 1);
    r.mPts[0][0] = ImVec2(10, 5); r.mPts[0][1] = ImVec2(20, 9); r.mPointCount[0] = 2;
    CHECK(r.evaluate(0).x == 5.0f && r.evaluate(99).x == 9.0f, "before/after the curve holds the end values (was: 0)");
    r.mPointCount[1] = 0; r.mRest = {0, 3, 0};
    CHECK(r.evaluate(5).y == 3.0f, "a curve with zero points no longer underflows; uses the rest value");
    r.mPts[2][0] = ImVec2(10, 1); r.mPts[2][1] = ImVec2(10, 8); r.mPointCount[2] = 2;
    float z = r.evaluate(10).z;
    CHECK(std::isfinite(z), "two points on the same frame do not divide by zero");

    // --- EditPoint returns the dragged point's new index after sorting ---
    AS::RampEdit e(0, 1);
    e.mPts[0][0] = ImVec2(0, 0); e.mPts[0][1] = ImVec2(100, 1); e.mPointCount[0] = 2;
    int newIdx = e.EditPoint(0, 0, ImVec2(150, 0.5f));   // drag point 0 past point 1
    CHECK(newIdx == 1 && e.mPts[0][1].x == 150.0f, "EditPoint returns the new index when a drag reorders points");

    // --- item lifecycle ---
    seq.addItemForObject(&obj);
    seq.sequencer.Del(99); seq.sequencer.Del(-1); seq.sequencer.Duplicate(99);
    CHECK(seq.items.size() == 2, "Del/Duplicate ignore out-of-range indices instead of corrupting the vector");
    seq.removeItemsForObject(&obj);
    CHECK(seq.items.empty(), "removeItemsForObject drops every item referencing the object");
    std::printf("\nALL SEQUENCER TESTS PASSED\n");
    return 0;
}
