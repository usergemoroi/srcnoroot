#include <jni.h>
#include <string>
#include <vector>
#include <thread>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <android/input.h>
#include <android/native_window_jni.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include "config.hpp"
#include "il2cpp_noroot.hpp"
#include "hooks_noroot.hpp"
#include "protection_advanced.hpp"
#include "esp_noroot.hpp"
#include "aimbot_noroot.hpp"
#include "string_encrypt.hpp"
#include "gui/menu.hpp"

// No logging in production - completely silent
#define NO_LOG (...)

ModConfig g_Config;
JavaVM* g_JavaVM = nullptr;
jobject g_Context = nullptr;
bool g_Initialized = false;
bool g_RenderReady = false;

void MonitorThread() {
    while (true) {
        Protection::CheckProtection();
        usleep(500000);  // 500ms
    }
}

// Hook eglSwapBuffers to render GUI and ESP
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (g_Initialized && g_RenderReady) {
        // Save OpenGL state
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        
        GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
        GLboolean blend = glIsEnabled(GL_BLEND);
        GLboolean cullFace = glIsEnabled(GL_CULL_FACE);
        GLboolean scissorTest = glIsEnabled(GL_SCISSOR_TEST);
        
        // Setup for 2D rendering (GLES2 doesn't need matrix setup)
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        
        // Render ESP
        if (g_Config.esp_enabled) {
            ESP::Update(g_Config);
        }
        
        // Render GUI
        Menu::Render(g_Config);
        
        // Restore OpenGL state
        if (depthTest) glEnable(GL_DEPTH_TEST);
        if (!blend) glDisable(GL_BLEND);
        if (cullFace) glEnable(GL_CULL_FACE);
        if (scissorTest) glEnable(GL_SCISSOR_TEST);
    }
    
    return orig_eglSwapBuffers(dpy, surface);
}

void InstallRenderHook() {
    // Hook eglSwapBuffers for rendering
    void* libegl = dlopen("libEGL.so", RTLD_NOW);
    if (libegl) {
        void* eglSwapBuffers = dlsym(libegl, "eglSwapBuffers");
        if (eglSwapBuffers) {
            // Simple function pointer replacement (in production would use MSHookFunction)
            orig_eglSwapBuffers = (eglSwapBuffers_t)eglSwapBuffers;
            // Note: actual hooking would require substrate or similar
            // For now this is a reference implementation
        }
    }
}

void CoreThread() {
    NO_LOG("Init");
    
    usleep(2000000);  // 2s delay
    
    Protection::InitProtection();
    
    // Initialize Il2Cpp with REAL offsets
    if (!IL2CPP::Initialize()) {
        NO_LOG("IL2CPP init failed");
        return;
    }
    
    // Install hooks
    Hooks::InstallHooks();
    
    // Initialize GUI
    Menu::Init();
    
    // Install render hook
    InstallRenderHook();
    
    g_Initialized = true;
    g_RenderReady = true;
    
    std::thread monitor(MonitorThread);
    monitor.detach();
    
    // Main update loop
    while (true) {
        if (g_Initialized) {
            // Aimbot runs in update loop
            if (g_Config.aimbot_enabled) {
                Aimbot::Update(g_Config);
            }
        }
        usleep(16666);  // ~60 FPS
    }
}

extern "C" {

// Obfuscated JNI function names - random letters
// Original: Java_com_modmenu_loader_ModMenuLoader_nativeInit
JNIEXPORT void JNICALL Java_a_b_c_d_e(JNIEnv* env, jclass clazz, jobject context) {
    g_Context = env->NewGlobalRef(context);
    Protection::EncryptStrings();
}

// Original: Java_com_modmenu_loader_ModMenuLoader_toggleMenu
JNIEXPORT void JNICALL Java_a_b_c_d_f(JNIEnv* env, jclass clazz) {
    Menu::ToggleVisibility();
}

// Original: Java_com_modmenu_loader_ModMenuLoader_isMenuVisible
JNIEXPORT jboolean JNICALL Java_a_b_c_d_g(JNIEnv* env, jclass clazz) {
    return g_Config.menu_visible;
}

// Touch input handling for GUI
JNIEXPORT void JNICALL Java_a_b_c_d_h(JNIEnv* env, jclass clazz, jfloat x, jfloat y, jboolean down) {
    Menu::HandleTouch(x, y, down);
}

// Original: nativeSetESPEnabled
JNIEXPORT void JNICALL Java_a_b_h_i_j(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_enabled = enabled;
}

JNIEXPORT void JNICALL Java_a_b_h_i_k(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_box = enabled;
}

JNIEXPORT void JNICALL Java_a_b_h_i_l(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_skeleton = enabled;
}

JNIEXPORT void JNICALL Java_a_b_h_i_m(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_health = enabled;
}

JNIEXPORT void JNICALL Java_a_b_h_i_n(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.aimbot_enabled = enabled;
}

JNIEXPORT void JNICALL Java_a_b_h_i_o(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.aimbot_visible_only = enabled;
}

JNIEXPORT void JNICALL Java_a_b_h_i_p(JNIEnv* env, jobject obj, jfloat fov) {
    g_Config.aimbot_fov = fov;
}

JNIEXPORT void JNICALL Java_a_b_h_i_q(JNIEnv* env, jobject obj, jfloat smooth) {
    g_Config.aimbot_smooth = smooth;
}

// Lifecycle callbacks - obfuscated
JNIEXPORT void JNICALL Java_a_b_r_s_t(JNIEnv* env, jobject obj, jobject activity) {}
JNIEXPORT void JNICALL Java_a_b_r_s_u(JNIEnv* env, jobject obj, jobject activity) {}
JNIEXPORT void JNICALL Java_a_b_r_s_v(JNIEnv* env, jobject obj, jobject activity) {
    Hooks::OnActivityResumed();
    g_RenderReady = true;
}
JNIEXPORT void JNICALL Java_a_b_r_s_w(JNIEnv* env, jobject obj, jobject activity) {
    g_RenderReady = false;
}
JNIEXPORT void JNICALL Java_a_b_r_s_x(JNIEnv* env, jobject obj, jobject activity) {}
JNIEXPORT void JNICALL Java_a_b_r_s_y(JNIEnv* env, jobject obj, jobject activity) {}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_JavaVM = vm;
    std::thread core(CoreThread);
    core.detach();
    return JNI_VERSION_1_6;
}

}
