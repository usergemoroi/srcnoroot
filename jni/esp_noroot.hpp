#ifndef ESP_NOROOT_HPP
#define ESP_NOROOT_HPP

#include "il2cpp_noroot.hpp"
#include "config.hpp"
#include <vector>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

namespace ESP {

// OpenGL-based rendering
struct Color {
    float r, g, b, a;
    Color(uint32_t hex) {
        a = ((hex >> 24) & 0xFF) / 255.0f;
        r = ((hex >> 16) & 0xFF) / 255.0f;
        g = ((hex >> 8) & 0xFF) / 255.0f;
        b = (hex & 0xFF) / 255.0f;
    }
    Color(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}
};

inline void DrawLine(float x1, float y1, float x2, float y2, const Color& col, float width = 2.0f) {
    GLfloat vertices[] = {
        x1, y1,
        x2, y2
    };
    
    GLfloat colors[] = {
        col.r, col.g, col.b, col.a,
        col.r, col.g, col.b, col.a
    };
    
    glLineWidth(width);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, colors);
    glDrawArrays(GL_LINES, 0, 2);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

inline void DrawRect(float x, float y, float width, float height, const Color& col, float lineWidth = 2.0f) {
    GLfloat vertices[] = {
        x, y,
        x + width, y,
        x + width, y + height,
        x, y + height
    };
    
    GLfloat colors[16];
    for (int i = 0; i < 4; i++) {
        colors[i * 4 + 0] = col.r;
        colors[i * 4 + 1] = col.g;
        colors[i * 4 + 2] = col.b;
        colors[i * 4 + 3] = col.a;
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

inline void DrawFilledRect(float x, float y, float width, float height, const Color& col) {
    GLfloat vertices[] = {
        x, y,
        x + width, y,
        x, y + height,
        x + width, y + height
    };
    
    GLfloat colors[16];
    for (int i = 0; i < 4; i++) {
        colors[i * 4 + 0] = col.r;
        colors[i * 4 + 1] = col.g;
        colors[i * 4 + 2] = col.b;
        colors[i * 4 + 3] = col.a;
    }
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, colors);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

inline void DrawBox(float x, float y, float width, float height, const Color& col) {
    DrawRect(x - width/2, y - height, width, height, col, 2.0f);
}

inline void DrawHealthBar(float x, float y, float width, float height, float health, float maxHealth) {
    if (maxHealth <= 0) return;
    
    float healthPercent = health / maxHealth;
    if (healthPercent > 1.0f) healthPercent = 1.0f;
    if (healthPercent < 0.0f) healthPercent = 0.0f;
    
    float barWidth = width;
    float barHeight = 5.0f;
    float barX = x - barWidth/2;
    float barY = y - height - 15;
    
    // Background (dark)
    DrawFilledRect(barX, barY, barWidth, barHeight, Color(0xFF1a1a1a));
    
    // Health bar color: green -> yellow -> red based on health
    Color healthColor(0xFF00ff00);  // Green
    if (healthPercent < 0.5f) {
        healthColor = Color(0xFFffff00);  // Yellow
    }
    if (healthPercent < 0.25f) {
        healthColor = Color(0xFFff0000);  // Red
    }
    
    // Health (colored)
    DrawFilledRect(barX, barY, barWidth * healthPercent, barHeight, healthColor);
    
    // Border
    DrawRect(barX, barY, barWidth, barHeight, Color(0xFF000000), 1.0f);
}

inline void DrawSkeleton(IL2CPP::PlayerController* player, const Color& col) {
    // Simplified skeleton - using transform position as center
    IL2CPP::Vector3 pos = IL2CPP::GetTransformPosition(player->transform);
    IL2CPP::Vector3 screenPos = IL2CPP::WorldToScreen(pos);
    
    if (screenPos.z < 0 || screenPos.x < 0 || screenPos.y < 0) return;
    
    // Draw simple cross for skeleton representation
    float size = 40.0f;
    DrawLine(screenPos.x - size, screenPos.y, screenPos.x + size, screenPos.y, col, 2.0f);
    DrawLine(screenPos.x, screenPos.y - size, screenPos.x, screenPos.y + size, col, 2.0f);
    
    // Draw diagonal lines for arms/legs
    DrawLine(screenPos.x - size, screenPos.y - size/2, screenPos.x + size, screenPos.y - size/2, col, 1.5f);
    DrawLine(screenPos.x - size/2, screenPos.y + size/2, screenPos.x + size/2, screenPos.y + size/2, col, 1.5f);
}

inline Color GetTeamColor(IL2CPP::Team team) {
    switch(team) {
        case IL2CPP::Team_Tr:
            return Color(0xFFff8800);  // Orange for TR
        case IL2CPP::Team_Ct:
            return Color(0xFF0088ff);  // Blue for CT
        case IL2CPP::Team_Spectator:
            return Color(0xFF888888);  // Gray for Spectator
        default:
            return Color(0xFF00ff88);  // Cyan for None/Unknown
    }
}

inline void DrawPlayerESP(IL2CPP::PlayerController* player, const ModConfig& config) {
    if (!player || !player->isAlive) return;
    if (!player->transform) return;
    
    // Get REAL world position using actual Il2Cpp function
    IL2CPP::Vector3 worldPos = IL2CPP::GetTransformPosition(player->transform);
    
    // Get REAL screen position using actual Il2Cpp WorldToScreen
    IL2CPP::Vector3 screenPos = IL2CPP::WorldToScreen(worldPos);
    
    if (screenPos.z < 0 || screenPos.x < 0 || screenPos.y < 0) return;  // Behind camera or off-screen
    
    // Get team-based color
    Color color = GetTeamColor(player->team);
    
    // Calculate box size based on distance
    // Head position estimate (2 units above center)
    IL2CPP::Vector3 headPos = worldPos;
    headPos.y += 2.0f;
    IL2CPP::Vector3 headScreen = IL2CPP::WorldToScreen(headPos);
    
    // Foot position estimate (1 unit below center)
    IL2CPP::Vector3 footPos = worldPos;
    footPos.y -= 1.0f;
    IL2CPP::Vector3 footScreen = IL2CPP::WorldToScreen(footPos);
    
    if (headScreen.z < 0 || footScreen.z < 0) return;
    
    float boxHeight = fabsf(footScreen.y - headScreen.y);
    float boxWidth = boxHeight * 0.4f;  // Width is 40% of height
    
    if (config.esp_box) {
        DrawBox(screenPos.x, footScreen.y, boxWidth, boxHeight, color);
    }
    
    if (config.esp_skeleton) {
        DrawSkeleton(player, color);
    }
    
    if (config.esp_health) {
        // Max health is typically 100 for FPS games
        float maxHealth = 100.0f;
        DrawHealthBar(screenPos.x, footScreen.y, boxWidth, boxHeight, player->health, maxHealth);
    }
}

inline void Update(const ModConfig& config) {
    if (!config.esp_enabled) return;
    
    // Get all players using REAL Il2Cpp object enumeration
    auto players = IL2CPP::GetAllPlayers();
    
    // Save OpenGL state
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLint blendSrc, blendDst;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);
    
    // Setup for 2D drawing
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    // Draw ESP for all players
    for (auto player : players) {
        DrawPlayerESP(player, config);
    }
    
    // Restore OpenGL state
    if (!blendEnabled) glDisable(GL_BLEND);
    if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
    glBlendFunc(blendSrc, blendDst);
}

}
#endif
