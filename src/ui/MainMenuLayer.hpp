#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "Components/RGBBackground.hpp"
#include "Components/HoverButton.hpp"
using namespace geode::prelude;
namespace Stargrind::UI {
class MainMenuLayer : public Popup<> {
protected:
    bool setup() override;
    void onClose(CCObject*) override;
    
    // Button callbacks
    void onPlay(CCObject*);
    void onLeaderboard(CCObject*);
    void onSettings(CCObject*);
    void onCoffee(CCObject*);
    
    // Animation
    void animateIn();
    
    RGBBackground* m_background = nullptr;
    CCMenu* m_menu = nullptr;
    
public:
    static MainMenuLayer* create();
    static void show();
};
} // namespace Stargrind::UI