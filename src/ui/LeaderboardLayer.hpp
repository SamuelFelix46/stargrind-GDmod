#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "../network/API.hpp"
#include "Components/RGBBackground.hpp"
using namespace geode::prelude;
namespace Stargrind::UI {
class LeaderboardLayer : public Popup<> {
protected:
    bool setup() override;
    
    void loadLeaderboard();
    void displayLeaderboard(const std::vector<Network::LeaderboardEntry>& entries, 
                           const Network::PlayerStats* localPlayer);
    void displayError(const std::string& error);
    
    void onRefresh(CCObject*);
    void onClose(CCObject*) override;
    
    RGBBackground* m_background = nullptr;
    CCNode* m_listContainer = nullptr;
    CCNode* m_localPlayerCard = nullptr;
    CCLabelBMFont* m_loadingLabel = nullptr;
    
    bool m_isLoading = false;
    
public:
    static LeaderboardLayer* create();
};
} // namespace Stargrind::UI