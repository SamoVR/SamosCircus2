// Runs the REAL Object.cpp / Scene.cpp (no mirrors). Only the GL entry points
// are replaced with stubs that track which VAO/VBO ids are alive, so ownership
// bugs (double delete, leak, shared handle) are directly observable.
#include "Object.h"
#include "Scene.h"
#include "Geometry.h"
#include <set>
#include <cassert>
#include <cstdio>
#include <fstream>

static std::set<GLuint> liveVAO, liveVBO;
static GLuint nextVAO = 1, nextVBO = 1;
static int badDeletes = 0;

static void APIENTRY stubGenVAO(GLsizei n, GLuint* a) { for (GLsizei i = 0; i < n; ++i) { a[i] = nextVAO++; liveVAO.insert(a[i]); } }
static void APIENTRY stubGenBuf(GLsizei n, GLuint* a) { for (GLsizei i = 0; i < n; ++i) { a[i] = nextVBO++; liveVBO.insert(a[i]); } }
static void APIENTRY stubDelVAO(GLsizei n, const GLuint* a) { for (GLsizei i = 0; i < n; ++i) if (a[i]) { if (!liveVAO.erase(a[i])) { ++badDeletes; std::printf("!! bad VAO delete %u\n", a[i]); } } }
static void APIENTRY stubDelBuf(GLsizei n, const GLuint* a) { for (GLsizei i = 0; i < n; ++i) if (a[i]) { if (!liveVBO.erase(a[i])) { ++badDeletes; std::printf("!! bad VBO delete %u\n", a[i]); } } }
static void APIENTRY stubBindVAO(GLuint) {}
static void APIENTRY stubBindBuf(GLenum, GLuint) {}
static void APIENTRY stubBufData(GLenum, GLsizeiptr, const void*, GLenum) {}
static void APIENTRY stubEnable(GLuint) {}
static void APIENTRY stubAttrib(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) {}

PFNGLGENVERTEXARRAYSPROC __glewGenVertexArrays = stubGenVAO;
PFNGLDELETEVERTEXARRAYSPROC __glewDeleteVertexArrays = stubDelVAO;
PFNGLBINDVERTEXARRAYPROC __glewBindVertexArray = stubBindVAO;
PFNGLGENBUFFERSPROC __glewGenBuffers = stubGenBuf;
PFNGLDELETEBUFFERSPROC __glewDeleteBuffers = stubDelBuf;
PFNGLBINDBUFFERPROC __glewBindBuffer = stubBindBuf;
PFNGLBUFFERDATAPROC __glewBufferData = stubBufData;
PFNGLENABLEVERTEXATTRIBARRAYPROC __glewEnableVertexAttribArray = stubEnable;
PFNGLVERTEXATTRIBPOINTERPROC __glewVertexAttribPointer = stubAttrib;

#define CHECK(cond, msg) do { if (!(cond)) { std::printf("FAIL: %s  (%s:%d)\n", msg, __FILE__, __LINE__); return 1; } else std::printf("[PASS] %s\n", msg); } while (0)

