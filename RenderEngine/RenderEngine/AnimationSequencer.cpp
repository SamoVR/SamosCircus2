#include "AnimationSequencer.h"
#include <algorithm>
#include <cstdio>

namespace {
    const char* const kTypeNames[] = { "Object" };
    constexpr int kTypeNameCount = static_cast<int>(sizeof(kTypeNames) / sizeof(kTypeNames[0]));

    const char* const kModeNames[] = { "Position", "Rotation", "Scale" };
    static_assert(sizeof(kModeNames) / sizeof(kModeNames[0]) == AnimationSequencer::kTransformModeCount,
        "kModeNames must have one entry per TransformMode");
}

// ---------------------------------------------------------------- RampEdit
AnimationSequencer::RampEdit::RampEdit(float yMin, float yMax) {
    for (int c = 0; c < 3; ++c) {
        mPts[c][0] = ImVec2(0.0f, 0.0f);
        mPts[c][1] = ImVec2(200.0f, 0.0f);
        mPointCount[c] = 2;
        mbVisible[c] = true;
    }
    mMin = ImVec2(0.0f, yMin);
    mMax = ImVec2(200.0f, yMax);
}

size_t AnimationSequencer::RampEdit::GetCurveCount() { return 3; }
bool AnimationSequencer::RampEdit::IsVisible(size_t i) { return mbVisible[i]; }
size_t AnimationSequencer::RampEdit::GetPointCount(size_t i) { return mPointCount[i]; }
uint32_t AnimationSequencer::RampEdit::GetCurveColor(size_t i) {
    static const uint32_t colors[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 }; // X red, Y green, Z blue
    return colors[i];
}
ImVec2* AnimationSequencer::RampEdit::GetPoints(size_t i) { return mPts[i]; }

// Linear, because evaluate() interpolates linearly: the curve you see is now
// the curve that drives the object (it was drawn as a smooth spline).
ImCurveEdit::CurveType AnimationSequencer::RampEdit::GetCurveType(size_t) const { return ImCurveEdit::CurveLinear; }

int AnimationSequencer::RampEdit::EditPoint(size_t i, int idx, ImVec2 v) {
    mPts[i][idx] = v;
    SortValues(i);
    mDirty = true;
    // Sorting can move the dragged point; return its new index so the editor
    // keeps dragging the same point (as the upstream ImGuizmo example does).
    for (size_t p = 0; p < mPointCount[i]; ++p) {
        if (mPts[i][p].x == v.x) return static_cast<int>(p);
    }
    return idx;
}

void AnimationSequencer::RampEdit::AddPoint(size_t i, ImVec2 v) {
    if (mPointCount[i] < 8) {
        mPts[i][mPointCount[i]++] = v;
        SortValues(i);
        mDirty = true;
    }
}

ImVec2& AnimationSequencer::RampEdit::GetMax() { return mMax; }
ImVec2& AnimationSequencer::RampEdit::GetMin() { return mMin; }
unsigned int AnimationSequencer::RampEdit::GetBackgroundColor() { return 0; }

void AnimationSequencer::RampEdit::SortValues(size_t i) {
    std::sort(mPts[i], mPts[i] + mPointCount[i], [](const ImVec2& a, const ImVec2& b) { return a.x < b.x; });
}

void AnimationSequencer::RampEdit::seedFlat(const glm::vec3& value, int frameMin, int frameMax) {
    const float x0 = static_cast<float>(frameMin);
    const float x1 = static_cast<float>(std::max(frameMax, frameMin + 1));
    for (int c = 0; c < 3; ++c) {
        mPts[c][0] = ImVec2(x0, value[c]);
        mPts[c][1] = ImVec2(x1, value[c]);
        mPointCount[c] = 2;
    }
    mRest = value;

    // Keep the seeded values (plus some headroom) inside the visible range.
    const float lo = std::min({ value.x, value.y, value.z });
    const float hi = std::max({ value.x, value.y, value.z });
    const float pad = std::max(1.0f, (hi - lo) * 0.25f);
    mMin.y = std::min(mMin.y, lo - pad);
    mMax.y = std::max(mMax.y, hi + pad);
    mDirty = true;
}

glm::vec3 AnimationSequencer::RampEdit::evaluate(float time) const {
    glm::vec3 result = mRest;
    for (int c = 0; c < 3; ++c) {
        const size_t count = mPointCount[c];
        if (count == 0) continue; // keep rest value
        const ImVec2* pts = mPts[c];

        if (count == 1 || time <= pts[0].x) { result[c] = pts[0].y; continue; }
        if (time >= pts[count - 1].x)       { result[c] = pts[count - 1].y; continue; }

        for (size_t p = 0; p + 1 < count; ++p) {
            if (time >= pts[p].x && time <= pts[p + 1].x) {
                const float dx = pts[p + 1].x - pts[p].x;
                const float t = (dx > 0.0f) ? (time - pts[p].x) / dx : 0.0f; // two points at one frame: step
                result[c] = ImLerp(pts[p].y, pts[p + 1].y, t);
                break;
            }
        }
    }
    return result;
}

// ------------------------------------------------------------ SequenceImpl
int AnimationSequencer::SequenceImpl::GetItemTypeCount() const { return kTypeNameCount; }

const char* AnimationSequencer::SequenceImpl::GetItemTypeName(int typeIndex) const {
    if (typeIndex < 0 || typeIndex >= kTypeNameCount) return "Unknown";
    return kTypeNames[typeIndex];
}

