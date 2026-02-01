#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "../network/API.hpp"
using namespace geode::prelude;
namespace Stargrind::UI {
class VersusLayer : public Popup<const Network::MatchInfo&> {
protected:
    bool setup(const Network::MatchInfo& match) override;
    void update(float dt) override;
    
    void animateTeams();
    void transitionToVote();
    
    Network::MatchInfo m_match;
    float m_timer = 0;
    
    CCNode* m_teamAContainer = nullptr;
    CCNode* m_teamBContainer = nullptr;
    CCLabelBMFont* m_vsLabel = nullptr;
    
public:
    static VersusLayer* create(const Network::MatchInfo& match);
};