int main() {
    auto cube = createCubeVertices();

    // ---- 1. copy independence (RE-07) ----
    {
        auto a = std::make_unique<Object>("A", cube);
        auto b = std::make_unique<Object>(*a);
        CHECK(liveVAO.size() == 2 && liveVBO.size() == 2, "copy allocates its OWN VAO/VBO (2 live each)");
        a.reset();
        CHECK(liveVAO.size() == 1 && liveVBO.size() == 1, "deleting the original leaves the copy's buffers alive");
        b.reset();
        CHECK(liveVAO.empty() && liveVBO.empty() && badDeletes == 0, "deleting the copy frees the rest; no double delete, no leak");
    }

    // ---- 2. move leaves the source inert ----
    {
        Object a("A", cube);
        { Object b(std::move(a)); CHECK(liveVAO.size() == 1, "move transfers handles without allocating"); }
        CHECK(liveVAO.empty() && badDeletes == 0, "moved-from object's destructor does not double free");
    }

    // ---- 3. global transform + cycle rejection (RE-02 / graph safety) ----
    {
        Object g("G", cube), p("P", cube), c("C", cube);
        g.position = {100, 0, 0}; p.position = {10, 0, 0}; c.position = {1, 0, 0};
        p.setParent(&g); c.setParent(&p);
        glm::vec3 w(c.getGlobalTransform()[3]);
        CHECK(std::abs(w.x - 111.0f) < 1e-4f, "3-level hierarchy: child world position is (111,0,0)");
        g.setParent(&c); // would create a cycle
        CHECK(g.parent == nullptr, "parenting an object under its own descendant is rejected");
        g.setParent(&g);
        CHECK(g.parent == nullptr, "self-parenting is rejected");
        c.setParent(nullptr);
        CHECK(p.children.empty() && c.parent == nullptr, "re-parenting to null cleanly detaches both sides");
    }

    // ---- 4. Scene removal detaches hierarchy (RE-01 / RE-08) ----
    {
        Scene scene;
        scene.addObject(std::make_unique<Object>("Parent", cube));
        scene.addObject(std::make_unique<Object>("Child1", cube));
        scene.addObject(std::make_unique<Object>("Child2", cube));
        Object* parent = scene.findByName("Parent");
        Object* c1 = scene.findByName("Child1");
        Object* c2 = scene.findByName("Child2");
        c1->setParent(parent); c2->setParent(parent);
        CHECK(scene.removeObject(parent), "removeObject(parent) succeeds");
        CHECK(scene.objectCount() == 2, "children are NOT deleted along with the parent");
        CHECK(c1->parent == nullptr && c2->parent == nullptr, "no child is left with a dangling parent pointer");
        CHECK(!scene.removeObject(parent), "removing the already-removed pointer fails gracefully (no UB)");
        CHECK(liveVAO.size() == 2, "exactly the two surviving objects still hold GPU buffers");
        scene.clear();
        CHECK(liveVAO.empty() && liveVBO.empty() && badDeletes == 0, "clear() frees everything, no double delete");
    }

    // ---- 5. save / load round trip, duplicate names, normals ----
    {
        Scene s;
        s.addObject(std::make_unique<Object>("Cube", cube));
        s.addObject(std::make_unique<Object>("Cube", cube));   // duplicate name
        s.addObject(std::make_unique<Object>("Kid", cube));
        s.findByName("Kid")->setParent(s.findByName("Cube"));
        s.findByName("Kid")->position = {1, 2, 3};
        CHECK(s.saveToFile("/tmp/rt_scene.json"), "saveToFile reports success");

        Scene t;
        CHECK(t.loadFromFile("/tmp/rt_scene.json"), "loadFromFile reports success");
        CHECK(t.objectCount() == 3, "all three objects load back");
        CHECK(t.findByName("Cube") && t.findByName("Cube_1"), "duplicate name is disambiguated, neither object is lost");
        Object* kid = t.findByName("Kid");
        CHECK(kid && kid->parent && kid->parent->name == "Cube", "parent link restored");
        CHECK(kid->position == glm::vec3(1, 2, 3), "transform restored");
        CHECK(glm::length(kid->vertices[0].normal) > 0.99f, "vertex normals survive the round trip");

        // malformed file must not wipe the existing scene
        { std::ofstream bad("/tmp/rt_bad.json"); bad << "{ this is not json"; }
        CHECK(!t.loadFromFile("/tmp/rt_bad.json") && t.objectCount() == 3, "unparseable file is rejected and leaves the scene intact");
        { std::ofstream bad("/tmp/rt_bad2.json"); bad << R"({"objects":[{"name":"X","position":"oops"}]})"; }
        CHECK(!t.loadFromFile("/tmp/rt_bad2.json") && t.objectCount() == 3, "structurally wrong file is rejected and leaves the scene intact");

        // old save files (no normals, no textureOwned) must still load
        { std::ofstream old("/tmp/rt_old.json");
          old << R"({"objects":[{"name":"Old","position":[0,0,0],"rotation":[0,0,0],"scale":[1,1,1],"color":[1,1,1],
              "vertices":[{"position":[0,0,0],"texCoord":[0,0]},{"position":[1,0,0],"texCoord":[1,0]},{"position":[0,1,0],"texCoord":[0,1]}]}]})"; }
        CHECK(t.loadFromFile("/tmp/rt_old.json") && t.findByName("Old") != nullptr, "legacy save file without normals still loads");
        CHECK(t.findByName("Old")->vertices[0].normal == glm::vec3(0.0f), "legacy vertices get a zero (unlit) normal");
    }
    CHECK(liveVAO.empty() && liveVBO.empty() && badDeletes == 0, "after everything: no leaked or double-freed GPU buffers");

    std::printf("\nALL CORE TESTS PASSED\n");
    return 0;
}
