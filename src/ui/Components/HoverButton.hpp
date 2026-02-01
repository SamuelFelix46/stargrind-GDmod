#pragma once
#include <Geode/Geode.hpp>
using namespace geode::prelude;
namespace Stargrind::UI {
class HoverButton : public CCMenuItemSpriteExtra {
public:
    static HoverButton* create(
        const std::string& text,
        CCObject* target,
        SEL_MenuHandler callback,
        const std::string& font = "goldFont.fnt",
        float scale = 1.0f
    );
    
    static HoverButton* createWithSprite(
        CCSprite* sprite,
        CCObject* target,
        SEL_MenuHandler callback
    );
    
    bool init(CCNode* sprite, CCObject* target, SEL_MenuHandler callback);
    
    void selected() override;
    void unselected() override;
    void activate() override;
    
    void setHoverScale(float scale) { m_hoverScale = scale; }
    void setHoverColor(const ccColor3B& color) { m_hoverColor = color; }
    void setNormalColor(const ccColor3B& color);
    void setPlaySound(bool play) { m_playSound = play; }
    
private:
    float m_originalScale = 1.0f;
    float m_hoverScale = 1.15f;
    ccColor3B m_normalColor = {255, 255, 255};
    ccColor3B m_hoverColor = {100, 255, 100}; // Vert
    bool m_playSound = true;
    CCLabelBMFont* m_label = nullptr;
};
} // namespace Stargrind::UI