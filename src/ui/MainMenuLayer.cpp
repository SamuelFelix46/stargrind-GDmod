#include "MainMenuLayer.hpp"
#include "LobbyLayer.hpp"
#include "LeaderboardLayer.hpp"
#include "SettingsLayer.hpp"
#include "../core/Config.hpp"
#include "../core/Localization.hpp"
namespace Stargrind::UI {
MainMenuLayer* MainMenuLayer::create() {
    auto ret = new MainMenuLayer();
    if (ret->initAnchored(400.f, 300.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}
void MainMenuLayer::show() {
    create()->Popup::show();
}
bool MainMenuLayer::setup() {
    // RGB Background
    m_background = RGBBackground::create();
    m_background->setContentSize(m_mainLayer->getContentSize());
    m_background->setZOrder(-10);
    m_mainLayer->addChild(m_background);
    
    // Title
    auto title = CCLabelBMFont::create(LOC_KEY("MENU_TITLE").c_str(), "goldFont.fnt");
    title->setPosition({200.f, 270.f});
    title->setScale(0.8f);
    m_mainLayer->addChild(title);
    
    // Star decoration
    auto star = CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png");
    star->setPosition({200.f, 220.f});
    star->setScale(0.6f);
    star->runAction(CCRepeatForever::create(
        CCSequence::create(
            CCRotateBy::create(2.0f, 360.0f),
            nullptr
        )
    ));
    m_mainLayer->addChild(star);
    
    // Menu
    m_menu = CCMenu::create();
    m_menu->setPosition({0, 0});
    m_mainLayer->addChild(m_menu);
    
    // PLAY Button (Center Right, prominent)
    auto playBtn = HoverButton::create(
        LOC_KEY("MENU_PLAY"),
        this,
        menu_selector(MainMenuLayer::onPlay),
        "bigFont.fnt",
        1.2f
    );
    playBtn->setPosition({320.f, 150.f});
    playBtn->setHoverScale(1.2f);
    m_menu->addChild(playBtn);
    
    // Leaderboard Button (Center)
    auto lbBtn = HoverButton::create(
        LOC_KEY("MENU_LEADERBOARD"),
        this,
        menu_selector(MainMenuLayer::onLeaderboard),
        "goldFont.fnt",
        0.7f
    );
    lbBtn->setPosition({200.f, 130.f});
    m_menu->addChild(lbBtn);
    
    // Settings Button (Bottom Right)
    auto settingsSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingsSpr->setScale(0.7f);
    auto settingsBtn = HoverButton::createWithSprite(settingsSpr, this, menu_selector(MainMenuLayer::onSettings));
    settingsBtn->setPosition({370.f, 30.f});
    m_menu->addChild(settingsBtn);
    
    // Buy me a coffee Button (Top Left)
    auto coffeeBtn = HoverButton::create(
        "☕ " + LOC_KEY("MENU_COFFEE"),
        this,
        menu_selector(MainMenuLayer::onCoffee),
        "chatFont.fnt",
        0.5f
    );
    coffeeBtn->setPosition({80.f, 280.f});
    coffeeBtn->setNormalColor({255, 200, 100});
    coffeeBtn->setHoverColor({255, 255, 150});
    m_menu->addChild(coffeeBtn);
    
    animateIn();
    return true;
}
void MainMenuLayer::animateIn() {
    // Fade in animation
    m_mainLayer->setOpacity(0);
    m_mainLayer->runAction(CCFadeIn::create(0.3f));
    
    // Scale bounce for title
    auto title = m_mainLayer->getChildren()->objectAtIndex(1);
    if (title) {
        auto node = static_cast<CCNode*>(title);
        node->setScale(0.1f);
        node->runAction(CCEaseBackOut::create(CCScaleTo::create(0.4f, 0.8f)));
    }
}
void MainMenuLayer::onPlay(CCObject*) {
    onClose(nullptr);
    LobbyLayer::create()->show();
}
void MainMenuLayer::onLeaderboard(CCObject*) {
    LeaderboardLayer::create()->show();
}
void MainMenuLayer::onSettings(CCObject*) {
    SettingsLayer::create()->show();
}
void MainMenuLayer::onCoffee(CCObject*) {
    CCApplication::sharedApplication()->openURL(Config::COFFEE_URL.c_str());
}
void MainMenuLayer::onClose(CCObject* sender) {
    m_background->setPaused(true);
    Popup::onClose(sender);
}
} // namespace Stargrind::UI