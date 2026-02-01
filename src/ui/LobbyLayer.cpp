#include "LobbyLayer.hpp"
#include "VersusLayer.hpp"
#include "../core/Config.hpp"
#include "../core/Localization.hpp"
#include "../logic/Matchmaking.hpp"
#include "../logic/GameSession.hpp"
#include "Components/HoverButton.hpp"
namespace Stargrind::UI {
LobbyLayer* LobbyLayer::create() {
    auto ret = new LobbyLayer();
    if (ret->initAnchored(350.f, 250.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}
bool LobbyLayer::setup() {
    setTitle(LOC_KEY("QUEUE_TITLE"));
    
    // RGB Background
    m_background = RGBBackground::create();
    m_background->setContentSize(m_mainLayer->getContentSize());
    m_background->setZOrder(-10);
    m_mainLayer->addChild(m_background);
    
    auto center = m_mainLayer->getContentSize() / 2;
    
    // Loading circle animation
    m_loadingCircle = CCNode::create();
    m_loadingCircle->setPosition(center + ccp(0, 30));
    
    for (int i = 0; i < 8; i++) {
        auto dot = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        dot->setScale(0.4f);
        float angle = i * 45.0f * (M_PI / 180.0f);
        dot->setPosition({
            std::cos(angle) * 40.0f,
            std::sin(angle) * 40.0f
        });
        dot->setOpacity(255 - i * 25);
        m_loadingCircle->addChild(dot);
    }
    m_loadingCircle->runAction(CCRepeatForever::create(
        CCRotateBy::create(1.0f, 360.0f)
    ));
    m_mainLayer->addChild(m_loadingCircle);
    
    // Count label (0/10)
    m_countLabel = CCLabelBMFont::create("0/10", "bigFont.fnt");
    m_countLabel->setPosition(center + ccp(0, -20));
    m_countLabel->setScale(0.8f);
    m_mainLayer->addChild(m_countLabel);
    
    // Status label
    m_statusLabel = CCLabelBMFont::create(LOC_KEY("QUEUE_WAITING").c_str(), "goldFont.fnt");
    m_statusLabel->setPosition(center + ccp(0, -50));
    m_statusLabel->setScale(0.5f);
    m_statusLabel->setColor({200, 200, 200});
    m_mainLayer->addChild(m_statusLabel);
    
    // Cancel button
    auto menu = CCMenu::create();
    menu->setPosition({0, 0});
    m_mainLayer->addChild(menu);
    
    auto cancelBtn = HoverButton::create(
        LOC_KEY("QUEUE_CANCEL"),
        this,
        menu_selector(LobbyLayer::onCancel),
        "goldFont.fnt",
        0.6f
    );
    cancelBtn->setPosition({center.x, 35.f});
    cancelBtn->setNormalColor({255, 100, 100});
    cancelBtn->setHoverColor({255, 150, 150});
    menu->addChild(cancelBtn);
    
    // Démarrer la recherche
    auto matchmaking = Logic::Matchmaking::get();
    matchmaking->setOnQueueUpdated([this](int current, int needed) {
        updateQueueCount(current);
    });
    matchmaking->setOnMatchFound([this](const Network::MatchInfo& match) {
        onMatchFound();
    });
    matchmaking->startSearch();
    m_searching = true;
    
    scheduleUpdate();
    
    return true;
}
void LobbyLayer::update(float dt) {
    if (!m_searching) return;
    
    Logic::Matchmaking::get()->update(dt);
}
void LobbyLayer::updateQueueCount(int count) {
    std::string text = std::to_string(count) + "/" + std::to_string(Config::PLAYERS_PER_MATCH);
    m_countLabel->setString(text.c_str());
    
    // Animation de scale
    m_countLabel->stopAllActions();
    m_countLabel->setScale(1.0f);
    m_countLabel->runAction(CCSequence::create(
        CCScaleTo::create(0.1f, 0.9f),
        CCScaleTo::create(0.1f, 0.8f),
        nullptr
    ));
    
    // Mettre à jour le statut
    if (count >= Config::PLAYERS_PER_MATCH - 2) {
        m_statusLabel->setString("Almost ready...");
        m_statusLabel->setColor({100, 255, 100});
    }
}
void LobbyLayer::onMatchFound() {
    m_searching = false;
    
    // Animation de succès
    m_loadingCircle->stopAllActions();
    m_statusLabel->setString(LOC_KEY("QUEUE_FOUND").c_str());
    m_statusLabel->setColor({100, 255, 100});
    
    // Flash effet
    auto flash = CCLayerColor::create({255, 255, 255, 0});
    flash->setContentSize(m_mainLayer->getContentSize());
    m_mainLayer->addChild(flash, 100);
    flash->runAction(CCSequence::create(
        CCFadeTo::create(0.1f, 150),
        CCFadeTo::create(0.2f, 0),
        CCRemoveSelf::create(),
        nullptr
    ));
    
    // Transition vers VersusLayer après un délai
    runAction(CCSequence::create(
        CCDelayTime::create(1.0f),
        CCCallFunc::create(this, callfunc_selector(LobbyLayer::transitionToVersus)),
        nullptr
    ));
}
void LobbyLayer::transitionToVersus() {
    onClose(nullptr);
    
    auto match = Logic::Matchmaking::get()->getCurrentMatch();
    VersusLayer::create(match)->show();
}
void LobbyLayer::onCancel(CCObject*) {
    m_searching = false;
    Logic::Matchmaking::get()->cancelSearch();
    onClose(nullptr);
}
void LobbyLayer::onClose(CCObject* sender) {
    if (m_searching) {
        Logic::Matchmaking::get()->cancelSearch();
    }
    m_background->setPaused(true);
    Popup::onClose(sender);
}
