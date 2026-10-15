#include "TransformUtil.h"
#include <cstdio>
#include <random>

static float maxDiff(const glm::mat4& a, const glm::mat4& b) {
    float d = 0; for (int c = 0; c < 4; ++c) for (int r = 0; r < 4; ++r) d = std::max(d, std::abs(a[c][r] - b[c][r])); return d;
}
// The ORIGINAL gizmo code's extraction, copied from Interface.cpp renderGizmo()
static void originalExtract(const glm::mat4& model, glm::vec3& rot) {
    rot.x = glm::degrees(atan2(model[1][2], model[1][1]));
    rot.y = glm::degrees(atan2(model[2][0], model[0][0]));
    rot.z = glm::degrees(atan2(model[1][0], model[0][0]));
}

int main() {
    std::mt19937 rng(7);
    std::uniform_real_distribution<float> ang(-179, 179), pos(-20, 20), scl(0.1f, 5.0f);
    float worstNew = 0, worstOld = 0; int oldBad = 0, N = 2000;
    for (int i = 0; i < N; ++i) {
        glm::vec3 p(pos(rng), pos(rng), pos(rng)), r(ang(rng), ang(rng) * 0.49f, ang(rng)), s(scl(rng), scl(rng), scl(rng));
        glm::mat4 M = TransformUtil::compose(p, r, s);
        glm::vec3 p2, r2, s2; TransformUtil::decompose(M, p2, r2, s2);
        worstNew = std::max(worstNew, maxDiff(M, TransformUtil::compose(p2, r2, s2)));
        glm::vec3 ro; originalExtract(M, ro);
        float e = maxDiff(M, TransformUtil::compose(p, ro, s));
        worstOld = std::max(worstOld, e); if (e > 1e-2f) ++oldBad;
    }
    std::printf("new decompose: worst matrix error over %d random transforms = %.2e\n", N, worstNew);
    std::printf("original gizmo extraction: %d/%d transforms rebuilt WRONG (worst error %.2f)\n", oldBad, N, worstOld);

    // gimbal lock + mirrored
    int fails = 0;
    for (float y : { 90.0f, -90.0f }) {
        glm::mat4 M = TransformUtil::compose({1,2,3}, {30, y, 20}, {1,1,1});
        glm::vec3 p, r, s; TransformUtil::decompose(M, p, r, s);
        float e = maxDiff(M, TransformUtil::compose(p, r, s));
        std::printf("gimbal lock y=%+.0f: reconstruction error %.2e\n", y, e); if (e > 1e-4f) ++fails;
    }
    glm::mat4 mir = TransformUtil::compose({0,0,0}, {10,20,30}, {-2, 1, 1});
    glm::vec3 p, r, s; TransformUtil::decompose(mir, p, r, s);
    float e = maxDiff(mir, TransformUtil::compose(p, r, s));
    std::printf("mirrored (scale.x = -2): reconstruction error %.2e\n", e); if (e > 1e-4f) ++fails;

    bool ok = worstNew < 1e-3f && fails == 0 && oldBad > N / 4;
    std::printf(ok ? "\nTRANSFORM TESTS PASSED\n" : "\nTRANSFORM TESTS FAILED\n");
    return ok ? 0 : 1;
}
