#include "CoffeeButton.hpp"

CoffeeButton* CoffeeButton::create() {
    auto ret = new CoffeeButton();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CoffeeButton::init() {
    // Container sprite
    auto container = CCSprite::create();
    container->setContentSize(CCSizeMake(160, 45));
    
    // Fond doré/café
    m_bg = CCScale9Sprite::create("GJ_button_01.png");
    m_bg->setContentSize(CCSizeMake(160, 45));
    m_bg->setPosition(ccp(80, 22.5f));
    m_bg->setColor(ccc3(255, 180, 80)); // Orange/doré
    container->addChild(m_bg);
    
    // Icône café (ou étoile en fallback)
    m_icon = CCSprite::create("coffee_icon.png"_spr);
    if (!m_icon) {
        m_icon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    }
    m_icon->setPosition(ccp(28, 22.5f));
    m_icon->setScale(0.7f);
    container->addChild(m_icon);
    
    // Texte
    std::string text = Localization::get()->getString("UI_COFFEE");
    m_label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
    m_label->setPosition(ccp(95, 22.5f));
    m_label->setScale(0.3f);
    m_label->setColor(ccc3(60, 40, 20)); // Marron foncé
    container->addChild(m_label);
    
    // Initialiser le bouton
    if (!CCMenuItemSpriteExtra::init(
        container,
        nullptr,
        this,
        menu_selector(CoffeeButton::onClick)
    )) {
        return false;
    }
    
    return true;
}

void CoffeeButton::onHover() {
    if (m_isHovered) return;
    m_isHovered = true;
    
    // Agrandir
    this->runAction(CCEaseBackOut::create(CCScaleTo::create(0.15f, 1.1f)));
    
    // Éclaircir le fond
    m_bg->runAction(CCTintTo::create(0.1f, 255, 200, 100));
    
    // Son hover
    FMODAudioEngine::sharedEngine()->playEffect("click.mp3"_spr);
}

void CoffeeButton::onHoverEnd() {
    if (!m_isHovered) return;
    m_isHovered = false;
    
    // Retour à la normale
    this->runAction(CCEaseBackOut::create(CCScaleTo::create(0.15f, 1.0f)));
    m_bg->runAction(CCTintTo::create(0.1f, 255, 180, 80));
}

void CoffeeButton::onClick(CCObject* sender) {
    // Son click
    FMODAudioEngine::sharedEngine()->playEffect("click.mp3"_spr);
    
    // Ouvrir le lien
    CCApplication::sharedApplication()->openURL("https://buymeacoffee.com/stargrind");
    
    // Petit feedback visuel
    auto heart = CCLabelBMFont::create("<3", "bigFont.fnt");
    heart->setPosition(this->getPosition() + ccp(0, 30));
    heart->setScale(0.6f);
    heart->setColor(ccc3(255, 100, 150));
    this->getParent()->addChild(heart, 100);
    
    heart->runAction(CCSequence::create(
        CCSpawn::create(
            CCMoveBy::create(0.6f, ccp(0, 40)),
            CCFadeOut::create(0.6f),
            nullptr
        ),
        CCRemoveSelf::create(),
        nullptr
    ));
}

void CoffeeButton::updateLanguage() {
    std::string text = Localization::get()->getString("UI_COFFEE");
    m_label->setString(text.c_str());
}