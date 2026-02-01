#include "MatchLayer.hpp"
#include "../core/Config.hpp"
#include "../core/Localization.hpp"
#include "../logic/GameSession.hpp"
#include "../network/P2PNetwork.hpp"
#include "Components/HoverButton.hpp"
namespace Stargrind::UI {
MatchLayer* MatchLayer::create(const Network::MatchInfo& match) {
    auto ret = new MatchLayer();
    if (ret->initAnchored(500.f, 350.f, match)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}
void MatchLayer::show(const Network::MatchInfo& match) {
    create(match)->Popup::show();
}
bool MatchLayer::setup(const Network::MatchInfo& match) {
    m_match = match;
    
    setTitle(LOC_KEY("MATCH_TITLE"));
    
    auto winSize = m_mainLayer->getContentSize();
    
    // Timer en haut
    m_timerLabel = CCLabelBMFont::create("30:00", "bigFont.fnt");
    m_timerLabel->setPosition({winSize.width / 2, winSize.height - 40});
    m_timerLabel->setScale(0.7f);
    m_mainLayer->addChild(m_timerLabel);
    
    auto timeLabel = CCLabelBMFont::create(LOC_KEY("MATCH_TIME_LEFT").c_str(), "goldFont.fnt");
    timeLabel->setPosition({winSize.width / 2, winSize.height - 60});
    timeLabel->setScale(0.4f);
    timeLabel->setColor({150, 150, 150});
    m_mainLayer->addChild(timeLabel);
    
    // Scores des équipes
    // Team A (gauche)
    auto teamATitle = CCLabelBMFont::create(LOC_KEY("MATCH_YOUR_TEAM").c_str(), "goldFont.fnt");
    teamATitle->setPosition({120, winSize.height - 90});
    teamATitle->setScale(0.5f);
    teamATitle->setColor(m_match.your_team == "A" ? ccColor3B{100, 255, 100} : ccColor3B{200, 200, 200});
    m_mainLayer->addChild(teamATitle);
    
    m_teamAScoreLabel = CCLabelBMFont::create("0 ⭐", "bigFont.fnt");
    m_teamAScoreLabel->setPosition({120, winSize.height - 120});
    m_teamAScoreLabel->setScale(0.6f);
    m_mainLayer->addChild(m_teamAScoreLabel);
    
    m_teamAPlayers = CCNode::create();
    m_teamAPlayers->setPosition({120, winSize.height - 160});
    m_mainLayer->addChild(m_teamAPlayers);
    
    // Team B (droite)
    auto teamBTitle = CCLabelBMFont::create(LOC_KEY("MATCH_ENEMY_TEAM").c_str(), "goldFont.fnt");
    teamBTitle->setPosition({380, winSize.height - 90});
    teamBTitle->setScale(0.5f);
    teamBTitle->setColor(m_match.your_team == "B" ? ccColor3B{100, 255, 100} : ccColor3B{200, 200, 200});
    m_mainLayer->addChild(teamBTitle);
    
    m_teamBScoreLabel = CCLabelBMFont::create("0 ⭐", "bigFont.fnt");
    m_teamBScoreLabel->setPosition({380, winSize.height - 120});
    m_teamBScoreLabel->setScale(0.6f);
    m_mainLayer->addChild(m_teamBScoreLabel);
    
    m_teamBPlayers = CCNode::create();
    m_teamBPlayers->setPosition({380, winSize.height - 160});
    m_mainLayer->addChild(m_teamBPlayers);
    
    // Mes étoiles (en bas)
    m_myStarsLabel = CCLabelBMFont::create("My Stars: 0", "goldFont.fnt");
    m_myStarsLabel->setPosition({winSize.width / 2, 70});
    m_myStarsLabel->setScale(0.6f);
    m_mainLayer->addChild(m_myStarsLabel);
    
    // Menu pour les actions
    auto menu = CCMenu::create();
    menu->setPosition({0, 0});
    m_mainLayer->addChild(menu);
    
    // Bouton Skip
    auto skipBtn = HoverButton::create(
        "→ " + LOC_KEY("MATCH_SKIP_LEVEL"),
        this,
        menu_selector(MatchLayer::onSkipLevel),
        "goldFont.fnt",
        0.5f
    );
    skipBtn->setPosition({winSize.width / 2, 35});
    menu->addChild(skipBtn);
    
    // Initialiser les scores des joueurs
    updateScores(match);
    
    // Démarrer le timer du match
    Logic::GameSession::get()->startMatchTimer();
    Logic::GameSession::get()->setOnTimeUpdate([this](float remaining) {
        updateTimer();
    });
    Logic::GameSession::get()->setOnScoresUpdated([this](const Network::MatchInfo& scores) {
        updateScores(scores);
    });
    
    // Écouter les mises à jour P2P
    Network::P2PNetwork::get()->setOnScoreUpdate([this](const std::vector<Network::ScoreData>& scores) {
        // Mettre à jour l'affichage
        refreshScores();
    });
    
    scheduleUpdate();
    
    return true;
}
void MatchLayer::update(float dt) {
    if (m_matchEnded) return;
    
    auto session = Logic::GameSession::get();
    
    // Mettre à jour le timer
    updateTimer();
    
    // Mettre à jour mes étoiles locales
    int myStars = session->getStarsGained();
    m_myStarsLabel->setString(fmt::format("My Stars: {}", myStars).c_str());
    
    // Sync périodique du score
    m_scoreSyncTimer += dt;
    if (m_scoreSyncTimer >= Config::SCORE_SYNC_INTERVAL) {
        m_scoreSyncTimer = 0;
        session->syncScore();
        refreshScores();
    }
    
    // Vérifier l'état de la session
    auto state = session->getState();
    
    if (state == Logic::SessionState::LAST_TRY && !m_lastTryBanner) {
        showLastTryWarning();
    }
    
    if (state == Logic::SessionState::SPECTATING && !m_matchEnded) {
        m_matchEnded = true;
        // TODO: Afficher l'écran de spectateur/attente
    }
    
    if (state == Logic::SessionState::RESULTS) {
        m_matchEnded = true;
        showResultScreen(m_match.leader);
    }
}
void MatchLayer::updateTimer() {
    float remaining = Logic::GameSession::get()->getTimeRemaining();
    
    int minutes = static_cast<int>(remaining) / 60;
    int seconds = static_cast<int>(remaining) % 60;
    
    std::string timeStr = fmt::format("{:02}:{:02}", minutes, seconds);
    m_timerLabel->setString(timeStr.c_str());
    
    // Changer la couleur si peu de temps
    if (remaining < 60) {
        m_timerLabel->setColor({255, 100, 100});
    } else if (remaining < 300) {
        m_timerLabel->setColor({255, 200, 100});
    }
}
void MatchLayer::updateScores(const Network::MatchInfo& scores) {
    m_teamAScoreLabel->setString(fmt::format("{} ⭐", scores.team_a_total).c_str());
    m_teamBScoreLabel->setString(fmt::format("{} ⭐", scores.team_b_total).c_str());
    
    // Mettre à jour les listes de joueurs
    m_teamAPlayers->removeAllChildren();
    float yOffset = 0;
    for (const auto& player : scores.team_a) {
        auto label = CCLabelBMFont::create(
            fmt::format("{}: {}", player.player_name, player.stars_count).c_str(),
            "chatFont.fnt"
        );
        label->setPosition({0, yOffset});
        label->setScale(0.5f);
        
        if (player.gd_id == Config::Utils::getPlayerId()) {
            label->setColor({255, 255, 100});
        }
        
        m_teamAPlayers->addChild(label);
        yOffset -= 20;
    }
    
    m_teamBPlayers->removeAllChildren();
    yOffset = 0;
    for (const auto& player : scores.team_b) {
        auto label = CCLabelBMFont::create(
            fmt::format("{}: {}", player.player_name, player.stars_count).c_str(),
            "chatFont.fnt"
        );
        label->setPosition({0, yOffset});
        label->setScale(0.5f);
        
        if (player.gd_id == Config::Utils::getPlayerId()) {
            label->setColor({255, 255, 100});
        }
        
        m_teamBPlayers->addChild(label);
        yOffset -= 20;
    }
    
    // Highlight l'équipe en tête
    if (scores.team_a_total > scores.team_b_total) {
        m_teamAScoreLabel->setColor({100, 255, 100});
        m_teamBScoreLabel->setColor({255, 255, 255});
    } else if (scores.team_b_total > scores.team_a_total) {
        m_teamBScoreLabel->setColor({100, 255, 100});
        m_teamAScoreLabel->setColor({255, 255, 255});
    } else {
        m_teamAScoreLabel->setColor({255, 200, 100});
        m_teamBScoreLabel->setColor({255, 200, 100});
    }
}
void MatchLayer::showLastTryWarning() {
    auto winSize = m_mainLayer->getContentSize();
    
    m_lastTryBanner = CCNode::create();
    m_lastTryBanner->setPosition({winSize.width / 2, winSize.height / 2});
    m_mainLayer->addChild(m_lastTryBanner, 100);
    
    auto bg = CCLayerColor::create({255, 50, 50, 200}, 300, 50);
    bg->setPosition({-150, -25});
    m_lastTryBanner->addChild(bg);
    
    auto label = CCLabelBMFont::create(LOC_KEY("MATCH_LAST_TRY").c_str(), "bigFont.fnt");
    label->setScale(0.6f);
    m_lastTryBanner->addChild(label);
    
    // Animation
    m_lastTryBanner->setScale(0);
    m_lastTryBanner->runAction(CCSequence::create(
        CCEaseBackOut::create(CCScaleTo::create(0.3f, 1.0f)),
        CCDelayTime::create(2.0f),
        CCFadeOut::create(0.5f),
        nullptr
    ));
}
void MatchLayer::showResultScreen(const std::string& result) {
    // TODO: Implémenter l'écran de résultats
    // Avec calcul des XP, MVP, etc.
    
    bool isWinner = (result == m_match.your_team);
    
    auto path = Mod::get()->getResourcesDir() / (isWinner ? "victory.mp3" : "defeat.mp3");
    FMODAudioEngine::sharedEngine()->playEffect(path.string());
    
    log::info("Match ended! Result: {}", result);
}
void MatchLayer::refreshScores() {
    Network::API::get()->getScores(
        m_match.match_id,
        [this](const Network::MatchInfo& scores) {
            m_match.team_a_total = scores.team_a_total;
            m_match.team_b_total = scores.team_b_total;
            m_match.team_a = scores.team_a;
            m_match.team_b = scores.team_b;
            m_match.leader = scores.leader;
            
            updateScores(m_match);
        },
        [](const std::string& error) {
            log::error("Failed to refresh scores: {}", error);
        }
    );
}
void MatchLayer::onSkipLevel(CCObject*) {
    Logic::GameSession::get()->skipLevel();
    
    // TODO: Charger le prochain niveau
    // GameLevelManager::sharedState()->...
}
void MatchLayer::onClose(CCObject* sender) {
    // Ne pas permettre de fermer pendant un match actif
    auto state = Logic::GameSession::get()->getState();
    if (state == Logic::SessionState::PLAYING || 
        state == Logic::SessionState::LAST_TRY) {
        return; // Ignorer
    }
    
    Popup::onClose(sender);
}
