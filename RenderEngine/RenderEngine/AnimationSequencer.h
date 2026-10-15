#pragma once

#include "ImGuizmo-master/ImSequencer.h"
#include "ImGuizmo-master/ImCurveEdit.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Object.h"

#include <vector>
#include <array>
#include <string>

struct AnimationSequencer {

    enum class TransformMode { Position, Rotation, Scale };
    static constexpr int kTransformModeCount = 3;

    // Three editable curves (X, Y, Z) for ONE transform channel.
    struct RampEdit : public ImCurveEdit::Delegate {
        // yMin/yMax: initial visible value range of the curve editor.
        explicit RampEdit(float yMin = 0.0f, float yMax = 1.0f);

        size_t GetCurveCount() override;
        size_t GetPointCount(size_t curveIndex) override;
        ImVec2* GetPoints(size_t curveIndex) override;
        ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const override;
        uint32_t GetCurveColor(size_t curveIndex) override;
        bool IsVisible(size_t curveIndex) override;
        void AddPoint(size_t curveIndex, ImVec2 value) override;
        int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value) override;
        ImVec2& GetMax() override;
        ImVec2& GetMin() override;
        unsigned int GetBackgroundColor() override;

        void SortValues(size_t curveIndex);

        // Replaces every curve with a flat line at `value` across
        // [frameMin, frameMax] and widens the visible range to contain it.
        // Used when an item is created, so the default animation is "hold
        // the object's current transform" -- not a demo curve that jumps the
        // object somewhere else (the old defaults drove Scale toward 0).
        void seedFlat(const glm::vec3& value, int frameMin, int frameMax);

        // Linear interpolation between bracketing points; before the first /
        // after the last point the nearest end value is held. (The original
        // fell back to 0.0, which as a Scale factor makes the object vanish,
        // and it also underflowed `count - 1` on a curve with no points.)
        glm::vec3 evaluate(float time) const;

        ImVec2 mPts[3][8];
        size_t mPointCount[3];
        bool mbVisible[3];
        ImVec2 mMin, mMax;
        glm::vec3 mRest{0.0f}; // value used only if a curve has no points
        bool mDirty = true;    // set when a point is added/edited
    };

    struct Item {
        int type = 0;
        int startFrame = 0;
        int endFrame = 30;
        bool expanded = false;
        Object* animatedObject = nullptr;
        // Which single channel this item drives. Each channel keeps its OWN
        // curves, so switching modes no longer reinterprets one shared curve
        // set as units, then degrees, then a scale factor.
        TransformMode mode = TransformMode::Position;

        std::array<RampEdit, kTransformModeCount> curvesByMode{
            RampEdit(-10.0f, 10.0f),   // Position (units)
            RampEdit(-360.0f, 360.0f), // Rotation (degrees)
            RampEdit(0.0f, 4.0f)       // Scale (factor)
        };

        RampEdit& activeCurves() { return curvesByMode[static_cast<int>(mode)]; }
        const RampEdit& activeCurves() const { return curvesByMode[static_cast<int>(mode)]; }
    };

    struct SequenceImpl : public ImSequencer::SequenceInterface {
        AnimationSequencer* parent;
        mutable std::string labelCache;

        explicit SequenceImpl(AnimationSequencer* p) : parent(p) {}
        int GetFrameMin() const override { return parent->frameMin; }
        int GetFrameMax() const override { return parent->frameMax; }
        int GetItemCount() const override { return (int)parent->items.size(); }

        // Derived from the real name table so the two can never drift apart
        // (it returned a hardcoded 5 for a 2-entry table, and the name lookup
        // had no bounds check).
        int GetItemTypeCount() const override;
        const char* GetItemTypeName(int typeIndex) const override;
        const char* GetItemLabel(int index) const override;
        void Get(int index, int** start, int** end, int* type, unsigned int* color) override;
        void Add(int type) override;
        void Del(int index) override;
        void Duplicate(int index) override;
        size_t GetCustomHeight(int index) override;
        void DoubleClick(int index) override;
        void CustomDraw(int index, ImDrawList*, const ImRect&, const ImRect&, const ImRect&, const ImRect&) override;
        void CustomDrawCompact(int index, ImDrawList*, const ImRect&, const ImRect&) override;
    };

    AnimationSequencer();

    // Adds an item animating `object`, with every channel's curves seeded flat
    // at the object's current position / rotation / scale. Returns its index.
    int addItemForObject(Object* object, int startFrame = 0, int endFrame = 30);

    // Drops every item animating `object`. Call before destroying an Object.
    void removeItemsForObject(Object* object);

    // Drops all items (e.g. when the scene is replaced).
    void clearItems();

    // Writes the animated transform into each item's object for the current
    // frame. Must be called once per frame, independent of the UI.
    //
    // In the original this logic lived inside CustomDraw, which the widget
    // only calls for EXPANDED rows -- so animations silently did nothing
    // unless you had double-clicked the row open -- and it also ignored each
    // item's start/end frame. It only writes when the frame changed or a
    // curve was edited, so it doesn't fight manual gizmo edits while paused.
    void applyToObjects();

    int currentFrame = 0;
    int frameMin = 0, frameMax = 200;
    std::vector<Item> items;
    SequenceImpl sequencer;

private:
    int lastAppliedFrame_ = -1;
};
