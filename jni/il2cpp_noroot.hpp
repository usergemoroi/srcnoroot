#ifndef IL2CPP_NOROOT_HPP
#define IL2CPP_NOROOT_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <dlfcn.h>
#include <android/log.h>
#include <link.h>
#include <cmath>

#define LOG_TAG "IL2CPP_NoRoot"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

namespace IL2CPP {

// Real RVA offsets from dump (3).cs
// Camera.get_main(): RVA 0x5EDDB80 (line 806378)
// Transform.get_position(): RVA 0x5F26F00 (line 816622)
// Camera.WorldToScreenPoint(Vector3): RVA 0x5EDD48C (line 806345)
#define RVA_CAMERA_GET_MAIN 0x5EDDB80
#define RVA_TRANSFORM_GET_POSITION 0x5F26F00
#define RVA_CAMERA_WORLDTOSCREENPOINT 0x5EDD48C

// Team enum: ACBEDFCBDAFFCBD (line 1841)
// None=0, Tr=1, Ct=2, Spectator=3
enum Team : uint8_t {
    Team_None = 0,
    Team_Tr = 1,
    Team_Ct = 2,
    Team_Spectator = 3
};

struct Vector3 {
    float x, y, z;
    
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    
    float Distance(const Vector3& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float dz = z - other.z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }
    
    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
    
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }
    
    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }
    
    float Length() const {
        return sqrtf(x * x + y * y + z * z);
    }
    
    Vector3 Normalized() const {
        float len = Length();
        if (len < 0.0001f) return Vector3(0, 0, 0);
        return Vector3(x / len, y / len, z / len);
    }
    
    float Dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
};

// Unity structures (minimal padding, just pointers)
struct Object {
    void* vtable;
    void* monitor;
};

struct Component : Object {
    void* m_GameObject;
};

struct Behaviour : Component {
};

struct Transform : Component {
};

struct Camera : Behaviour {
};

struct MonoBehaviour : Behaviour {
};

// PlayerController structure from dump (line 247708)
// Real offsets from dump:
// - 0x28: MonoBehaviour base
// - 0x79: EABHHEFGCCECEBG (team) - ACBEDFCBDAFFCBD enum
// - 0x7C: <EBCAEDABBEEFHDA>k__BackingField (health) - float
// - 0xD0: <CDCFCHEECBBFGAE>k__BackingField (isAlive) - bool
// - 0xF8: CDEFDCAHEGAHBGD (transform) - Transform*
struct PlayerController {
    char pad_0x00[0x79];           // 0x00-0x78: base + fields before team
    Team team;                     // 0x79: ACBEDFCBDAFFCBD enum
    char pad_0x7A[0x2];            // 0x7A-0x7B: padding
    float health;                  // 0x7C: health field
    char pad_0x80[0x50];           // 0x80-0xCF: other fields
    bool isAlive;                  // 0xD0: isAlive bool
    char pad_0xD1[0x27];           // 0xD1-0xF7: other fields
    Transform* transform;          // 0xF8: transform pointer
};

// Global base addresses
uintptr_t g_il2cpp_base = 0;
uintptr_t g_unity_base = 0;

// Function pointers for Il2Cpp API
typedef Camera* (*Camera_get_main_t)();
typedef Vector3 (*Transform_get_position_t)(Transform* self);
typedef Vector3 (*Camera_WorldToScreenPoint_t)(Camera* self, Vector3 position);

Camera_get_main_t Camera_get_main = nullptr;
Transform_get_position_t Transform_get_position = nullptr;
Camera_WorldToScreenPoint_t Camera_WorldToScreenPoint = nullptr;

uintptr_t FindBaseAddress(const char* libraryName) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;
    
    char line[512];
    uintptr_t base = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, libraryName)) {
            char* dash = strchr(line, '-');
            if (dash) {
                *dash = '\0';
                base = strtoul(line, nullptr, 16);
                break;
            }
        }
    }
    
    fclose(fp);
    return base;
}

