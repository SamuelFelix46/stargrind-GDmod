#pragma once
#include <Geode/Geode.hpp>
#include "../utils/Localization.hpp"

using namespace geode::prelude;

class CoffeeButton : public CCMenuItemSpriteExtra {
protected:
    CCScale9Sprite* m_bg;
    CCSprite* m_icon;
    CCLabelBMFont* m_label;
    CCParticleSystemQuad* m_steam;
    
    bool m_isHovered = false;
    
    bool init();
    void onHover();
    void onHoverEnd();
    void onClick(CCObject* sender);
    
public:
    static CoffeeButton* create();
    
    void updateLanguage();
};