#include "SpectatorLayer.hpp"
#include "EndGameLayer.hpp"
#include "../logic/RewardSystem.hpp"

SpectatorLayer* SpectatorLayer::create(float remainingTime, const std::vector<std::string>& activePlayers) {
    auto ret = new SpectatorLayer();
    if (ret && ret->init(remainingTime, activePlayers)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

CCScene* SpectatorLayer::scene(float remainingTime, const std::vector<std::string>& activePlayers) {
    auto scene = CCScene::create();
    scene->addChild(SpectatorLayer::create(remainingTime, activePlayers));
    return scene;
}

bool SpectatorLayer::init(float remainingTime, const std::vector<std::string>& activePlayers) {
    if (!CCLayer::init()) return false;
    
    m_remainingTime = remainingTime;
    m_activePlayers = activePlayers;
    
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto center = ccp(winSize.width / 2, winSize.height / 2);
    
    // ─────────────────────────────────────────────────────────────────
    // FOND NOIR AVEC GRADIENT ANIMÉ
    // ─────────────────────────────────────────────────────────────────
    auto blackBg = CCLayerColor::create(ccc4(5, 0, 15, 255));
    this->addChild(blackBg, -100);
    
    // Glow de fond animé
    m_backgroundGlow = CCSprite::create("GJ_gradientBG.png");
    m_backgroundGlow->setPosition(center);
    m_backgroundGlow->setScale(3.0f);
    m_backgroundGlow->setOpacity(40);
    m_backgroundGlow->setColor(ccc3(80, 40, 120));
    m_backgroundGlow->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
    this->addChild(m_backgroundGlow, -50);
    
    // Animation de rotation lente
    m_backgroundGlow->runAction(CCRepeatForever::create(
        CCRotateBy::create(30.0f, 360.0f)
    ));
    
    // ─────────────────────────────────────────────────────────────────
    // CONTENEUR DE PARTICULES AMBIANTES
    // ─────────────────────────────────────────────────────────────────
    m_particleContainer = CCNode::create();
    this->addChild(m_particleContainer, -40);
    
    // ─────────────────────────────────────────────────────────────────
    // ORBES FLOTTANTS
    // ─────────────────────────────────────────────────────────────────
    std::vector<ccColor3B> orbColors = {
        ccc3(100, 50, 180),   // Violet
        ccc3(50, 100, 200),   // Bleu
        ccc3(180, 50, 100),   // Rose
        ccc3(50, 150, 150),   // Cyan
    };
    
    for (int i = 0; i < 6; i++) {
        auto orb = CCSprite::create("GJ_gradientBG.png");
        float randomX = (rand() % (int)winSize.width);
        float randomY = (rand() % (int)winSize.height);
        orb->setPosition(ccp(randomX, randomY));
        orb->setScale(0.3f + (rand() % 100) / 200.0f);
        orb->setOpacity(60 + rand() % 60);
        orb->setColor(orbColors[rand() % orbColors.size()]);
        orb->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
        this->addChild(orb, -30);
        m_floatingOrbs.push_back(orb);
        
        // Mouvement flottant aléatoire
        float duration = 8.0f + (rand() % 40) / 10.0f;
        float moveX = (rand() % 200) - 100;
        float moveY = (rand() % 200) - 100;
        
        orb->runAction(CCRepeatForever::create(CCSequence::create(
            CCEaseInOut::create(CCMoveBy::create(duration, ccp(moveX, moveY)), 2.0f),
            CCEaseInOut::create(CCMoveBy::create(duration, ccp(-moveX, -moveY)), 2.0f),
            nullptr
        )));
        
        // Pulse d'opacité
        orb->runAction(CCRepeatForever::create(CCSequence::create(
            CCFadeTo::create(3.0f + (rand() % 20) / 10.0f, 40 + rand() % 40),
            CCFadeTo::create(3.0f + (rand() % 20) / 10.0f, 80 + rand() % 40),
            nullptr
        )));
    }
    
    // ─────────────────────────────────────────────────────────────────
    // TITRE "MODE SPECTATEUR"
    // ─────────────────────────────────────────────────────────────────
    m_statusLabel = CCLabelBMFont::create("MODE SPECTATEUR", "goldFont.fnt");
    m_statusLabel->setPosition(ccp(center.x, winSize.height - 50));
    m_statusLabel->setScale(0.8f);
    m_statusLabel->setColor(ccc3(200, 180, 255));
    this->addChild(m_statusLabel, 10);
    
    // Pulse léger
    m_statusLabel->runAction(CCRepeatForever::create(CCSequence::create(
        CCFadeTo::create(2.0f, 180),
        CCFadeTo::create(2.0f, 255),
        nullptr
    )));
    
    // ─────────────────────────────────────────────────────────────────
    // TIMER
    // ─────────────────────────────────────────────────────────────────
    int mins = (int)m_remainingTime / 60;
    int secs = (int)m_remainingTime % 60;
    char timerStr[16];
    snprintf(timerStr, sizeof(timerStr), "%02d:%02d", mins, secs);
    
    m_timerLabel = CCLabelBMFont::create(timerStr, "bigFont.fnt");
    m_timerLabel->setPosition(ccp(center.x, center.y + 50));
    m_timerLabel->setScale(1.2f);
    m_timerLabel->setColor(ccc3(255, 220, 100));
    this->addChild(m_timerLabel, 10);
    
    // ─────────────────────────────────────────────────────────────────
    // LISTE DES JOUEURS ACTIFS
    // ─────────────────────────────────────────────────────────────────
    auto playersTitle = CCLabelBMFont::create("Joueurs en jeu:", "bigFont.fnt");
    playersTitle->setPosition(ccp(center.x, center.y - 30));
    playersTitle->setScale(0.45f);
    playersTitle->setColor(ccc3(180, 180, 200));
    this->addChild(playersTitle, 10);
    
    m_playerListContainer = CCNode::create();
    m_playerListContainer->setPosition(ccp(center.x, center.y - 70));
    this->addChild(m_playerListContainer, 10);
    
    float rowY = 0;
    for (const auto& playerName : m_activePlayers) {
        auto nameLabel = CCLabelBMFont::create(playerName.c_str(), "bigFont.fnt");
        nameLabel->setScale(0.4f);
        nameLabel->setPositionY(rowY);
        nameLabel->setColor(ccc3(150, 200, 150));
        m_playerListContainer->addChild(nameLabel);
        rowY -= 25;
    }
    
    // ─────────────────────────────────────────────────────────────────
    // MESSAGE D'ATTENTE
    // ─────────────────────────────────────────────────────────────────
    auto waitLabel = CCLabelBMFont::create("En attente de la fin du match...", "chatFont.fnt");
    waitLabel->setPosition(ccp(center.x, 40));
    waitLabel->setScale(0.7f);
    waitLabel->setOpacity(150);
    this->addChild(waitLabel, 10);
    
    // Animation des points
    waitLabel->runAction(CCRepeatForever::create(CCSequence::create(
        CCFadeTo::create(0.5f, 100),
        CCFadeTo::create(0.5f, 200),
        nullptr
    )));
    
    // ─────────────────────────────────────────────────────────────────
    // DÉMARRER LES UPDATES
    // ─────────────────────────────────────────────────────────────────
    this->scheduleUpdate();
    this->schedule(schedule_selector(SpectatorLayer::spawnAmbientParticle), 0.3f);
    
    return true;
}

void SpectatorLayer::update(float dt) {
    // Mettre à jour le timer
    m_remainingTime -= dt;
    
    if (m_remainingTime <= 0) {
        m_remainingTime = 0;
        this->onMatchEnd();
        return;
    }
    
    int mins = (int)m_remainingTime / 60;
    int secs = (int)m_remainingTime % 60;
    char timerStr[16];
    snprintf(timerStr, sizeof(timerStr), "%02d:%02d", mins, secs);
    m_timerLabel->setString(timerStr);
    
    // Changer la couleur si moins d'1 minute
    if (m_remainingTime < 60) {
        m_timerLabel->setColor(ccc3(255, 100, 100));
    }
}

void SpectatorLayer::spawnAmbientParticle() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    auto particle = CCSprite::create("GJ_gradientBG.png");
    
    // Position aléatoire en bas
    float startX = rand() % (int)winSize.width;
    particle->setPosition(ccp(startX, -20));
    particle->setScale(0.01f + (rand() % 30) / 1000.0f);
    particle->setOpacity(100 + rand() % 100);
    
    std::vector<ccColor3B> colors = {
        ccc3(150, 100, 200),
        ccc3(100, 150, 200),
        ccc3(200, 100, 150),
        ccc3(255, 200, 100),
    };
    particle->setColor(colors[rand() % colors.size()]);
    particle->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
    
    m_particleContainer->addChild(particle);
    
    // Monter et disparaître
    float duration = 4.0f + (rand() % 30) / 10.0f;
    float drift = (rand() % 100 - 50);
    
    particle->runAction(CCSequence::create(
        CCSpawn::create(
            CCMoveBy::create(duration, ccp(drift, winSize.height + 40)),
            CCSequence::create(
                CCDelayTime::create(duration * 0.7f),
                CCFadeOut::create(duration * 0.3f),
                nullptr
            ),
            nullptr
        ),
        CCRemoveSelf::create(),
        nullptr
    ));
}

void SpectatorLayer::onMatchEnd() {
    this->unscheduleAllCallbacks();
    
    // TODO: Récupérer les vrais résultats depuis le réseau
    std::vector<PlayerResult> results;
    // ... remplir avec les données du match
    
    int winningTeam = 0; // TODO: Calculer l'équipe gagnante
    
    auto scene = EndGameLayer::scene(results, winningTeam);
    CCDirector::sharedDirector()->replaceScene(
        CCTransitionFade::create(0.5f, scene)
    );
}