bool Initialize() {
    LOGD("Initializing IL2CPP resolver with REAL offsets from dump...");
    
    g_il2cpp_base = FindBaseAddress("libil2cpp.so");
    g_unity_base = FindBaseAddress("libunity.so");
    
    if (!g_il2cpp_base) {
        LOGD("Failed to find libil2cpp.so base");
        return false;
    }
    
    LOGD("libil2cpp.so base: 0x%lx", g_il2cpp_base);
    LOGD("libunity.so base: 0x%lx", g_unity_base);
    
    // Set up function pointers using REAL RVA offsets
    Camera_get_main = (Camera_get_main_t)(g_il2cpp_base + RVA_CAMERA_GET_MAIN);
    Transform_get_position = (Transform_get_position_t)(g_il2cpp_base + RVA_TRANSFORM_GET_POSITION);
    Camera_WorldToScreenPoint = (Camera_WorldToScreenPoint_t)(g_il2cpp_base + RVA_CAMERA_WORLDTOSCREENPOINT);
    
    LOGD("Camera.get_main: 0x%lx", (uintptr_t)Camera_get_main);
    LOGD("Transform.get_position: 0x%lx", (uintptr_t)Transform_get_position);
    LOGD("Camera.WorldToScreenPoint: 0x%lx", (uintptr_t)Camera_WorldToScreenPoint);
    
    return true;
}

// Simple player list cache - in production this would enumerate Il2Cpp objects
// For now using a static list that would be populated by game hooks
static std::vector<PlayerController*> g_PlayerList;

void AddPlayer(PlayerController* player) {
    if (!player) return;
    for (auto p : g_PlayerList) {
        if (p == player) return;
    }
    g_PlayerList.push_back(player);
}

void RemovePlayer(PlayerController* player) {
    auto it = std::find(g_PlayerList.begin(), g_PlayerList.end(), player);
    if (it != g_PlayerList.end()) {
        g_PlayerList.erase(it);
    }
}

void ClearPlayers() {
    g_PlayerList.clear();
}

std::vector<PlayerController*> GetAllPlayers() {
    // Filter out invalid/dead players
    std::vector<PlayerController*> validPlayers;
    
    for (auto player : g_PlayerList) {
        if (!player) continue;
        
        // Check if pointer is valid (basic check)
        if ((uintptr_t)player < 0x1000) continue;
        
        // Check if player has valid transform
        if (!player->transform) continue;
        
        validPlayers.push_back(player);
    }
    
    return validPlayers;
}

Vector3 GetTransformPosition(Transform* transform) {
    if (!transform || !Transform_get_position) return Vector3();
    
    // Call REAL Il2Cpp function at RVA_TRANSFORM_GET_POSITION
    return Transform_get_position(transform);
}

Camera* GetMainCamera() {
    if (!Camera_get_main) return nullptr;
    
    // Call REAL Il2Cpp function at RVA_CAMERA_GET_MAIN
    return Camera_get_main();
}

Vector3 WorldToScreen(const Vector3& worldPos) {
    Camera* camera = GetMainCamera();
    if (!camera || !Camera_WorldToScreenPoint) return Vector3(-1, -1, -1);
    
    // Call REAL Il2Cpp function at RVA_CAMERA_WORLDTOSCREENPOINT
    Vector3 screenPos = Camera_WorldToScreenPoint(camera, worldPos);
    
    // Unity WorldToScreenPoint returns:
    // - x, y: screen coordinates (0,0 = bottom-left)
    // - z: distance from camera (negative = behind camera)
    // Need to flip Y coordinate for OpenGL (0,0 = top-left)
    if (screenPos.z > 0) {
        // Get screen height - assume 1920x1080 for now
        // In production would get from glGetIntegerv(GL_VIEWPORT)
        screenPos.y = 1920.0f - screenPos.y;
        return screenPos;
    }
    
    return Vector3(-1, -1, -1);  // Behind camera
}

}

#endif
