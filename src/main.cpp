#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "ui/MainMenuLayer.hpp"
#include "ui/Components/HoverButton.hpp"
#include "core/Localization.hpp"
using namespace geode::prelude;
// ============================================
// HOOK: Menu Principal de GD
// ============================================
class $modify(StargrindMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        
        // Initialiser la localisation
        Stargrind::Localization::get();
        
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
        
        // Positionnement à côté des coffres (bottom right)
        // Trouver le menu des coffres
        if (auto rightMenu = this->getChildByID("right-side-menu")) {
            starBtn->setPosition({-30.f, 0.f});
            static_cast<CCMenu*>(rightMenu)->addChild(starBtn);
        } else {
            // Fallback: créer un nouveau menu
            auto menu = CCMenu::create();
            menu->setPosition({winSize.width - 60.f, 60.f});
            starBtn->setPosition({0, 0});
            menu->addChild(starBtn);
            this->addChild(menu, 10);
        }
        
        // Effet de notification (pulsation subtile)
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
// HOOK: Gameplay (pour détecter les étoiles gagnées)
// ============================================
#include <Geode/modify/PlayLayer.hpp>
class $modify(StargrindPlayLayer, PlayLayer) {
    void levelComplete() {
        PlayLayer::levelComplete();
        
        // Sync score si en match
        auto session = Stargrind::Logic::GameSession::get();
        if (session->getState() == Stargrind::Logic::SessionState::PLAYING ||
            session->getState() == Stargrind::Logic::SessionState::LAST_TRY) {
            session->syncScore();
        }
    }
    
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        
        // Si en mode "dernier essai" et mort, passer en spectateur
        auto session = Stargrind::Logic::GameSession::get();
        if (session->getState() == Stargrind::Logic::SessionState::LAST_TRY) {
            session->onLastTryComplete(false);
        }
    }
    
    // Hook pour le skip de niveau (flèche droite)
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        
        // Ajouter listener pour la touche flèche droite
        // (Le skip sera géré dans la logique du mod)
        
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