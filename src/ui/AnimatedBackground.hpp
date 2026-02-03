#pragma once
#include <Geode/Geode.hpp>
#include "ParticleSystem.hpp"

using namespace geode::prelude;

class AnimatedBackground : public CCLayerColor {
protected:
    float m_hue = 0.0f;
    float m_saturation = 0.6f;
    float m_brightness = 0.25f;
    float m_speed = 0.015f;
    
    CCSprite* m_gradientOverlay;
    CCParticleSystemQuad* m_particles;
    std::vector<CCSprite*> m_orbs;
    
    bool init() override;
    void update(float dt) override;
    ccColor3B hsvToRgb(float h, float s, float v);
    void createOrbs(int count);
    
public:
    static AnimatedBackground* create();
    
    void setSpeed(float s) { m_speed = s; }
    void setColors(float sat, float bright) { 
        m_saturation = sat; 
        m_brightness = bright; 
    }
};