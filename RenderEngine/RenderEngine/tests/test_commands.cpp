#include "Commands.h"
#include "CommandHistory.h"
#include "Settings.h"
#include "Geometry.h"
#include <set>
#include <cstdio>

static std::set<GLuint> liveVAO; static GLuint nv = 1; static int badDeletes = 0;
static void APIENTRY gv(GLsizei n, GLuint* a) { for (GLsizei i = 0; i < n; ++i) { a[i] = nv++; liveVAO.insert(a[i]); } }
static void APIENTRY gb(GLsizei n, GLuint* a) { for (GLsizei i = 0; i < n; ++i) a[i] = nv++; }
static void APIENTRY dv(GLsizei n, const GLuint* a) { for (GLsizei i = 0; i < n; ++i) if (a[i] && !liveVAO.erase(a[i])) ++badDeletes; }
static void APIENTRY db(GLsizei, const GLuint*) {}
static void APIENTRY bv(GLuint) {}
static void APIENTRY bb(GLenum, GLuint) {}
static void APIENTRY bd(GLenum, GLsizeiptr, const void*, GLenum) {}
static void APIENTRY ev(GLuint) {}
static void APIENTRY ap(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) {}
PFNGLGENVERTEXARRAYSPROC __glewGenVertexArrays = gv;   PFNGLDELETEVERTEXARRAYSPROC __glewDeleteVertexArrays = dv;
PFNGLBINDVERTEXARRAYPROC __glewBindVertexArray = bv;   PFNGLGENBUFFERSPROC __glewGenBuffers = gb;
PFNGLDELETEBUFFERSPROC __glewDeleteBuffers = db;       PFNGLBINDBUFFERPROC __glewBindBuffer = bb;
PFNGLBUFFERDATAPROC __glewBufferData = bd;             PFNGLENABLEVERTEXATTRIBARRAYPROC __glewEnableVertexAttribArray = ev;
PFNGLVERTEXATTRIBPOINTERPROC __glewVertexAttribPointer = ap;

#define CHECK(c, m) do { if (!(c)) { std::printf("FAIL: %s (line %d)\n", m, __LINE__); return 1; } else std::printf("[PASS] %s\n", m); } while (0)

int main() {
    auto cube = createCubeVertices();
    {
        Scene scene; AnimationSequencer anim; CommandHistory h;

        // ---- add / undo / redo
        auto o = std::make_unique<Object>("A", cube); Object* A = o.get();
        h.execute(std::make_unique<AddObjectCommand>(scene, std::move(o), "Add A"));
        CHECK(scene.contains(A), "add: object is in the scene");
        h.undo(); CHECK(!scene.contains(A) && scene.objectCount() == 0, "add undone: removed, but alive inside the command");
        h.redo(); CHECK(scene.contains(A), "add redone: back in the scene (same pointer)");

        // ---- modify / undo / redo
        ObjectState before = ObjectState::capture(*A);
        A->position = {5, 6, 7}; A->setName("Renamed");
        h.execute(std::make_unique<ModifyObjectCommand>(A, before, ObjectState::capture(*A), "Edit"));
        h.undo(); CHECK(A->position == glm::vec3(0) && A->name == "A", "modify undone: transform and name restored");
        h.redo(); CHECK(A->position == glm::vec3(5, 6, 7) && A->name == "Renamed", "modify redone");

        // ---- delete a PARENT that has children + an animation, then undo (RE-01 scenario)
        scene.addObject(std::make_unique<Object>("P", cube));
        scene.addObject(std::make_unique<Object>("C1", cube));
        scene.addObject(std::make_unique<Object>("C2", cube));
        Object *P = scene.findByName("P"), *C1 = scene.findByName("C1"), *C2 = scene.findByName("C2");
        C1->setParent(P); C2->setParent(P);
        anim.addItemForObject(P);
        size_t pIndex = scene.indexOf(P);
        h.execute(std::make_unique<RemoveObjectCommand>(scene, anim, P));
        CHECK(!scene.contains(P) && scene.contains(C1) && scene.contains(C2), "delete parent: children survive");
        CHECK(C1->parent == nullptr && C2->parent == nullptr, "delete parent: no child keeps a dangling parent pointer");
        CHECK(anim.items.empty(), "delete parent: its animation item is removed (no dangling Object*)");
        h.undo();
        CHECK(scene.contains(P) && scene.indexOf(P) == pIndex, "undo delete: object restored at its original list position");
        CHECK(C1->parent == P && C2->parent == P && P->children.size() == 2, "undo delete: hierarchy restored");
        CHECK(anim.items.size() == 1 && anim.items[0].animatedObject == P, "undo delete: animation item restored");
        h.redo(); CHECK(!scene.contains(P) && anim.items.empty(), "redo delete works again");
        h.undo();

        // ---- reparent keeping world position
        scene.addObject(std::make_unique<Object>("Anchor", cube));
        Object* anchor = scene.findByName("Anchor"); anchor->position = {10, 0, 0};
        ObjectState b = ObjectState::capture(*C1);
        glm::vec3 worldBefore(C1->getGlobalTransform()[3]);
        C1->setParent(anchor);                       // as the UI does: reparent, then fix up local transform
        ObjectState a = b; a.position = worldBefore - anchor->position; // world stays put
        C1->position = a.position;
        C1->setParent(P); C1->position = b.position;
        h.execute(std::make_unique<ReparentCommand>(C1, P, anchor, b, a));
        glm::vec3 worldAfter(C1->getGlobalTransform()[3]);
        CHECK(C1->parent == anchor && glm::length(worldAfter - worldBefore) < 1e-4f, "reparent keeps the object's world position");
        h.undo(); CHECK(C1->parent == P && C1->position == b.position, "reparent undone: old parent and local transform back");

        // ---- history cap must free objects owned by dropped commands (no leaks / double frees)
        for (int i = 0; i < CommandHistory::kMaxHistory + 20; ++i) {
            auto t = std::make_unique<Object>("T", cube); Object* tp = t.get();
            h.execute(std::make_unique<AddObjectCommand>(scene, std::move(t), "Add T"));
            h.execute(std::make_unique<RemoveObjectCommand>(scene, anim, tp));   // object now owned by the command
        }
        CHECK(h.canUndo(), "history survives hundreds of owning commands");
        h.clear();
    }
    CHECK(liveVAO.empty() && badDeletes == 0, "scene + history destroyed: every GPU buffer freed exactly once");

    // ---- settings round trip + tolerance
    AppSettings s; s.showGrid = false; s.uiScale = 1.5f; s.lastScenePath = "a/b.json";
    s.keybinds.push_back({"Undo", 90, true, false, false});
    AppSettings t; t.fromJson(s.toJson());
    CHECK(!t.showGrid && t.uiScale == 1.5f && t.lastScenePath == "a/b.json" && t.keybinds.size() == 1 && t.keybinds[0].key == 90, "settings survive a JSON round trip");
    AppSettings u; u.fromJson(nlohmann::json::parse(R"({"showGrid":"banana","uiScale":99,"keybinds":[{"name":1}]})"));
    CHECK(u.showGrid && u.uiScale == 3.0f && u.keybinds.empty(), "wrong-typed / out-of-range settings fall back safely instead of throwing");

    std::printf("\nALL COMMAND + SETTINGS TESTS PASSED\n");
    return 0;
}
