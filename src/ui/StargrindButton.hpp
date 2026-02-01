#pragma once
#include <Geode/Geode.hpp>
#include "Components/HoverButton.hpp"
using namespace geode::prelude;
namespace Stargrind::UI {
// ============================================
// BOUTON PERSONNALISÉ STARGRIND
// ============================================
class StargrindButton : public CCNode {
public:
    static StargrindButton* create(CCObject* target, SEL_MenuHandler callback);
    bool init(CCObject* target, SEL_MenuHandler callback);
    
    void setEnabled(bool enabled);
    void setPulsing(bool pulsing);
    void setNotificationBadge(int count);
    
private:
    CCSprite* m_starSprite = nullptr;
    CCSprite* m_glowSprite = nullptr;
    CCLabelBMFont* m_badge = nullptr;
    HoverButton* m_button = nullptr;
    
    bool m_pulsing = true;
    
    void startPulseAnimation();
    void stopPulseAnimation();
};
