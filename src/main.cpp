#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "ui/MainMenuLayer.hpp"
#include "ui/Components/HoverButton.hpp"
#include "core/Localization.hpp"
#include "network/P2PNetwork.hpp" // Ton header réseau
#include "core/GameSession.hpp"   // Pour la session

using namespace geode::prelude;

// ============================================
// HOOK: Menu Principal de GD
// ============================================
class $modify(StargrindMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        
        // Initialiser la localisation et le réseau
        Stargrind::Localization::get();
        Stargrind::Network::P2PNetwork::get()->initialize(7777);
        
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        // Créer le bouton Stargrind
        auto starSpr = CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png");
        starSpr->setScale(0.5f);
        
        // Ajouter un effet de brillance
        auto glow = CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png");
        glow->setScale(1.2f);
        glow->setOpacity(100);
        glow->setColor({255, 255, 100});
        glow->setPosition(starSpr->getContentSize() / 2);
        glow->runAction(CCRepeatForever::create(
            CCSequence::create(
                CCFadeTo::create(1.0f, 50),
                CCFadeTo::create(1.0f, 150),
                nullptr
            )
        ));
        starSpr->addChild(glow, -1);
        
        auto starBtn = Stargrind::UI::HoverButton::createWithSprite(
            starSpr,
            this,
            menu_selector(StargrindMenuLayer::onStargrind)
        );
        
        // Positionnement (bottom right)
        if (auto rightMenu = this->getChildByID("right-side-menu")) {
            starBtn->setPosition({-30.f, 0.f});
            static_cast<CCMenu*>(rightMenu)->addChild(starBtn);
        } else {
            auto menu = CCMenu::create();
            menu->setPosition({winSize.width - 60.f, 60.f});
            starBtn->setPosition({0, 0});
            menu->addChild(starBtn);
            this->addChild(menu, 10);
        }
        
        starBtn->runAction(CCRepeatForever::create(
            CCSequence::create(
                CCScaleTo::create(1.5f, 0.55f),
                CCScaleTo::create(1.5f, 0.5f),
                nullptr
            )
        ));
        
        return true;
    }
    
    void onStargrind(CCObject*) {
        Stargrind::UI::MainMenuLayer::show();
    }
};

// ============================================
// HOOK: Gameplay & Network Loop
// ============================================
class $modify(StargrindPlayLayer, PlayLayer) {
    
    void update(float dt) {
        PlayLayer::update(dt);

        // --- BOUCLE RÉSEAU ---
        // On lit les messages entrants à chaque frame du jeu
        Stargrind::Network::P2PNetwork::get()->processIncomingMessages();

        // Envoi d'un heartbeat toutes les 3 secondes pour rester connecté
        static float heartbeatTimer = 0;
        heartbeatTimer += dt;
        if (heartbeatTimer > 3.0f) {
            Stargrind::Network::P2PNetwork::get()->sendHeartbeat();
            heartbeatTimer = 0;
        }
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        
        // Sync score si en match
        auto session = Stargrind::Logic::GameSession::get();
        if (session->getState() == Stargrind::Logic::SessionState::PLAYING ||
            session->getState() == Stargrind::Logic::SessionState::LAST_TRY) {
            
            // On envoie physiquement la mise à jour via le réseau
            Stargrind::Network::P2PNetwork::get()->sendScoreUpdate(
                1, // +1 étoile
                m_level->m_levelID, 
                100.0f
            );
            
            session->syncScore();
        }
    }
    
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        
        auto session = Stargrind::Logic::GameSession::get();
        if (session->getState() == Stargrind::Logic::SessionState::LAST_TRY) {
            session->onLastTryComplete(false);
        }
    }
    
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        return true;
    }
};

// ============================================
// LIFECYCLE
// ============================================
$on_mod(Loaded) {
    log::info("Stargrind Tournament loaded!");
    
    // Précharger les ressources audio
    auto audioEngine = FMODAudioEngine::sharedEngine();
    auto resourcesDir = Mod::get()->getResourcesDir();
    
    audioEngine->preloadEffect((resourcesDir / "click.mp3").string());
    audioEngine->preloadEffect((resourcesDir / "hover.mp3").string());
    audioEngine->preloadEffect((resourcesDir / "match_found.mp3").string());
    audioEngine->preloadEffect((resourcesDir / "victory.mp3").string());
    audioEngine->preloadEffect((resourcesDir / "defeat.mp3").string());
}

$on_mod(Unloaded) {
    // Proprement fermer ENet quand le mod est déchargé
    Stargrind::Network::P2PNetwork::get()->shutdown();
}