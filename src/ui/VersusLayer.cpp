#include "VersusLayer.hpp"
#include "MatchLayer.hpp"
#include "../core/Config.hpp"
#include "../core/Localization.hpp"
#include "../logic/VoteSystem.hpp"
#include "../logic/GameSession.hpp"
namespace Stargrind::UI {
VersusLayer* VersusLayer::create(const Network::MatchInfo& match) {
    auto ret = new VersusLayer();
    if (ret->initAnchored(450.f, 320.f, match)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}
bool VersusLayer::setup(const Network::MatchInfo& match) {
    m_match = match;
    m_timer = Config::VERSUS_SCREEN_DURATION;
    
    setTitle(LOC_KEY("VERSUS_TITLE"));
    
    auto winSize = m_mainLayer->getContentSize();
    auto center = winSize / 2;
    
    // Background gris
    auto bg = CCLayerColor::create({50, 50, 60, 255});
    bg->setContentSize(winSize);
    bg->setZOrder(-10);
    m_mainLayer->addChild(bg);
    
    // Conteneur Team A (gauche)
    m_teamAContainer = CCNode::create();
    m_teamAContainer->setPosition({100.f, center.y});
    m_mainLayer->addChild(m_teamAContainer);
    
    auto teamATitle = CCLabelBMFont::create(LOC_KEY("VERSUS_TEAM_A").c_str(), "goldFont.fnt");
    teamATitle->setPosition({0, 100});
    teamATitle->setScale(0.6f);
    teamATitle->setColor({100, 150, 255});
    m_teamAContainer->addChild(teamATitle);
    
    float yOffset = 60;
    for (const auto& player : match.team_a) {
        auto label = CCLabelBMFont::create(player.player_name.c_str(), "chatFont.fnt");
        label->setPosition({0, yOffset});
        label->setScale(0.7f);
        
        // Highlight le joueur local
        if (player.gd_id == Config::Utils::getPlayerId()) {
            label->setColor({255, 255, 100});
        }
        
        m_teamAContainer->addChild(label);
        yOffset -= 25;
    }
    
    // Score total Team A
    auto scoreA = CCLabelBMFont::create(
        fmt::format("Total: {}", match.team_a_total).c_str(),
        "gjFont17.fnt"
    );
    scoreA->setPosition({0, -60});
    scoreA->setScale(0.5f);
    scoreA->setColor({150, 150, 150});
    m_teamAContainer->addChild(scoreA);
    
    // VS au centre
    m_vsLabel = CCLabelBMFont::create("VS", "bigFont.fnt");
    m_vsLabel->setPosition(center);
    m_vsLabel->setScale(1.5f);
    m_vsLabel->setColor({255, 50, 50});
    m_mainLayer->addChild(m_vsLabel);
    
    // Conteneur Team B (droite)
    m_teamBContainer = CCNode::create();
    m_teamBContainer->setPosition({350.f, center.y});
    m_mainLayer->addChild(m_teamBContainer);
    
    auto teamBTitle = CCLabelBMFont::create(LOC_KEY("VERSUS_TEAM_B").c_str(), "goldFont.fnt");
    teamBTitle->setPosition({0, 100});
    teamBTitle->setScale(0.6f);
    teamBTitle->setColor({255, 100, 100});
    m_teamBContainer->addChild(teamBTitle);
    
    yOffset = 60;
    for (const auto& player : match.team_b) {
        auto label = CCLabelBMFont::create(player.player_name.c_str(), "chatFont.fnt");
        label->setPosition({0, yOffset});
        label->setScale(0.7f);
        
        if (player.gd_id == Config::Utils::getPlayerId()) {
            label->setColor({255, 255, 100});
        }
        
        m_teamBContainer->addChild(label);
        yOffset -= 25;
    }
    
    // Score total Team B
    auto scoreB = CCLabelBMFont::create(
        fmt::format("Total: {}", match.team_b_total).c_str(),
        "gjFont17.fnt"
    );
    scoreB->setPosition({0, -60});
    scoreB->setScale(0.5f);
    scoreB->setColor({150, 150, 150});
    m_teamBContainer->addChild(scoreB);
    
    // Animer les équipes
    animateTeams();
    
    scheduleUpdate();
    
    return true;
}
void VersusLayer::animateTeams() {
    // Team A slide depuis la gauche
    m_teamAContainer->setPositionX(-100);
    m_teamAContainer->runAction(CCEaseBackOut::create(
        CCMoveTo::create(0.5f, {100.f, m_teamAContainer->getPositionY()})
    ));
    
    // Team B slide depuis la droite
    m_teamBContainer->setPositionX(550);
    m_teamBContainer->runAction(CCEaseBackOut::create(
        CCMoveTo::create(0.5f, {350.f, m_teamBContainer->getPositionY()})
    ));
    
    // VS scale bounce
    m_vsLabel->setScale(0);
    m_vsLabel->runAction(CCSequence::create(
        CCDelayTime::create(0.3f),
        CCEaseBackOut::create(CCScaleTo::create(0.3f, 1.5f)),
        CCRepeatForever::create(
            CCSequence::create(
                CCScaleTo::create(0.5f, 1.6f),
                CCScaleTo::create(0.5f, 1.5f),
                nullptr
            )
        ),
        nullptr
    ));
}
void VersusLayer::update(float dt) {
    m_timer -= dt;
    
    if (m_timer <= 0) {
        unscheduleUpdate();
        transitionToVote();
    }
}
void VersusLayer::transitionToVote() {
    // Mettre à jour la session de jeu
    Logic::GameSession::get()->startMatch(m_match);
    
    // Démarrer le système de vote
    int playerCount = m_match.team_a.size() + m_match.team_b.size();
    Logic::VoteSystem::get()->startVoting(m_match.match_id, playerCount);
    
    // TODO: Transition vers VoteLayer (ou MatchLayer si pas de vote)
    // Pour l'instant, on passe directement au match
    onClose(nullptr);
    MatchLayer::create(m_match)->show();
}
} // namespace Stargrind::UI