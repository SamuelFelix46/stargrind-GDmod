#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class IntroLayer : public CCLayer {
protected:
    bool init() override;
    void runIntroSequence();
    void onIntroComplete();
    
    // Éléments visuels
    CCSprite* m_logo;
    CCSprite* m_glowRing;
    CCSprite* m_starburstBg;
    CCSprite* m_vortex;
    CCParticleSystemQuad* m_particles;
    CCParticleSystemQuad* m_sparkles;
    CCLayerColor* m_flashLayer;
    
    // Lettres animées
    std::vector<CCLabelBMFont*> m_titleLetters;
    std::vector<CCSprite*> m_orbitingStars;
    
    float m_animTime = 0.0f;
    
public:
    static IntroLayer* create();
    static CCScene* scene();
    
    void update(float dt) override;
};