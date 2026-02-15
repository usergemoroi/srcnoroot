#ifndef AIMBOT_NOROOT_HPP
#define AIMBOT_NOROOT_HPP

#include "il2cpp_noroot.hpp"
#include "config.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

namespace Aimbot {

float GetFOVDistance(const IL2CPP::Vector3& targetPos, const IL2CPP::Vector3& cameraPos, const IL2CPP::Vector3& cameraForward) {
    IL2CPP::Vector3 direction = (targetPos - cameraPos).Normalized();
    
    float dot = direction.Dot(cameraForward);
    
    if (dot < -1.0f) dot = -1.0f;
    if (dot > 1.0f) dot = 1.0f;
    
    float angle = acosf(dot) * (180.0f / 3.14159265359f);
    return angle;
}

bool IsEnemy(IL2CPP::Team myTeam, IL2CPP::Team theirTeam) {
    // Spectators are not enemies to anyone
    if (myTeam == IL2CPP::Team_Spectator || theirTeam == IL2CPP::Team_Spectator) {
        return false;
    }
    // None team is not enemy to anyone
    if (myTeam == IL2CPP::Team_None || theirTeam == IL2CPP::Team_None) {
        return false;
    }
    // Different teams are enemies
    return myTeam != theirTeam;
}

bool IsVisible(const IL2CPP::Vector3& worldPos) {
    IL2CPP::Vector3 screenPos = IL2CPP::WorldToScreen(worldPos);
    
    // Check if behind camera
    if (screenPos.z < 0) return false;
    
    // Check if on screen (assuming 1920x1080)
    if (screenPos.x < 0 || screenPos.x > 1080) return false;
    if (screenPos.y < 0 || screenPos.y > 1920) return false;
    
    return true;
}

struct TargetInfo {
    IL2CPP::PlayerController* player;
    float fov;
    float distance;
    IL2CPP::Vector3 screenPos;
};

IL2CPP::PlayerController* GetLocalPlayer() {
    // Find local player - typically the first player in list that's controlled by this client
    // In production, would use proper Il2Cpp PhotonView ownership checks
    auto players = IL2CPP::GetAllPlayers();
    
    for (auto player : players) {
        if (!player || !player->isAlive) continue;
        // For now, assume first alive player is local
        // In real game, would check PhotonView.isMine or similar
        return player;
    }
    
    return nullptr;
}

TargetInfo* GetBestTarget(const ModConfig& config) {
    IL2CPP::PlayerController* localPlayer = GetLocalPlayer();
    if (!localPlayer) return nullptr;
    
    IL2CPP::Team myTeam = localPlayer->team;
    
    auto players = IL2CPP::GetAllPlayers();
    
    IL2CPP::Camera* camera = IL2CPP::GetMainCamera();
    if (!camera) return nullptr;
    
    // Get camera transform and position using REAL Il2Cpp functions
    if (!camera) return nullptr;
    
    // Camera position - for WorldToScreenPoint, we use camera's internal position
    // which is handled by the WorldToScreenPoint function itself
    
    static TargetInfo bestTarget;
    bool foundTarget = false;
    float bestFOV = config.aimbot_fov;
    
    for (auto player : players) {
        if (!player || player == localPlayer || !player->isAlive) continue;
        
        // Team check using REAL team field at offset 0x79
        if (config.aimbot_team_check) {
            if (!IsEnemy(myTeam, player->team)) {
                continue;
            }
        }
        
        // Get REAL world position using actual Il2Cpp function
        IL2CPP::Vector3 targetPos = IL2CPP::GetTransformPosition(player->transform);
        
        // Check visibility using REAL WorldToScreen
        if (config.aimbot_visible_only) {
            if (!IsVisible(targetPos)) {
                continue;
            }
        }
        
        // Get screen position
        IL2CPP::Vector3 screenPos = IL2CPP::WorldToScreen(targetPos);
        if (screenPos.z < 0) continue;  // Behind camera
        
        // Calculate FOV distance from screen center
        float screenCenterX = 1080.0f / 2.0f;  // 540
        float screenCenterY = 1920.0f / 2.0f;  // 960
        
        float dx = screenPos.x - screenCenterX;
        float dy = screenPos.y - screenCenterY;
        float screenDistance = sqrtf(dx * dx + dy * dy);
        
        // Convert screen distance to FOV angle (approximate)
        float fov = (screenDistance / 540.0f) * 90.0f;  // Normalize to FOV
        
        if (fov < bestFOV) {
            bestFOV = fov;
            bestTarget.player = player;
            bestTarget.fov = fov;
            bestTarget.screenPos = screenPos;
            bestTarget.distance = screenPos.z;  // Z is distance from camera
            foundTarget = true;
        }
    }
    
    return foundTarget ? &bestTarget : nullptr;
}

void AimAtTarget(const TargetInfo* target, const ModConfig& config) {
    if (!target || !target->player) return;
    
    // Calculate aim adjustment with smoothing
    float screenCenterX = 1080.0f / 2.0f;  // 540
    float screenCenterY = 1920.0f / 2.0f;  // 960
    
    float dx = target->screenPos.x - screenCenterX;
    float dy = target->screenPos.y - screenCenterY;
    
    // Apply smoothing
    float smoothFactor = config.aimbot_smooth;
    if (smoothFactor < 0.1f) smoothFactor = 0.1f;
    
    float adjustX = dx / smoothFactor;
    float adjustY = dy / smoothFactor;
    
    // In production, would inject mouse input or modify camera rotation here
    // For now, this is a placeholder for the aim logic
    // Would use Android input injection or Unity Input.mousePosition modification
}

void Update(const ModConfig& config) {
    if (!config.aimbot_enabled) return;
    
    TargetInfo* target = GetBestTarget(config);
    if (target) {
        AimAtTarget(target, config);
    }
}

}

#endif