const char* AnimationSequencer::SequenceImpl::GetItemLabel(int i) const {
    if (i < 0 || i >= static_cast<int>(parent->items.size())) return "";
    const Item& item = parent->items[i];
    const char* objectName = item.animatedObject ? item.animatedObject->name.c_str() : GetItemTypeName(item.type);
    char buf[160];
    std::snprintf(buf, sizeof(buf), "[%02d] %s (%s)", i, objectName, kModeNames[static_cast<int>(item.mode)]);
    labelCache = buf;
    return labelCache.c_str();
}

void AnimationSequencer::SequenceImpl::Get(int i, int** s, int** e, int* t, unsigned int* c) {
    Item& item = parent->items[i];
    if (s) *s = &item.startFrame;
    if (e) *e = &item.endFrame;
    if (t) *t = item.type;
    if (c) *c = 0xFFAA8080;
}

void AnimationSequencer::SequenceImpl::Add(int type) {
    Item item;
    item.type = type;
    item.startFrame = parent->frameMin;
    item.endFrame = std::min(parent->frameMin + 30, parent->frameMax);
    parent->items.push_back(item);
}

void AnimationSequencer::SequenceImpl::Del(int i) {
    if (i < 0 || i >= static_cast<int>(parent->items.size())) return;
    parent->items.erase(parent->items.begin() + i);
}

void AnimationSequencer::SequenceImpl::Duplicate(int i) {
    if (i < 0 || i >= static_cast<int>(parent->items.size())) return;
    Item copy = parent->items[i];
    parent->items.push_back(copy);
}

size_t AnimationSequencer::SequenceImpl::GetCustomHeight(int i) { return parent->items[i].expanded ? 300 : 0; }

void AnimationSequencer::SequenceImpl::DoubleClick(int i) {
    const bool wasExpanded = parent->items[i].expanded;
    for (auto& item : parent->items) item.expanded = false;
    parent->items[i].expanded = !wasExpanded;
}

// Draws the editor UI only. It no longer applies the animation (see
// AnimationSequencer::applyToObjects).
void AnimationSequencer::SequenceImpl::CustomDraw(int i, ImDrawList*, const ImRect& rc, const ImRect&, const ImRect& clip, const ImRect&) {
    Item& item = parent->items[i];

    ImGui::SetCursorScreenPos(rc.Min);
    ImGui::PushID(i);

    int currentMode = static_cast<int>(item.mode);
    if (ImGui::Combo("Transform", &currentMode, kModeNames, kTransformModeCount)) {
        item.mode = static_cast<TransformMode>(currentMode);
        item.activeCurves().mDirty = true; // re-apply with the newly selected channel
    }
    ImGui::SameLine();
    ImGui::TextDisabled("X red  Y green  Z blue");

    RampEdit& curves = item.activeCurves();
    curves.mMin.x = static_cast<float>(parent->frameMin);
    curves.mMax.x = static_cast<float>(parent->frameMax);

    ImGui::SetCursorScreenPos(ImVec2(rc.Min.x, rc.Min.y + 40)); // leave room for the header row
    const ImVec2 size(rc.GetWidth(), rc.GetHeight() - 40.0f);
    ImCurveEdit::Edit(curves, size, 1337u + static_cast<unsigned>(i) * 4u + static_cast<unsigned>(item.mode), &clip);

    ImGui::PopID();
}

void AnimationSequencer::SequenceImpl::CustomDrawCompact(int, ImDrawList*, const ImRect&, const ImRect&) {}

// ------------------------------------------------------ AnimationSequencer
AnimationSequencer::AnimationSequencer() : sequencer(this) {}

int AnimationSequencer::addItemForObject(Object* object, int startFrame, int endFrame) {
    Item item;
    item.type = 0;
    item.startFrame = startFrame;
    item.endFrame = endFrame;
    item.animatedObject = object;
    if (object) {
        item.curvesByMode[static_cast<int>(TransformMode::Position)].seedFlat(object->position, frameMin, frameMax);
        item.curvesByMode[static_cast<int>(TransformMode::Rotation)].seedFlat(object->rotation, frameMin, frameMax);
        item.curvesByMode[static_cast<int>(TransformMode::Scale)].seedFlat(object->scale, frameMin, frameMax);
    }
    items.push_back(item);
    return static_cast<int>(items.size()) - 1;
}

void AnimationSequencer::removeItemsForObject(Object* object) {
    items.erase(std::remove_if(items.begin(), items.end(),
        [object](const Item& it) { return it.animatedObject == object; }), items.end());
}

void AnimationSequencer::clearItems() {
    items.clear();
    lastAppliedFrame_ = -1;
}

void AnimationSequencer::applyToObjects() {
    bool curvesChanged = false;
    for (const Item& item : items) {
        for (const RampEdit& r : item.curvesByMode) curvesChanged = curvesChanged || r.mDirty;
    }
    if (currentFrame == lastAppliedFrame_ && !curvesChanged) return;

    for (Item& item : items) {
        Object* obj = item.animatedObject;
        if (!obj) continue;
        if (currentFrame < item.startFrame || currentFrame > item.endFrame) continue; // outside this item's range

        const glm::vec3 value = item.activeCurves().evaluate(static_cast<float>(currentFrame));
        switch (item.mode) {
        case TransformMode::Position: obj->position = value; break;
        case TransformMode::Rotation: obj->rotation = value; break;
        case TransformMode::Scale:    obj->scale = value; break;
        }
    }

    for (Item& item : items) {
        for (RampEdit& r : item.curvesByMode) r.mDirty = false;
    }
    lastAppliedFrame_ = currentFrame;
}
