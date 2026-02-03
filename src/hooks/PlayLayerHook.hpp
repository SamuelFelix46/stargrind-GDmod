#pragma once
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../logic/GameSession.hpp"
#include "../logic/LevelQueue.hpp"
#include "../ui/ParticleSystem.hpp"

using namespace geode::prelude;

class $modify(StargrindPlayLayer, PlayLayer) {
    struct Fields {
        bool m_isInTournament = false;
        
        // UI Elements
        CCLabelBMFont* m_timerLabel = nullptr;
        CCLabelBMFont* m_myScoreLabel = nullptr;
        CCLabelBMFont* m_opponentScoreLabel = nullptr;
        CCLabelBMFont* m_levelCountLabel = nullptr;
        CCSprite* m_skipIcon = nullptr;
        CCLabelBMFont* m_skipLabel = nullptr;
        CCLabelBMFont* m_lastAttemptLabel = nullptr;
        
        // État
        float m_lastTimerUpdate = 0.0f;
    };
    
    // ═══════════════════════════════════════════════════════════════
    // INITIALISATION
    // ═══════════════════════════════════════════════════════════════
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }
        
        // Vérifier si on est en mode tournoi
        auto session = GameSession::get();
        if (session->isPlaying()) {
            m_fields->m_isInTournament = true;
            setupTournamentUI();
            this->setKeyboardEnabled(true);
        }
        
        return true;
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SETUP UI TOURNOI
    // ═══════════════════════════════════════════════════════════════
    void setupTournamentUI() {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto session = GameSession::get();
        
        // ─────────────────────────────────────────────────────────
        // TIMER (Centre haut)
        // ─────────────────────────────────────────────────────────
        m_fields->m_timerLabel = CCLabelBMFont::create(
            session->getFormattedTime().c_str(), 
            "bigFont.fnt"
        );
        m_fields->m_timerLabel->setPosition(ccp(winSize.width / 2, winSize.height - 20));
        m_fields->m_timerLabel->setScale(0.45f);
        m_fields->m_timerLabel->setOpacity(200);
        m_fields->m_timerLabel->setColor(ccc3(255, 255, 255));
        this->addChild(m_fields->m_timerLabel, 1000);
        
        // ─────────────────────────────────────────────────────────
        // MON SCORE (Gauche)
        // ─────────────────────────────────────────────────────────
        std::string myScoreStr = fmt::format("{}★", session->getMyTeamScore());
        m_fields->m_myScoreLabel = CCLabelBMFont::create(myScoreStr.c_str(), "bigFont.fnt");
        m_fields->m_myScoreLabel->setPosition(ccp(50, winSize.height - 20));
        m_fields->m_myScoreLabel->setScale(0.4f);
        m_fields->m_myScoreLabel->setAnchorPoint(ccp(0, 0.5f));
        m_fields->m_myScoreLabel->setColor(ccc3(100, 255, 100)); // Vert
        this->addChild(m_fields->m_myScoreLabel, 1000);
        
        // ─────────────────────────────────────────────────────────
        // SCORE ADVERSAIRE (Droite du timer)
        // ─────────────────────────────────────────────────────────
        std::string oppScoreStr = fmt::format("{}★", session->getOpponentTeamScore());
        m_fields->m_opponentScoreLabel = CCLabelBMFont::create(oppScoreStr.c_str(), "bigFont.fnt");
        m_fields->m_opponentScoreLabel->setPosition(ccp(winSize.width - 50, winSize.height - 20));
        m_fields->m_opponentScoreLabel->setScale(0.4f);
        m_fields->m_opponentScoreLabel->setAnchorPoint(ccp(1, 0.5f));
        m_fields->m_opponentScoreLabel->setColor(ccc3(255, 100, 100)); // Rouge
        this->addChild(m_fields->m_opponentScoreLabel, 1000);
        
        // ─────────────────────────────────────────────────────────
        // COMPTEUR DE NIVEAU (Sous le timer)
        // ─────────────────────────────────────────────────────────
        std::string levelStr = fmt::format("Level #{}", LevelQueue::get()->getCurrentIndex());
        m_fields->m_levelCountLabel = CCLabelBMFont::create(levelStr.c_str(), "chatFont.fnt");
        m_fields->m_levelCountLabel->setPosition(ccp(winSize.width / 2, winSize.height - 40));
        m_fields->m_levelCountLabel->setScale(0.4f);
        m_fields->m_levelCountLabel->setOpacity(150);
        this->addChild(m_fields->m_levelCountLabel, 1000);
        
        // ─────────────────────────────────────────────────────────
        // INDICATEUR SKIP (Bas droite)
        // ─────────────────────────────────────────────────────────
        m_fields->m_skipIcon = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        m_fields->m_skipIcon->setPosition(ccp(winSize.width - 40, 45));
        m_fields->m_skipIcon->setScale(0.5f);
        m_fields->m_skipIcon->setOpacity(120);
        m_fields->m_skipIcon->setFlipX(false);
        this->addChild(m_fields->m_skipIcon, 1000);
        
        // Animation pulsante
        m_fields->m_skipIcon->runAction(CCRepeatForever::create(CCSequence::create(
            CCFadeTo::create(0.6f, 200),
            CCFadeTo::create(0.6f, 80),
            nullptr
        )));
        
        m_fields->m_skipLabel = CCLabelBMFont::create("SKIP [→]", "chatFont.fnt");
        m_fields->m_skipLabel->setPosition(ccp(winSize.width - 40, 25));
        m_fields->m_skipLabel->setScale(0.3f);
        m_fields->m_skipLabel->setOpacity(100);
        this->addChild(m_fields->m_skipLabel, 1000);
        
        // Scheduler pour update UI
        this->schedule(schedule_selector(StargrindPlayLayer::updateTournamentUI));
    }
    
    // ═══════════════════════════════════════════════════════════════
    // UPDATE UI
    // ═══════════════════════════════════════════════════════════════
    void updateTournamentUI(float dt) {
        if (!m_fields->m_isInTournament) return;
        
        auto session = GameSession::get();
        session->update(dt);
        
        // ─────────────────────────────────────────────────────────
        // Update Timer
        // ─────────────────────────────────────────────────────────
        m_fields->m_timerLabel->setString(session->getFormattedTime().c_str());
        
        int remaining = session->getRemainingSeconds();
        
        // Couleur dynamique du timer
        if (remaining <= 30) {
            // Dernières 30 secondes : rouge clignotant
            float pulse = (sinf(remaining * 8.0f) + 1.0f) / 2.0f;
            m_fields->m_timerLabel->setColor(ccc3(255, 50 + pulse * 50, 50));
            m_fields->m_timerLabel->setScale(0.45f + pulse * 0.08f);
        } else if (remaining <= 60) {
            // Dernière minute : rouge fixe
            m_fields->m_timerLabel->setColor(ccc3(255, 80, 80));
        } else if (remaining <= 300) {
            // Dernières 5 minutes : orange
            m_fields->m_timerLabel->setColor(ccc3(255, 180, 80));
        } else {
            m_fields->m_timerLabel->setColor(ccc3(255, 255, 255));
        }
        
        // ─────────────────────────────────────────────────────────
        // Update Scores
        // ─────────────────────────────────────────────────────────
        m_fields->m_myScoreLabel->setString(
            fmt::format("{}★", session->getMyTeamScore()).c_str()
        );
        m_fields->m_opponentScoreLabel->setString(
            fmt::format("{}★", session->getOpponentTeamScore()).c_str()
        );
        
        // ─────────────────────────────────────────────────────────
        // Update Level Count
        // ─────────────────────────────────────────────────────────
        m_fields->m_levelCountLabel->setString(
            fmt::format("Level #{}", LevelQueue::get()->getCurrentIndex()).c_str()
        );
        
        // ─────────────────────────────────────────────────────────
        // Mode "Last Attempt"
        // ─────────────────────────────────────────────────────────
        if (session->getState() == SessionState::LastAttempt && 
            !m_fields->m_lastAttemptLabel) {
            
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            m_fields->m_lastAttemptLabel = CCLabelBMFont::create(
                "LAST ATTEMPT!", 
                "goldFont.fnt"
            );
            m_fields->m_lastAttemptLabel->setPosition(ccp(winSize.width / 2, winSize.height - 65));
            m_fields->m_lastAttemptLabel->setScale(0.0f);
            m_fields->m_lastAttemptLabel->setColor(ccc3(255, 100, 100));
            this->addChild(m_fields->m_lastAttemptLabel, 1000);
            
            // Animation d'apparition
            m_fields->m_lastAttemptLabel->runAction(CCSequence::create(
                CCEaseElasticOut::create(CCScaleTo::create(0.5f, 0.55f), 0.7f),
                CCRepeatForever::create(CCSequence::create(
                    CCScaleTo::create(0.4f, 0.6f),
                    CCScaleTo::create(0.4f, 0.55f),
                    nullptr
                )),
                nullptr
            ));
            
            // Cacher l'indicateur de skip
            m_fields->m_skipIcon->setVisible(false);
            m_fields->m_skipLabel->setVisible(false);
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // GESTION DES TOUCHES
    // ═══════════════════════════════════════════════════════════════
    void keyDown(cocos2d::enumKeyCodes key) {
        if (m_fields->m_isInTournament) {
            auto session = GameSession::get();
            
            // Skip avec flèche droite (seulement si pas en "last attempt")
            if (key == KEY_Right && session->getState() == SessionState::Playing) {
                performSkip();
                return;
            }
        }
        
        PlayLayer::keyDown(key);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SKIP FLUIDE (Pas d'écran intermédiaire)
    // ═══════════════════════════════════════════════════════════════
    void performSkip() {
        // Effet visuel de confirmation
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        auto skipText = CCLabelBMFont::create("SKIPPED!", "bigFont.fnt");
        skipText->setPosition(ccp(winSize.width / 2, winSize.height / 2));
        skipText->setScale(0.0f);
        skipText->setColor(ccc3(255, 200, 100));
        this->addChild(skipText, 2000);
        
        skipText->runAction(CCSequence::create(
            CCSpawn::create(
                CCEaseBackOut::create(CCScaleTo::create(0.2f, 0.8f)),
                CCFadeIn::create(0.1f),
                nullptr
            ),
            CCDelayTime::create(0.2f),
            CCSpawn::create(
                CCScaleTo::create(0.2f, 1.2f),
                CCFadeOut::create(0.2f),
                nullptr
            ),
            CCRemoveSelf::create(),
            nullptr
        ));
        
        // Déclencher le skip
        LevelQueue::get()->skipCurrent();
    }
    
    // ═══════════════════════════════════════════════════════════════
    // NIVEAU COMPLÉTÉ
    // ═══════════════════════════════════════════════════════════════
    void levelComplete() {
        if (m_fields->m_isInTournament) {
            auto session = GameSession::get();
            
            // Effet de victoire
            auto particles = ParticleSystem::createVictoryBurst();
            particles->setPosition(m_player1->getPosition());
            this->addChild(particles, 500);
            
            // Si dernier essai terminé → spectateur
            if (session->getState() == SessionState::LastAttempt) {
                session->setState(SessionState::Spectating);
                showSpectatorMode();
                return;
            }
            
            // Sinon → niveau suivant automatiquement
            LevelQueue::get()->onLevelComplete();
            return;
        }
        
        PlayLayer::levelComplete();
    }
    
    // ═══════════════════════════════════════════════════════════════
    // MORT DU JOUEUR
    // ═══════════════════════════════════════════════════════════════
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (m_fields->m_isInTournament) {
            auto session = GameSession::get();
            
            LevelQueue::get()->onLevelFail();
            
            // Si dernier essai et mort → spectateur
            if (session->getState() == SessionState::LastAttempt) {
                session->setState(SessionState::Spectating);
                showSpectatorMode();
                return;
            }
        }
        
        PlayLayer::destroyPlayer(player, obj);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // MODE SPECTATEUR
    // ═══════════════════════════════════════════════════════════════
    void showSpectatorMode() {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        // Overlay sombre
        auto overlay = CCLayerColor::create(ccc4(0, 0, 0, 180));
        this->addChild(overlay, 900);
        
        // Message
        auto waitingLabel = CCLabelBMFont::create(
            "Waiting for other players...", 
            "bigFont.fnt"
        );
        waitingLabel->setPosition(ccp(winSize.width / 2, winSize.height / 2));
        waitingLabel->setScale(0.6f);
        this->addChild(waitingLabel, 1000);
        
        // Animation
        waitingLabel->runAction(CCRepeatForever::create(CCSequence::create(
            CCFadeTo::create(0.8f, 150),
            CCFadeTo::create(0.8f, 255),
            nullptr
        )));
        
        // TODO: Afficher les scores en temps réel des autres joueurs
    }
    
    // ═══════════════════════════════════════════════════════════════
    // UPDATE PROGRESS
    // ═══════════════════════════════════════════════════════════════
    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        
        if (m_fields->m_isInTournament && m_percentLabel) {
            // Récupérer le pourcentage actuel
            std::string percentStr = m_percentLabel->getString();
            int percent = 0;
            sscanf(percentStr.c_str(), "%d", &percent);
            
            GameSession::get()->onProgress(percent);
        }
    }
};