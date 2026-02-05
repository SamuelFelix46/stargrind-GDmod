#pragma once
#include <cmath>

class Easing {
public:
    // ═══════════════════════════════════════════════════════════════
    // FONCTIONS D'EASING
    // ═══════════════════════════════════════════════════════════════
    
    static float linear(float t) {
        return t;
    }
    
    static float easeInQuad(float t) {
        return t * t;
    }
    
    static float easeOutQuad(float t) {
        return t * (2.0f - t);
    }
    
    static float easeInOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    }
    
    static float easeInCubic(float t) {
        return t * t * t;
    }
    
    static float easeOutCubic(float t) {
        float f = t - 1.0f;
        return f * f * f + 1.0f;
    }
    
    static float easeInOutCubic(float t) {
        return t < 0.5f 
            ? 4.0f * t * t * t 
            : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
    }
    
    static float easeOutExpo(float t) {
        return t == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
    }
    
    static float easeInExpo(float t) {
        return t == 0.0f ? 0.0f : powf(2.0f, 10.0f * (t - 1.0f));
    }
    
    static float easeOutBack(float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;
        return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
    }
    
    static float easeOutElastic(float t) {
        const float c4 = (2.0f * M_PI) / 3.0f;
        return t == 0.0f ? 0.0f 
            : t == 1.0f ? 1.0f 
            : powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
    }
};
