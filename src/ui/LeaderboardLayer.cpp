#include "LeaderboardLayer.hpp"
#include "../core/Config.hpp"
#include "../core/Localization.hpp"
#include "Components/HoverButton.hpp"
namespace Stargrind::UI {
LeaderboardLayer* LeaderboardLayer::create() {
    auto ret = new LeaderboardLayer();
    if (ret->initAnchored(400.f, 350.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}
bool LeaderboardLayer::setup() {
    setTitle(LOC_KEY("LB_TITLE"));
    
    auto winSize = m_mainLayer->getContentSize();
    
    // RGB Background subtil
    m_background = RGBBackground::create();
    m_background->setContentSize(winSize);
    m_background->setLerpSpeed(0.2f);
    m_background->setZOrder(-10);
    m_mainLayer->addChild(m_background);
    
    // Container pour la liste
    m_listContainer = CCNode::create();
    m_listContainer->setPosition({winSize.width / 2, winSize.height / 2 + 20});
    m_mainLayer->addChild(m_listContainer);
    
    // Loading label
    m_loadingLabel = CCLabelBMFont::create("Loading...", "goldFont.fnt");
    m_loadingLabel->setPosition({winSize.width / 2, winSize.height / 2});
    m_loadingLabel->setScale(0.6f);
    m_mainLayer->addChild(m_loadingLabel);
    
    // Container pour le joueur local (en bas)
    m_localPlayerCard = CCNode::create();
    m_localPlayerCard->setPosition({winSize.width / 2, 60});
    m_localPlayerCard->setVisible(false);
    m_mainLayer->addChild(m_localPlayerCard);
    
    // Menu
    auto menu = CCMenu::create();
    menu->setPosition({0, 0});
    m_mainLayer->addChild(menu);
    
    // Bouton refresh
    auto refreshSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    refreshSpr->setScale(0.7f);
    auto refreshBtn = HoverButton::createWithSprite(refreshSpr, this, menu_selector(LeaderboardLayer::onRefresh));
    refreshBtn->setPosition({winSize.width - 35, winSize.height - 35});
    menu->addChild(refreshBtn);
    
    // Charger le leaderboard
    loadLeaderboard();
    
    return true;
}
void LeaderboardLayer::loadLeaderboard() {
    if (m_isLoading) return;
    
    m_isLoading = true;
    m_loadingLabel->setVisible(true);
    m_listContainer->removeAllChildren();
    
    Network::API::get()->getLeaderboard(
        [this](const std::vector<Network::LeaderboardEntry>& entries, 
               const Network::PlayerStats* localPlayer) {
            m_isLoading = false;
            m_loadingLabel->setVisible(false);
            displayLeaderboard(entries, localPlayer);
        },
        [this](const std::string& error) {
            m_isLoading = false;
            displayError(error);
        }
    );
}
void LeaderboardLayer::displayLeaderboard(const std::vector<Network::LeaderboardEntry>& entries,
                                          const Network::PlayerStats* localPlayer) {
    m_listContainer->removeAllChildren();
    
    // Headers
    float yPos = 100;
    auto headers = CCNode::create();
    headers->setPosition({0, yPos + 20});
    
    auto rankHeader = CCLabelBMFont::create(LOC_KEY("LB_RANK").c_str(), "goldFont.fnt");
    rankHeader->setPosition({-150, 0});
    rankHeader->setScale(0.4f);
    headers->addChild(rankHeader);
    
    auto playerHeader = CCLabelBMFont::create(LOC_KEY("LB_PLAYER").c_str(), "goldFont.fnt");
    playerHeader->setPosition({-50, 0});
    playerHeader->setScale(0.4f);
    headers->addChild(playerHeader);
    
    auto levelHeader = CCLabelBMFont::create(LOC_KEY("LB_LEVEL").c_str(), "goldFont.fnt");
    levelHeader->setPosition({80, 0});
    levelHeader->setScale(0.4f);
    headers->addChild(levelHeader);
    
    auto xpHeader = CCLabelBMFont::create(LOC_KEY("LB_XP").c_str(), "goldFont.fnt");
    xpHeader->setPosition({140, 0});
    xpHeader->setScale(0.4f);
    headers->addChild(xpHeader);
    
    m_listContainer->addChild(headers);
    
    // Liste des joueurs (Top 25)
    int displayCount = std::min(static_cast<int>(entries.size()), 25);
    for (int i = 0; i < displayCount; i++) {
        const auto& entry = entries[i];
        
        auto row = CCNode::create();
        row->setPosition({0, yPos - i * 18});
        
        // Couleur selon le rang
        ccColor3B color = {255, 255, 255};
        if (entry.rank == 1) color = {255, 215, 0}; // Or
        else if (entry.rank == 2) color = {192, 192, 192}; // Argent
        else if (entry.rank == 3) color = {205, 127, 50}; // Bronze
        
        // Highlight si c'est le joueur local
        if (localPlayer && entry.gd_id == Config::Utils::getPlayerId()) {
            color = {100, 255, 100};
        }
        
        auto rankLabel = CCLabelBMFont::create(
            fmt::format("#{}", entry.rank).c_str(),
            "chatFont.fnt"
        );
        rankLabel->setPosition({-150, 0});
        rankLabel->setScale(0.5f);
        rankLabel->setColor(color);
        row->addChild(rankLabel);
        
        auto nameLabel = CCLabelBMFont::create(entry.player_name.c_str(), "chatFont.fnt");
        nameLabel->setPosition({-50, 0});
        nameLabel->setScale(0.5f);
        nameLabel->setColor(color);
        nameLabel->setAnchorPoint({0, 0.5f});
        row->addChild(nameLabel);
        
        auto levelLabel = CCLabelBMFont::create(
            fmt::format("Lv.{}", entry.level).c_str(),
            "chatFont.fnt"
        );
        levelLabel->setPosition({80, 0});
        levelLabel->setScale(0.5f);
        levelLabel->setColor(color);
        row->addChild(levelLabel);
        
        auto xpLabel = CCLabelBMFont::create(
            std::to_string(entry.total_xp).c_str(),
            "chatFont.fnt"
        );
        xpLabel->setPosition({140, 0});
        xpLabel->setScale(0.5f);
        xpLabel->setColor(color);
        row->addChild(xpLabel);
        
        m_listContainer->addChild(row);
    }
    
    // Afficher le joueur local s'il n'est pas dans le top 25
    if (localPlayer && localPlayer->rank > 25) {
        m_localPlayerCard->setVisible(true);
        m_localPlayerCard->removeAllChildren();
        
        auto separator = CCLayerColor::create({100, 100, 100, 255}, 350, 2);
        separator->setPosition({-175, 25});
        m_localPlayerCard->addChild(separator);
        
        auto yourRankLabel = CCLabelBMFont::create(
            fmt::format("{}: #{}", LOC_KEY("LB_YOUR_RANK"), localPlayer->rank).c_str(),
            "goldFont.fnt"
        );
        yourRankLabel->setScale(0.5f);
        yourRankLabel->setColor({100, 255, 100});
        m_localPlayerCard->addChild(yourRankLabel);
        
        auto statsLabel = CCLabelBMFont::create(
            fmt::format("Lv.{} | {} XP | {} wins",
                localPlayer->level,
                localPlayer->total_xp,
                localPlayer->matches_won
            ).c_str(),
            "chatFont.fnt"
        );
        statsLabel->setPosition({0, -20});
        statsLabel->setScale(0.4f);
        m_localPlayerCard->addChild(statsLabel);
    }
}
void LeaderboardLayer::displayError(const std::string& error) {
    m_loadingLabel->setString(LOC_KEY("ERROR_CONNECTION").c_str());
    m_loadingLabel->setColor({255, 100, 100});
    log::error("Leaderboard error: {}", error);
}
void LeaderboardLayer::onRefresh(CCObject*) {
    loadLeaderboard();
}
void LeaderboardLayer::onClose(CCObject* sender) {
    m_background->setPaused(true);
    Popup::onClose(sender);
}
