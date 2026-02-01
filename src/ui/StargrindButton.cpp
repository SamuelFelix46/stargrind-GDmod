#include "StargrindButton.hpp"
#include "../core/Config.hpp"
namespace Stargrind::UI {
StargrindButton* StargrindButton::create(CCObject* target, SEL_MenuHandler callback) {
    auto ret = new StargrindButton();
    if (ret->init(target, callback)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}
bool StargrindButton::init(CCObject* target, SEL_MenuHandler callback) {
    if (!CCNode::init()) return false;
    
    // Créer le sprite étoile
    m_starSprite = CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png");
    m_starSprite->setScale(0.5f);
    
    // Créer l'effet de lueur
    m_glowSprite = CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png");
    m_glowSprite->setScale(0.65f);
    m_glowSprite->setOpacity(100);
    m_glowSprite->setColor({255, 255, 100});
    m_glowSprite->setPosition(m_starSprite->getContentSize() / 2);
    m_glowSprite->setBlendFunc({GL_SRC_ALPHA, GL_ONE}); // Additive blending
    m_starSprite->addChild(m_glowSprite, -1);
    
    // Animation de lueur
    m_glowSprite->runAction(CCRepeatForever::create(
        CCSequence::create(
            CCFadeTo::create(1.0f, 50),
            CCFadeTo::create(1.0f, 150),
            nullptr
        )
    ));
    
    // Créer le bouton hover
    m_button = HoverButton::createWithSprite(m_starSprite, target, callback);
    m_button->setPosition({0, 0});
    addChild(m_button);
    
    // Démarrer l'animation de pulsation
    startPulseAnimation();
    
    setContentSize(m_starSprite->getContentSize() * 0.5f);
    
    return true;
}
void StargrindButton::setEnabled(bool enabled) {
    m_button->setEnabled(enabled);
    m_starSprite->setOpacity(enabled ? 255 : 128);
}
void StargrindButton::setPulsing(bool pulsing) {
    if (m_pulsing == pulsing) return;
    m_pulsing = pulsing;
    
    if (pulsing) {
        startPulseAnimation();
    } else {
        stopPulseAnimation();
    }
}
void StargrindButton::setNotificationBadge(int count) {
    if (count <= 0) {
        if (m_badge) {
            m_badge->removeFromParent();
            m_badge = nullptr;
        }
        return;
    }
    
    if (!m_badge) {
        m_badge = CCLabelBMFont::create("", "gjFont17.fnt");
        m_badge->setScale(0.5f);
        m_badge->setColor({255, 50, 50});
        m_badge->setPosition({
            m_starSprite->getContentSize().width * 0.3f,
            m_starSprite->getContentSize().height * 0.3f
        });
        m_starSprite->addChild(m_badge, 10);
    }
    
    m_badge->setString(std::to_string(count).c_str());
}
void StargrindButton::startPulseAnimation() {
    m_button->stopAllActions();
    m_button->runAction(CCRepeatForever::create(
        CCSequence::create(
            CCScaleTo::create(1.5f, 0.55f),
            CCScaleTo::create(1.5f, 0.5f),
            nullptr
        )
    ));
}
void StargrindButton::stopPulseAnimation() {
    m_button->stopAllActions();
    m_button->setScale(0.5f);
}
