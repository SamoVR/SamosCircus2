#pragma once

#include "ImGuizmo-master/ImSequencer.h"
#include "ImGuizmo-master/ImCurveEdit.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "Object.h"
//

#include <vector>

struct AnimationSequencer {
    struct Item {
        int type;
        int startFrame;
        int endFrame;
        bool expanded = false;
        Object* animatedObject = nullptr;
    };

    struct RampEdit : public ImCurveEdit::Delegate {
        RampEdit();

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

        ImVec2 mPts[3][8];
        size_t mPointCount[3];
        bool mbVisible[3];
        ImVec2 mMin, mMax;
    };

    struct SequenceImpl : public ImSequencer::SequenceInterface {
        AnimationSequencer* parent;

        SequenceImpl(AnimationSequencer* p) : parent(p) {}
        int GetFrameMin() const override { return parent->frameMin; }
        int GetFrameMax() const override { return parent->frameMax; }
        int GetItemCount() const override { return (int)parent->items.size(); }
        int GetItemTypeCount() const override { return 5; }
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

    int currentFrame = 0;
    int frameMin = 0, frameMax = 200;
    std::vector<Item> items;
    RampEdit rampEdit;
    SequenceImpl sequencer;
};
