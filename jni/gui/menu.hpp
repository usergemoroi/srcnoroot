#ifndef MENU_HPP
#define MENU_HPP

#include "../config.hpp"
#include <GLES2/gl2.h>
#include <cmath>

namespace Menu {

// Simple GUI state
struct MenuState {
    bool visible = true;
    bool dragging = false;
    float menuX = 50.0f;
    float menuY = 100.0f;
    float dragOffsetX = 0.0f;
    float dragOffsetY = 0.0f;
    float touchX = 0.0f;
    float touchY = 0.0f;
    bool touching = false;
};

static MenuState g_MenuState;

// Colors (RGBA)
struct Color {
    float r, g, b, a;
    
    Color(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}
    
    Color(uint32_t hex) {
        a = ((hex >> 24) & 0xFF) / 255.0f;
        r = ((hex >> 16) & 0xFF) / 255.0f;
        g = ((hex >> 8) & 0xFF) / 255.0f;
        b = (hex & 0xFF) / 255.0f;
    }
};

// Theme colors
const Color COLOR_BG = Color(0xFF1a1a1a);
const Color COLOR_HEADER = Color(0xFF2d2d2d);
const Color COLOR_BUTTON = Color(0xFF3d3d3d);
const Color COLOR_BUTTON_ACTIVE = Color(0xFF4d4d4d);
const Color COLOR_ESP = Color(0xFF00ff88);
const Color COLOR_AIMBOT = Color(0xFFff4757);
const Color COLOR_TEXT = Color(0xFFffffff);
const Color COLOR_BORDER = Color(0xFF444444);

// Simple drawing functions
inline void DrawFilledRect(float x, float y, float w, float h, const Color& c) {
    GLfloat vertices[] = {
        x, y,
        x + w, y,
        x, y + h,
        x + w, y + h
    };
    
    GLfloat colors[16];
    for (int i = 0; i < 4; i++) {
        colors[i * 4 + 0] = c.r;
        colors[i * 4 + 1] = c.g;
        colors[i * 4 + 2] = c.b;
        colors[i * 4 + 3] = c.a;
    }
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, colors);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

inline void DrawRect(float x, float y, float w, float h, const Color& c, float lineWidth = 2.0f) {
    GLfloat vertices[] = {
        x, y,
        x + w, y,
        x + w, y + h,
        x, y + h
    };
    
    GLfloat colors[16];
    for (int i = 0; i < 4; i++) {
        colors[i * 4 + 0] = c.r;
        colors[i * 4 + 1] = c.g;
        colors[i * 4 + 2] = c.b;
        colors[i * 4 + 3] = c.a;
    }
    
    glLineWidth(lineWidth);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, colors);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

inline void DrawText(float x, float y, const char* text, const Color& c) {
    // Text rendering would require font atlas
    // For now, this is a placeholder
    // In production, would use stb_truetype or similar
}

inline bool IsPointInRect(float px, float py, float rx, float ry, float rw, float rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

inline void DrawToggle(float x, float y, float w, float h, const char* label, bool& value, const Color& activeColor) {
    // Draw toggle background
    Color bgColor = value ? activeColor : COLOR_BUTTON;
    DrawFilledRect(x, y, w, h, bgColor);
    DrawRect(x, y, w, h, COLOR_BORDER, 2.0f);
    
    // Draw checkmark if active
    if (value) {
        float checkX = x + 10;
        float checkY = y + h/2;
        DrawFilledRect(checkX, checkY - 3, 15, 6, COLOR_TEXT);
    }
    
    // Handle touch
    if (g_MenuState.touching && !g_MenuState.dragging) {
        if (IsPointInRect(g_MenuState.touchX, g_MenuState.touchY, x, y, w, h)) {
            value = !value;
            g_MenuState.touching = false;  // Consume touch
        }
    }
}

inline void DrawSlider(float x, float y, float w, float h, const char* label, float& value, float min, float max, const Color& activeColor) {
    // Draw slider background
    DrawFilledRect(x, y, w, h, COLOR_BUTTON);
    DrawRect(x, y, w, h, COLOR_BORDER, 2.0f);
    
    // Draw slider fill
    float fillWidth = ((value - min) / (max - min)) * (w - 4);
    DrawFilledRect(x + 2, y + 2, fillWidth, h - 4, activeColor);
    
    // Draw slider thumb
    float thumbX = x + fillWidth;
    DrawFilledRect(thumbX, y, 4, h, COLOR_TEXT);
    
    // Handle touch
    if (g_MenuState.touching && !g_MenuState.dragging) {
        if (IsPointInRect(g_MenuState.touchX, g_MenuState.touchY, x, y, w, h)) {
            float ratio = (g_MenuState.touchX - x) / w;
            if (ratio < 0) ratio = 0;
            if (ratio > 1) ratio = 1;
            value = min + ratio * (max - min);
        }
    }
}

void Init() {
    g_MenuState.visible = true;
    g_MenuState.menuX = 50.0f;
    g_MenuState.menuY = 100.0f;
}

void HandleTouch(float x, float y, bool down) {
    g_MenuState.touchX = x;
    g_MenuState.touchY = y;
    g_MenuState.touching = down;
    
    if (down && !g_MenuState.dragging) {
        // Check if touching menu header for dragging
        float headerX = g_MenuState.menuX;
        float headerY = g_MenuState.menuY;
        float headerW = 350.0f;
        float headerH = 50.0f;
        
        if (IsPointInRect(x, y, headerX, headerY, headerW, headerH)) {
            g_MenuState.dragging = true;
            g_MenuState.dragOffsetX = x - headerX;
            g_MenuState.dragOffsetY = y - headerY;
        }
    } else if (!down) {
        g_MenuState.dragging = false;
    }
    
    if (g_MenuState.dragging) {
        g_MenuState.menuX = x - g_MenuState.dragOffsetX;
        g_MenuState.menuY = y - g_MenuState.dragOffsetY;
    }
}

void ToggleVisibility() {
    g_MenuState.visible = !g_MenuState.visible;
}

void Render(ModConfig& config) {
    if (!g_MenuState.visible) {
        // Draw floating menu button
        float btnX = 20.0f;
        float btnY = 100.0f;
        float btnW = 60.0f;
        float btnH = 60.0f;
        
        DrawFilledRect(btnX, btnY, btnW, btnH, COLOR_HEADER);
        DrawRect(btnX, btnY, btnW, btnH, COLOR_ESP, 3.0f);
        
        // Draw "M" text (simplified)
        DrawFilledRect(btnX + 15, btnY + 15, 30, 30, COLOR_ESP);
        
        // Handle touch on button
        if (g_MenuState.touching && !g_MenuState.dragging) {
            if (IsPointInRect(g_MenuState.touchX, g_MenuState.touchY, btnX, btnY, btnW, btnH)) {
                g_MenuState.visible = true;
                g_MenuState.touching = false;
            }
        }
        
        return;
    }
    
    // Save OpenGL state
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    float x = g_MenuState.menuX;
    float y = g_MenuState.menuY;
    float w = 350.0f;
    float itemHeight = 50.0f;
    float padding = 10.0f;
    
    // Draw menu background
    float menuHeight = 600.0f;
    DrawFilledRect(x, y, w, menuHeight, COLOR_BG);
    DrawRect(x, y, w, menuHeight, COLOR_ESP, 3.0f);
    
    // Draw header
    DrawFilledRect(x, y, w, 50, COLOR_HEADER);
    DrawRect(x, y, w, 50, COLOR_ESP, 2.0f);
    
    float currentY = y + 60;
    
    // ESP Section
    DrawFilledRect(x + padding, currentY, w - padding * 2, 30, COLOR_ESP);
    currentY += 40;
    
    DrawToggle(x + padding, currentY, w - padding * 2, itemHeight, "ESP Enabled", config.esp_enabled, COLOR_ESP);
    currentY += itemHeight + padding;
    
    DrawToggle(x + padding, currentY, w - padding * 2, itemHeight, "ESP Box", config.esp_box, COLOR_ESP);
    currentY += itemHeight + padding;
    
    DrawToggle(x + padding, currentY, w - padding * 2, itemHeight, "ESP Skeleton", config.esp_skeleton, COLOR_ESP);
    currentY += itemHeight + padding;
    
    DrawToggle(x + padding, currentY, w - padding * 2, itemHeight, "ESP Health", config.esp_health, COLOR_ESP);
    currentY += itemHeight + padding * 2;
    
    // Aimbot Section
    DrawFilledRect(x + padding, currentY, w - padding * 2, 30, COLOR_AIMBOT);
    currentY += 40;
    
    DrawToggle(x + padding, currentY, w - padding * 2, itemHeight, "Aimbot Enabled", config.aimbot_enabled, COLOR_AIMBOT);
    currentY += itemHeight + padding;
    
    DrawToggle(x + padding, currentY, w - padding * 2, itemHeight, "Visible Only", config.aimbot_visible_only, COLOR_AIMBOT);
    currentY += itemHeight + padding;
    
    DrawToggle(x + padding, currentY, w - padding * 2, itemHeight, "Team Check", config.aimbot_team_check, COLOR_AIMBOT);
    currentY += itemHeight + padding;
    
    DrawSlider(x + padding, currentY, w - padding * 2, itemHeight, "FOV", config.aimbot_fov, 10.0f, 180.0f, COLOR_AIMBOT);
    currentY += itemHeight + padding;
    
    DrawSlider(x + padding, currentY, w - padding * 2, itemHeight, "Smooth", config.aimbot_smooth, 1.0f, 20.0f, COLOR_AIMBOT);
    
    // Restore OpenGL state
    if (!blendEnabled) glDisable(GL_BLEND);
    if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
    
    // Clear touch after processing
    if (!g_MenuState.dragging) {
        g_MenuState.touching = false;
    }
}

}

#endif
