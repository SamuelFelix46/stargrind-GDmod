#include "EndGameLayer.hpp"
#include "LobbyLayer.hpp"
#include "../ui/AnimatedBackground.hpp"

EndGameLayer* EndGameLayer::create(const std::vector<PlayerResult>& results, int winningTeam) {
    auto ret = new EndGameLayer();
    if (ret && ret->init(results, winningTeam)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

CCScene* EndGameLayer::scene(const std::vector<PlayerResult>& results, int winningTeam) {
    auto scene = CCScene::create();
    scene->addChild(EndGameLayer::create(results, winningTeam));
    return scene;
}

bool EndGameLayer::init(const std::vector<PlayerResult>& results, int winningTeam) {
    if (!CCLayer::init()) return false;
    
    m_results = results;
    m_winningTeam = winningTeam;
    
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto center = ccp(winSize.width / 2, winSize.height / 2);
    
    // ─────────────────────────────────────────────────────────────────
    // FOND ANIMÉ
    // ─────────────────────────────────────────────────────────────────
    auto bg = AnimatedBackground::create();
    this->addChild(bg, -10);
    
    // ─────────────────────────────────────────────────────────────────
    // TITRE "MATCH TERMINÉ"
    // ─────────────────────────────────────────────────────────────────
    auto titleLabel = CCLabelBMFont::create("MATCH TERMINE", "goldFont.fnt");
    titleLabel->setPosition(ccp(center.x, winSize.height - 50));
    titleLabel->setScale(0.9f);
    this->addChild(titleLabel, 10);
    
    // ─────────────────────────────────────────────────────────────────
    // ÉQUIPE GAGNANTE
    // ─────────────────────────────────────────────────────────────────
    std::string winnerText = (winningTeam == 0) ? "EQUIPE BLEUE GAGNE!" : "EQUIPE ROUGE GAGNE!";
    ccColor3B winnerColor = (winningTeam == 0) ? ccc3(100, 150, 255) : ccc3(255, 100, 100);
    
    m_winnerLabel = CCLabelBMFont::create(winnerText.c_str(), "bigFont.fnt");
    m_winnerLabel->setPosition(ccp(center.x, winSize.height - 100));
    m_winnerLabel->setScale(0.7f);
    m_winnerLabel->setColor(winnerColor);
    m_winnerLabel->setOpacity(0);
    this->addChild(m_winnerLabel, 10);
    
    // ─────────────────────────────────────────────────────────────────
    // CONTENEUR DES RÉSULTATS
    // ─────────────────────────────────────────────────────────────────
    m_resultsContainer = CCNode::create();
    m_resultsContainer->setPosition(ccp(center.x, center.y + 30));
    this->addChild(m_resultsContainer, 5);
    
    // Créer les lignes de résultats
    float rowHeight = 35.0f;
    float startY = (m_results.size() * rowHeight) / 2;
    
    for (size_t i = 0; i < m_results.size(); i++) {
        auto& player = m_results[i];
        
        auto row = CCNode::create();
        row->setPositionY(startY - i * rowHeight);
        row->setOpacity(0);
        
        // Rang MVP (si applicable)
        if (player.mvpRank > 0 && player.mvpRank <= 3) {
            std::string mvpText = "";
            ccColor3B mvpColor;
            switch (player.mvpRank) {
                case 1: mvpText = "MVP"; mvpColor = ccc3(255, 215, 0); break;
                case 2: mvpText = "2ND"; mvpColor = ccc3(192, 192, 192); break;
                case 3: mvpText = "3RD"; mvpColor = ccc3(205, 127, 50); break;
            }
            auto mvpLabel = CCLabelBMFont::create(mvpText.c_str(), "goldFont.fnt");
            mvpLabel->setScale(0.4f);
            mvpLabel->setPositionX(-180);
            mvpLabel->setColor(mvpColor);
            row->addChild(mvpLabel);
        }
        
        // Nom du joueur
        auto nameLabel = CCLabelBMFont::create(player.displayName.c_str(), "bigFont.fnt");
        nameLabel->setScale(0.45f);
        nameLabel->setPositionX(-80);
        nameLabel->setAnchorPoint(ccp(0, 0.5f));
        ccColor3B teamColor = (player.team == 0) ? ccc3(150, 180, 255) : ccc3(255, 150, 150);
        nameLabel->setColor(teamColor);
        row->addChild(nameLabel);
        
        // Étoiles
        auto starsLabel = CCLabelBMFont::create(
            (std::to_string(player.stars) + " ★").c_str(), 
            "goldFont.fnt"
        );
        starsLabel->setScale(0.4f);
        starsLabel->setPositionX(80);
        row->addChild(starsLabel);
        
        // XP gagné (animé)
        auto xpLabel = CCLabelBMFont::create("+0 XP", "bigFont.fnt");
        xpLabel->setScale(0.4f);
        xpLabel->setPositionX(160);
        xpLabel->setColor(ccc3(150, 255, 150));
        xpLabel->setTag(100 + i); // Pour référence dans l'animation
        row->addChild(xpLabel);
        
        m_resultsContainer->addChild(row);
    }
    
    // ─────────────────────────────────────────────────────────────────
    // BOUTON RETOUR AU LOBBY
    // ─────────────────────────────────────────────────────────────────
    auto backSpr = ButtonSprite::create("LOBBY", 100, true, "goldFont.fnt", "GJ_button_01.png", 35.0f, 1.0f);
    m_backButton = CCMenuItemSpriteExtra::create(
        backSpr,
        this,
        menu_selector(EndGameLayer::onBackToLobby)
    );
    m_backButton->setPosition(ccp(0, -winSize.height / 2 + 60));
    m_backButton->setOpacity(0);
    
    auto menu = CCMenu::create();
    menu->addChild(m_backButton);
    menu->setPosition(center);
    this->addChild(menu, 20);
    
    // ─────────────────────────────────────────────────────────────────
    // DÉMARRER LES ANIMATIONS
    // ─────────────────────────────────────────────────────────────────
    this->scheduleOnce(schedule_selector(EndGameLayer::animateResults), 0.5f);
    
    return true;
}

void EndGameLayer::animateResults() {
    // Animer le label gagnant
    m_winnerLabel->runAction(CCSequence::create(
        CCFadeIn::create(0.5f),
        CCRepeatForever::create(CCSequence::create(
            CCScaleTo::create(0.8f, 0.75f),
            CCScaleTo::create(0.8f, 0.7f),
            nullptr
        )),
        nullptr
    ));
    
    // Animer chaque ligne de résultat
    auto children = m_resultsContainer->getChildren();
    for (int i = 0; i < children->count(); i++) {
        auto row = static_cast<CCNode*>(children->objectAtIndex(i));
        float delay = 0.3f + i * 0.15f;
        
        row->runAction(CCSequence::create(
            CCDelayTime::create(delay),
            CCSpawn::create(
                CCFadeIn::create(0.3f),
                CCEaseBackOut::create(CCMoveBy::create(0.3f, ccp(0, -10))),
                nullptr
            ),
            nullptr
        ));
        
        // Animer l'XP
        auto xpLabel = static_cast<CCLabelBMFont*>(row->getChildByTag(100 + i));
        if (xpLabel && i < m_results.size()) {
            this->animateXPGain(xpLabel, m_results[i].xpGained, delay + 0.5f);
        }
    }
    
    // Bouton retour
    m_backButton->runAction(CCSequence::create(
        CCDelayTime::create(0.5f + m_results.size() * 0.15f + 1.0f),
        CCSpawn::create(
            CCFadeIn::create(0.3f),
            CCEaseBackOut::create(CCScaleTo::create(0.3f, 1.0f)),
            nullptr
        ),
        nullptr
    ));
}

void EndGameLayer::animateXPGain(CCLabelBMFont* label, int finalXP, float delay) {
    // Animation de comptage de l'XP
    label->runAction(CCSequence::create(
        CCDelayTime::create(delay),
        CCCallFuncN::create([finalXP](CCNode* node) {
            auto lbl = static_cast<CCLabelBMFont*>(node);
            
            // Simuler un comptage
            int steps = 20;
            float stepDuration = 0.8f / steps;
            
            for (int i = 0; i <= steps; i++) {
                int currentXP = (finalXP * i) / steps;
                
                lbl->runAction(CCSequence::create(
                    CCDelayTime::create(stepDuration * i),
                    CCCallFuncN::create([currentXP](CCNode* n) {
                        auto l = static_cast<CCLabelBMFont*>(n);
                        l->setString(("+" + std::to_string(currentXP) + " XP").c_str());
                    }),
                    nullptr
                ));
            }
        }),
        nullptr
    ));
}

void EndGameLayer::onBackToLobby(CCObject* sender) {
    FMODAudioEngine::sharedEngine()->playEffect("click.mp3"_spr);
    
    auto scene = LobbyLayer::scene();
    CCDirector::sharedDirector()->replaceScene(
        CCTransitionFade::create(0.5f, scene)
    );
}