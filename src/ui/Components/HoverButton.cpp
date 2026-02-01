#include "HoverButton.hpp"
#include "../../core/Config.hpp"
namespace Stargrind::UI {
    HoverButton* HoverButton::create(
        const std::string& text,
        CCObject* target,
        SEL_MenuHandler callback,
        const std::string& font,
        float scale
    ) {
        auto label = CCLabelBMFont::create(text.c_str(), font.c_str());
        label->setScale(scale);

        auto ret = new HoverButton();
        if (ret->init(label, target, callback)) {
            ret->m_label = label;
            ret->m_originalScale = scale;
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
    HoverButton* HoverButton::createWithSprite(
        CCSprite* sprite,
        CCObject* target,
        SEL_MenuHandler callback
    ) {
        auto ret = new HoverButton();
        if (ret->init(sprite, target, callback)) {
            ret->m_originalScale = sprite->getScale();
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
    bool HoverButton::init(CCNode* sprite, CCObject* target, SEL_MenuHandler callback) {
        if (!CCMenuItemSpriteExtra::init(sprite, sprite, target, callback)) return false;
        return true;
    }
    void HoverButton::selected() {
        CCMenuItemSpriteExtra::selected();

        // Animation d'agrandissement
        stopAllActions();
        runAction(CCEaseSineOut::create(
            CCScaleTo::create(Config::BUTTON_ANIM_DURATION, m_originalScale * m_hoverScale)
        ));

        // Changement de couleur si c'est un label
        if (m_label) {
            m_label->setColor(m_hoverColor);
        }

        // Son de survol (optionnel)
        // FMODAudioEngine::sharedEngine()->playEffect("hover.mp3");
    }
    void HoverButton::unselected() {
        CCMenuItemSpriteExtra::unselected();

        // Retour � la taille normale
        stopAllActions();
        runAction(CCEaseSineOut::create(
            CCScaleTo::create(Config::BUTTON_ANIM_DURATION, m_originalScale)
        ));

        // Retour � la couleur normale
        if (m_label) {
            m_label->setColor(m_normalColor);
        }
    }
    void HoverButton::activate() {
        CCMenuItemSpriteExtra::activate();

        // Son de clic
        if (m_playSound) {
            auto path = Mod::get()->getResourcesDir() / "click.mp3";
            FMODAudioEngine::sharedEngine()->playEffect(path.string());
        }
    }
    void HoverButton::setNormalColor(const ccColor3B& color) {
        m_normalColor = color;
        if (m_label) {
            m_label->setColor(color);
        }
    }
