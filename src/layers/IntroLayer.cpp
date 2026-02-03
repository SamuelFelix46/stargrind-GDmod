#include "IntroLayer.hpp"
#include "../ui/ParticleSystem.hpp"
#include "LobbyLayer.hpp"

IntroLayer* IntroLayer::create() {
    auto ret = new IntroLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

CCScene* IntroLayer::scene() {
    auto scene = CCScene::create();
    scene->addChild(IntroLayer::create());
    return scene;
}

bool IntroLayer::init() {
    if (!CCLayer::init()) return false;
    
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto center = ccp(winSize.width / 2, winSize.height / 2);
    
    // ═══════════════════════════════════════════════════════════════
    // FOND NOIR DE BASE
    // ═══════════════════════════════════════════════════════════════
    auto blackBg = CCLayerColor::create(ccc4(0, 0, 0, 255));
    this->addChild(blackBg, -100);
    
    // ═══════════════════════════════════════════════════════════════
    // VORTEX TOURNANT (Effet de portail)
    // ═══════════════════════════════════════════════════════════════
    m_vortex = CCSprite::create("GJ_gradientBG.png");
    m_vortex->setPosition(center);
    m_vortex->setScale(0.1f);
    m_vortex->setColor(ccc3(80, 0, 120)); // Violet profond
    m_vortex->setOpacity(0);
    m_vortex->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
    this->addChild(m_vortex, -5);
    
    // Rotation continue du vortex
    m_vortex->runAction(CCRepeatForever::create(
        CCRotateBy::create(8.0f, 360.0f)
    ));
    
    // ═══════════════════════════════════════════════════════════════
    // STARBURST (Rayons de lumière)
    // ═══════════════════════════════════════════════════════════════
    m_starburstBg = CCSprite::create("GJ_gradientBG.png");
    m_starburstBg->setPosition(center);
    m_starburstBg->setScale(0.0f);
    m_starburstBg->setColor(ccc3(138, 43, 226));
    m_starburstBg->setOpacity(0);
    m_starburstBg->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
    this->addChild(m_starburstBg, -3);
    
    // Rotation inverse
    m_starburstBg->runAction(CCRepeatForever::create(
        CCRotateBy::create(15.0f, -360.0f)
    ));
    
    // ═══════════════════════════════════════════════════════════════
    // ANNEAU DE LUMIÈRE (Explosion circulaire)
    // ═══════════════════════════════════════════════════════════════
    m_glowRing = CCSprite::create("GJ_gradientBG.png");
    m_glowRing->setPosition(center);
    m_glowRing->setScale(0.0f);
    m_glowRing->setColor(ccc3(255, 200, 100));
    m_glowRing->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
    this->addChild(m_glowRing, 2);
    
    // ═══════════════════════════════════════════════════════════════
    // ÉTOILES EN ORBITE
    // ═══════════════════════════════════════════════════════════════
    for (int i = 0; i < 8; i++) {
        auto star = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        star->setPosition(center);
        star->setScale(0.0f);
        star->setOpacity(0);
        star->setColor(ccc3(255, 220, 100));
        this->addChild(star, 4);
        m_orbitingStars.push_back(star);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // TITRE "STARGRIND" - Lettres individuelles
    // ═══════════════════════════════════════════════════════════════
    std::string title = "STARGRIND";
    float letterSpacing = 45.0f;
    float startX = center.x - (title.length() - 1) * letterSpacing / 2;
    float titleY = center.y + 60;
    
    for (size_t i = 0; i < title.length(); i++) {
        std::string letter(1, title[i]);
        auto lbl = CCLabelBMFont::create(letter.c_str(), "goldFont.fnt");
        
        // Position de départ : en haut, hors écran, avec rotation
        lbl->setPosition(ccp(startX + i * letterSpacing, winSize.height + 100));
        lbl->setScale(0.0f);
        lbl->setOpacity(0);
        lbl->setRotation(-30.0f + (rand() % 60));
        this->addChild(lbl, 10);
        m_titleLetters.push_back(lbl);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // FLASH LAYER (Éclair blanc)
    // ═══════════════════════════════════════════════════════════════
    m_flashLayer = CCLayerColor::create(ccc4(255, 255, 255, 0));
    this->addChild(m_flashLayer, 50);
    
    // ═══════════════════════════════════════════════════════════════
    // SYSTÈMES DE PARTICULES
    // ═══════════════════════════════════════════════════════════════
    m_particles = ParticleSystem::createStarfield();
    m_particles->setPosition(center);
    m_particles->stopSystem();
    this->addChild(m_particles, 3);
    
    m_sparkles = ParticleSystem::createSparkles();
    m_sparkles->setPosition(center);
    m_sparkles->stopSystem();
    this->addChild(m_sparkles, 6);
    
    // Démarrer la séquence d'animation
    this->scheduleOnce(schedule_selector(IntroLayer::runIntroSequence), 0.2f);
    this->scheduleUpdate();
    
    return true;
}

void IntroLayer::runIntroSequence() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto center = ccp(winSize.width / 2, winSize.height / 2);
    
    // ═══════════════════════════════════════════════════════════════
    // T+0.0s : VORTEX apparaît et s'étend
    // ═══════════════════════════════════════════════════════════════
    m_vortex->runAction(CCSequence::create(
        CCSpawn::create(
            CCFadeTo::create(0.3f, 200),
            CCEaseExponentialOut::create(CCScaleTo::create(1.0f, 6.0f)),
            nullptr
        ),
        nullptr
    ));
    
    // Jouer le son whoosh
    FMODAudioEngine::sharedEngine()->playEffect("whoosh.mp3"_spr);
    
    // ═══════════════════════════════════════════════════════════════
    // T+0.3s : STARBURST explose
    // ═══════════════════════════════════════════════════════════════
    m_starburstBg->runAction(CCSequence::create(
        CCDelayTime::create(0.3f),
        CCSpawn::create(
            CCFadeTo::create(0.2f, 150),
            CCEaseExponentialOut::create(CCScaleTo::create(0.6f, 5.0f)),
            nullptr
        ),
        nullptr
    ));
    
    // ═══════════════════════════════════════════════════════════════
    // T+0.5s : ANNEAU DE LUMIÈRE explose depuis le centre
    // ═══════════════════════════════════════════════════════════════
    m_glowRing->runAction(CCSequence::create(
        CCDelayTime::create(0.5f),
        CCSpawn::create(
            CCScaleTo::create(0.3f, 4.0f),
            CCFadeTo::create(0.3f, 180),
            nullptr
        ),
        CCSpawn::create(
            CCScaleTo::create(0.4f, 10.0f),
            CCFadeTo::create(0.4f, 0),
            nullptr
        ),
        nullptr
    ));
    
    // ═══════════════════════════════════════════════════════════════
    // T+0.7s : ÉTOILES en orbite apparaissent
    // ═══════════════════════════════════════════════════════════════
    for (size_t i = 0; i < m_orbitingStars.size(); i++) {
        auto star = m_orbitingStars[i];
        float angle = (360.0f / m_orbitingStars.size()) * i;
        float radius = 120.0f;
        float delay = 0.7f + i * 0.05f;
        
        // Position finale en cercle
        float endX = center.x + cosf(CC_DEGREES_TO_RADIANS(angle)) * radius;
        float endY = center.y + sinf(CC_DEGREES_TO_RADIANS(angle)) * radius;
        
        star->runAction(CCSequence::create(
            CCDelayTime::create(delay),
            CCSpawn::create(
                CCFadeIn::create(0.2f),
                CCEaseBackOut::create(CCScaleTo::create(0.3f, 0.6f)),
                CCEaseBackOut::create(CCMoveTo::create(0.3f, ccp(endX, endY))),
                nullptr
            ),
            nullptr
        ));
        
        // Orbite continue après apparition
        star->runAction(CCSequence::create(
            CCDelayTime::create(delay + 0.4f),
            CCRepeatForever::create(CCSequence::create(
                CCRotateBy::create(0.5f, 360.0f),
                nullptr
            )),
            nullptr
        ));
    }
    
    // ═══════════════════════════════════════════════════════════════
    // T+0.9s : LETTRES tombent avec effet élastique
    // ═══════════════════════════════════════════════════════════════
    float titleY = center.y + 60;
    
    for (size_t i = 0; i < m_titleLetters.size(); i++) {
        auto letter = m_titleLetters[i];
        float delay = 0.9f + i * 0.07f;
        float targetX = letter->getPositionX();
        
        letter->runAction(CCSequence::create(
            CCDelayTime::create(delay),
            CCSpawn::create(
                CCFadeIn::create(0.1f),
                CCEaseBounceOut::create(CCMoveTo::create(0.5f, ccp(targetX, titleY))),
                CCEaseElasticOut::create(CCScaleTo::create(0.4f, 1.3f), 0.6f),
                CCRotateTo::create(0.3f, 0.0f),
                nullptr
            ),
            CCScaleTo::create(0.1f, 1.2f), // Rebond final
            nullptr
        ));
        
        // Effet de couleur pulsante sur chaque lettre
        auto colorCycle = CCRepeatForever::create(CCSequence::create(
            CCTintTo::create(0.4f, 255, 215, 0),   // Or
            CCTintTo::create(0.4f, 255, 150, 50),  // Orange
            CCTintTo::create(0.4f, 255, 100, 150), // Rose
            CCTintTo::create(0.4f, 150, 100, 255), // Violet
            CCTintTo::create(0.4f, 100, 200, 255), // Cyan
            nullptr
        ));
        
        letter->runAction(CCSequence::create(
            CCDelayTime::create(delay + 0.6f),
            colorCycle,
            nullptr
        ));
    }
    
    // ═══════════════════════════════════════════════════════════════
    // T+1.5s : FLASH BLANC
    // ═══════════════════════════════════════════════════════════════
    m_flashLayer->runAction(CCSequence::create(
        CCDelayTime::create(1.5f),
        CCFadeTo::create(0.03f, 220),
        CCFadeTo::create(0.3f, 0),
        nullptr
    ));
    
    // ═══════════════════════════════════════════════════════════════
    // T+1.5s : PARTICULES démarrent
    // ═══════════════════════════════════════════════════════════════
    this->scheduleOnce([this](float) {
        m_particles->resetSystem();
        m_sparkles->resetSystem();
    }, 1.5f, "start_particles");
    
    // ═══════════════════════════════════════════════════════════════
    // T+1.8s : SOUS-TITRE "TOURNAMENT" apparaît
    // ═══════════════════════════════════════════════════════════════
    auto subtitle = CCLabelBMFont::create("TOURNAMENT", "bigFont.fnt");
    subtitle->setPosition(ccp(center.x, center.y - 10));
    subtitle->setScale(0.0f);
    subtitle->setOpacity(0);
    subtitle->setColor(ccc3(200, 200, 255));
    this->addChild(subtitle, 10);
    
    subtitle->runAction(CCSequence::create(
        CCDelayTime::create(1.8f),
        CCSpawn::create(
            CCFadeIn::create(0.2f),
            CCEaseElasticOut::create(CCScaleTo::create(0.5f, 0.7f), 0.7f),
            nullptr
        ),
        nullptr
    ));
    
    // Sous-titre pulse
    subtitle->runAction(CCSequence::create(
        CCDelayTime::create(2.3f),
        CCRepeatForever::create(CCSequence::create(
            CCScaleTo::create(0.8f, 0.75f),
            CCScaleTo::create(0.8f, 0.7f),
            nullptr
        )),
        nullptr
    ));
    
    // ═══════════════════════════════════════════════════════════════
    // T+2.5s : "TAP TO CONTINUE" apparaît
    // ═══════════════════════════════════════════════════════════════
    auto tapLabel = CCLabelBMFont::create("TAP TO CONTINUE", "chatFont.fnt");
    tapLabel->setPosition(ccp(center.x, center.y - 80));
    tapLabel->setScale(0.0f);
    tapLabel->setOpacity(0);
    tapLabel->setColor(ccc3(180, 180, 180));
    this->addChild(tapLabel, 10);
    
    tapLabel->runAction(CCSequence::create(
        CCDelayTime::create(2.5f),
        CCSpawn::create(
            CCFadeIn::create(0.3f),
            CCScaleTo::create(0.3f, 0.5f),
            nullptr
        ),
        CCRepeatForever::create(CCSequence::create(
            CCFadeTo::create(0.6f, 120),
            CCFadeTo::create(0.6f, 255),
            nullptr
        )),
        nullptr
    ));
    
    // Activer le touch après l'animation
    this->scheduleOnce([this](float) {
        this->setTouchEnabled(true);
    }, 2.5f, "enable_touch");
}

void IntroLayer::update(float dt) {
    m_animTime += dt;
    
    // Faire orbiter les étoiles autour du centre
    if (m_animTime > 1.2f) {
        auto center = CCDirector::sharedDirector()->getWinSize() / 2;
        float orbitRadius = 120.0f;
        float orbitSpeed = 0.5f;
        
        for (size_t i = 0; i < m_orbitingStars.size(); i++) {
            auto star = m_orbitingStars[i];
            float baseAngle = (360.0f / m_orbitingStars.size()) * i;
            float currentAngle = baseAngle + m_animTime * 60.0f * orbitSpeed;
            
            float x = center.width + cosf(CC_DEGREES_TO_RADIANS(currentAngle)) * orbitRadius;
            float y = center.height + sinf(CC_DEGREES_TO_RADIANS(currentAngle)) * orbitRadius;
            
            star->setPosition(ccp(x, y));
        }
    }
}

void IntroLayer::onIntroComplete() {
    // Transition vers le lobby avec effet de zoom
    this->runAction(CCSequence::create(
        CCSpawn::create(
            CCScaleTo::create(0.4f, 1.5f),
            CCFadeOut::create(0.4f),
            nullptr
        ),
        CCCallFunc::create([this]() {
            auto scene = LobbyLayer::scene();
            CCDirector::sharedDirector()->replaceScene(
                CCTransitionFade::create(0.3f, scene, ccc3(0, 0, 0))
            );
        }),
        nullptr
    ));
}