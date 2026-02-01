#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "../network/API.hpp"
#include "Components/RGBBackground.hpp"
using namespace geode::prelude;
namespace Stargrind::UI {
class MatchLayer : public Popup<const Network::MatchInfo&> {
protected:
    bool setup(const Network::MatchInfo& match) override;
    void update(float dt) override;
    void onClose(CCObject*) override;
    
    // UI Updates
    void updateTimer();
    void updateScores(const Network::MatchInfo& scores);
    void showLastTryWarning();
    void showResultScreen(const std::string& result);
    
    // Actions
    void onSkipLevel(CCObject*);
    
    // Match data
    Network::MatchInfo m_match;
    float m_scoreSyncTimer = 0;
    bool m_matchEnded = false;
    
    // UI Elements
    CCLabelBMFont* m_timerLabel = nullptr;
    CCLabelBMFont* m_teamAScoreLabel = nullptr;
    CCLabelBMFont* m_teamBScoreLabel = nullptr;
    CCLabelBMFont* m_myStarsLabel = nullptr;
    CCNode* m_teamAPlayers = nullptr;
    CCNode* m_teamBPlayers = nullptr;
    CCNode* m_lastTryBanner = nullptr;
    
public:
    static MatchLayer* create(const Network::MatchInfo& match);
    static void show(const Network::MatchInfo& match);
    
    // External updates
    void refreshScores();
};
