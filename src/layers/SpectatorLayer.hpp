#pragma once
#include <Geode/Geode.hpp>
#include <vector>

using namespace geode::prelude;

class SpectatorLayer : public CCLayer {
protected:
    bool init(float remainingTime, const std::vector<std::string>& activePlayers);
    
    void updateTimer(float dt);
    void animateBackground();
    void spawnAmbientParticle();
    void onMatchEnd();
    
    float m_remainingTime;
    std::vector<std::string> m_activePlayers;
    
    CCLabelBMFont* m_timerLabel;
    CCLabelBMFont* m_statusLabel;
    CCNode* m_playerListContainer;
    CCNode* m_particleContainer;
    
    // Effets visuels
    std::vector<CCSprite*> m_floatingOrbs;
    CCSprite* m_backgroundGlow;
    
public:
    static SpectatorLayer* create(float remainingTime, const std::vector<std::string>& activePlayers);
    static CCScene* scene(float remainingTime, const std::vector<std::string>& activePlayers);
    
    void update(float dt) override;
};