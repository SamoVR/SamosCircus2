// Style.cpp
#include "Style.h"
#include "raylib.h"
#include "raygui.h"

void ApplyIndustrialStyle()
{
    // -- global background / text --
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(Color { 26, 29, 31, 255 }));
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(Color { 232, 230, 224, 255 }));
    GuiSetStyle(DEFAULT, TEXT_SIZE, 14);

    // -- buttons: normal / focused (hover) / pressed --
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(Color { 37, 42, 45, 255 }));
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, ColorToInt(Color { 58, 65, 69, 255 }));
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(Color { 232, 230, 224, 255 }));

    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(Color { 50, 56, 60, 255 }));
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, ColorToInt(Color { 212, 162, 76, 255 })); // amber highlight on hover
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, ColorToInt(Color { 232, 230, 224, 255 }));

    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt(Color { 212, 162, 76, 255 }));
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, ColorToInt(Color { 212, 162, 76, 255 }));
    GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, ColorToInt(Color { 26, 29, 31, 255 }));

    // -- sliders --
    GuiSetStyle(SLIDER, BASE_COLOR_NORMAL, ColorToInt(Color { 58, 65, 69, 255 }));
    GuiSetStyle(SLIDER, BORDER_COLOR_NORMAL, ColorToInt(Color { 58, 65, 69, 255 }));
    GuiSetStyle(SLIDER, BASE_COLOR_PRESSED, ColorToInt(Color { 212, 162, 76, 255 }));

    // -- checkbox --
    GuiSetStyle(CHECKBOX, BASE_COLOR_NORMAL, ColorToInt(Color { 37, 42, 45, 255 }));
    GuiSetStyle(CHECKBOX, BORDER_COLOR_NORMAL, ColorToInt(Color { 58, 65, 69, 255 }));
    GuiSetStyle(CHECKBOX, BASE_COLOR_PRESSED, ColorToInt(Color { 212, 162, 76, 255 }));

    // -- panel border radius / general --
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);
}