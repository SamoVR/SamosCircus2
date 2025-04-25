#include "AnimationSequencer.h"
#include <algorithm>
#include <cstdio>

static const char* typeNames[] = { "Object", "Placeholder"};

// --- RampEdit implementation (same as you already had) ---
AnimationSequencer::RampEdit::RampEdit() {
    mPts[0][0] = ImVec2(0, 0); mPts[0][1] = ImVec2(20, 0.6f); mPts[0][2] = ImVec2(25, 0.2f); mPts[0][3] = ImVec2(70, 0.4f); mPts[0][4] = ImVec2(120, 1.f);
    mPointCount[0] = 5;
    mPts[1][0] = ImVec2(0, 0.2f); mPts[1][1] = ImVec2(33, 0.7f); mPts[1][2] = ImVec2(80, 0.2f); mPts[1][3] = ImVec2(100, 0.8f);
    mPointCount[1] = 4;
    mPts[2][0] = ImVec2(40.f, 0); mPts[2][1] = ImVec2(60.f, 0.1f); mPts[2][2] = ImVec2(90.f, 0.82f); mPts[2][3] = ImVec2(150.f, 0.24f); mPts[2][4] = ImVec2(200.f, 0.34f); mPts[2][5] = ImVec2(250.f, 0.12f);
    mPointCount[2] = 6;
    mbVisible[0] = mbVisible[1] = mbVisible[2] = true;
    mMin = ImVec2(0.f, 0.f); mMax = ImVec2(1.f, 1.f);
}

size_t AnimationSequencer::RampEdit::GetCurveCount() { return 3; }
bool AnimationSequencer::RampEdit::IsVisible(size_t i) { return mbVisible[i]; }
size_t AnimationSequencer::RampEdit::GetPointCount(size_t i) { return mPointCount[i]; }
uint32_t AnimationSequencer::RampEdit::GetCurveColor(size_t i) { uint32_t c[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 }; return c[i]; }
ImVec2* AnimationSequencer::RampEdit::GetPoints(size_t i) { return mPts[i]; }
ImCurveEdit::CurveType AnimationSequencer::RampEdit::GetCurveType(size_t) const { return ImCurveEdit::CurveSmooth; }
int AnimationSequencer::RampEdit::EditPoint(size_t i, int idx, ImVec2 v) { mPts[i][idx] = v; SortValues(i); return idx; }
void AnimationSequencer::RampEdit::AddPoint(size_t i, ImVec2 v) { if (mPointCount[i] < 8) mPts[i][mPointCount[i]++] = v; SortValues(i); }
ImVec2& AnimationSequencer::RampEdit::GetMax() { return mMax; }
ImVec2& AnimationSequencer::RampEdit::GetMin() { return mMin; }
unsigned int AnimationSequencer::RampEdit::GetBackgroundColor() { return 0; }
void AnimationSequencer::RampEdit::SortValues(size_t i) {
    std::sort(mPts[i], mPts[i] + mPointCount[i], [](ImVec2 a, ImVec2 b) { return a.x < b.x; });
}

// --- SequenceImpl implementation ---
const char* AnimationSequencer::SequenceImpl::GetItemTypeName(int typeIndex) const { return typeNames[typeIndex]; }
const char* AnimationSequencer::SequenceImpl::GetItemLabel(int i) const {
    static char buf[64];
    sprintf_s(buf, "[%02d] %s", i, GetItemTypeName(parent->items[i].type));
    return buf;
}
void AnimationSequencer::SequenceImpl::Get(int i, int** s, int** e, int* t, unsigned int* c) {
    if (s) *s = &parent->items[i].startFrame;
    if (e) *e = &parent->items[i].endFrame;
    if (t) *t = parent->items[i].type;
    if (c) *c = 0xFFAA8080;
}
void AnimationSequencer::SequenceImpl::Add(int type) { parent->items.push_back({ type, 0, 10 }); }
void AnimationSequencer::SequenceImpl::Del(int i) { parent->items.erase(parent->items.begin() + i); }
void AnimationSequencer::SequenceImpl::Duplicate(int i) { parent->items.push_back(parent->items[i]); }
size_t AnimationSequencer::SequenceImpl::GetCustomHeight(int i) { return parent->items[i].expanded ? 300 : 0; }
void AnimationSequencer::SequenceImpl::DoubleClick(int i) {
    for (auto& item : parent->items) item.expanded = false;
    parent->items[i].expanded = !parent->items[i].expanded;
}
inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x - b.x, a.y - b.y); }

void AnimationSequencer::SequenceImpl::CustomDraw(int i, ImDrawList* draw, const ImRect& rc, const ImRect&, const ImRect& clip, const ImRect&) {
    Item& item = parent->items[i];
    Object* obj = item.animatedObject;
    if (!obj) return;

    ImGui::SetCursorScreenPos(rc.Min);
    ImGui::PushID(i);

    // Transform mode selector
    const char* modes[] = { "Position", "Rotation", "Scale" };
    int currentMode = static_cast<int>(item.mode);
    ImGui::Combo("Transform", &currentMode, modes, IM_ARRAYSIZE(modes));
    item.mode = static_cast<TransformMode>(currentMode);

    // Curve labels
    ImGui::Text("Curves: X (Red), Y (Green), Z (Blue)");

    // Set curve bounds to match timeline
    item.rampEdit.mMin.x = float(parent->frameMin);
    item.rampEdit.mMax.x = float(parent->frameMax);

    ImGui::SetCursorScreenPos(ImVec2(rc.Min.x, rc.Min.y + 40)); // leave space for labels
    ImCurveEdit::Edit(item.rampEdit, rc.GetSize() - ImVec2(0, 40), 1337 + i, &clip);

    // Apply the animation to object
    int current = parent->currentFrame;
    float time = float(current);
    glm::vec3 result(0.0f);

    for (int curveIndex = 0; curveIndex < 3; ++curveIndex) {
        ImVec2* pts = item.rampEdit.GetPoints(curveIndex);
        size_t count = item.rampEdit.GetPointCount(curveIndex);
        float value = 0.0f;

        for (size_t p = 0; p < count - 1; ++p) {
            if (pts[p].x <= time && pts[p + 1].x >= time) {
                float t = (time - pts[p].x) / (pts[p + 1].x - pts[p].x);
                value = ImLerp(pts[p].y, pts[p + 1].y, t);
                break;
            }
        }

        result[curveIndex] = value;
    }

    switch (item.mode) {
    case TransformMode::Position: obj->position = result; break;
    case TransformMode::Rotation: obj->rotation = result; break;
    case TransformMode::Scale: obj->scale = result; break;
    }

    ImGui::PopID();
}

void AnimationSequencer::SequenceImpl::CustomDrawCompact(int, ImDrawList*, const ImRect&, const ImRect&) {}

AnimationSequencer::AnimationSequencer() : sequencer(this) {

    //items.push_back({ 0, 0, 10 });  // Camera, starts at frame 0, ends at frame 10

}
