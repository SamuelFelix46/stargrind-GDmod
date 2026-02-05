#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <vector>
#include <cmath>

using namespace geode::prelude;

class IntroLayer : public CCLayer {
protected:
    bool init() override;
    
    // ═══════════════════════════════════════════════════════════════
    // PHASES D'ANIMATION
    // ═══════════════════════════════════════════════════════════════
    void startPhase1_BlackScreen();
    void startPhase2_Explosion();
    void startPhase3_Retraction();
    void startPhase4_TextReveal();
    void startPhase5_Stabilization();
    
    // ═══════════════════════════════════════════════════════════════
    // EFFETS
    // ═══════════════════════════════════════════════════════════════
    void shakeScreen(float intensity, float duration, float frequency);
    void spawnExplosionParticles();
    void spawnRetractionParticles();
    void animateLetterSpacing();
    
    // ═══════════════════════════════════════════════════════════════
    // CALLBACKS
    // ═══════════════════════════════════════════════════════════════
    void onEnterClicked(CCObject* sender);
    void onIntroComplete();
    
    // ═══════════════════════════════════════════════════════════════
    // ÉLÉMENTS VISUELS
    // ═══════════════════════════════════════════════════════════════
    CCLayerColor* m_blackBackground;
    CCSprite* m_explosionCircle;
    CCSprite* m_glowCenter;
    CCLayerColor* m_flashLayer;
    
    std::vector<CCSprite*> m_explosionParticles;
    std::vector<CCSprite*> m_retractionParticles;
    std::vector<CCLabelBMFont*> m_titleLetters;
    
    CCMenuItemSpriteExtra* m_enterButton;
    CCLabelBMFont* m_subtitleLabel;
    
    // ═══════════════════════════════════════════════════════════════
    // ÉTAT
    // ═══════════════════════════════════════════════════════════════
    CCPoint m_center;
    CCSize m_winSize;
    float m_letterSpacingProgress = 0.0f;
    float m_targetLetterSpacing = 55.0f;
    bool m_isAnimatingLetters = false;
    
public:
    static IntroLayer* create();
    static CCScene* scene();
    
    void update(float dt) override;
};