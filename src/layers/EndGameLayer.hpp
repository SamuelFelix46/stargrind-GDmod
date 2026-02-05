#pragma once
#include <Geode/Geode.hpp>
#include "../logic/RewardSystem.hpp"
#include <vector>

using namespace geode::prelude;

class EndGameLayer : public CCLayer {
protected:
    bool init(const std::vector<PlayerResult>& results, int winningTeam);
    
    void animateResults();
    void animatePlayerRow(CCNode* row, const PlayerResult& player, float delay);
    void animateXPGain(CCLabelBMFont* label, int finalXP, float delay);
    void onBackToLobby(CCObject* sender);
    
    std::vector<PlayerResult> m_results;
    int m_winningTeam;
    
    CCNode* m_resultsContainer;
    CCLabelBMFont* m_winnerLabel;
    CCMenuItemSpriteExtra* m_backButton;
    
public:
    static EndGameLayer* create(const std::vector<PlayerResult>& results, int winningTeam);
    static CCScene* scene(const std::vector<PlayerResult>& results, int winningTeam);
};