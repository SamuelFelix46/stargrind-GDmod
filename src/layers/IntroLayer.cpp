#include "IntroLayer.hpp"
#include "LobbyLayer.hpp"
#include "../utils/Easing.hpp"

// ═══════════════════════════════════════════════════════════════════════════
// CRÉATION
// ═══════════════════════════════════════════════════════════════════════════

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

// ═══════════════════════════════════════════════════════════════════════════
// INITIALISATION
// ═══════════════════════════════════════════════════════════════════════════

bool IntroLayer::init() {
    if (!CCLayer::init()) return false;
    
    m_winSize = CCDirector::sharedDirector()->getWinSize();
    m_center = ccp(m_winSize.width / 2, m_winSize.height / 2);
    
    // ─────────────────────────────────────────────────────────────────
    // FOND NOIR ABSOLU
    // ─────────────────────────────────────────────────────────────────
    m_blackBackground = CCLayerColor::create(ccc4(0, 0, 0, 255));
    this->addChild(m_blackBackground, -100);
    
    // ─────────────────────────────────────────────────────────────────
    // CERCLE D'EXPLOSION (caché au début)
    // ─────────────────────────────────────────────────────────────────
    m_explosionCircle = CCSprite::create("GJ_gradientBG.png");
    m_explosionCircle->setPosition(m_center);
    m_explosionCircle->setScale(0.0f);
    m_explosionCircle->setOpacity(0);
    m_explosionCircle->setColor(ccc3(255, 200, 100)); // Or/Orange
    m_explosionCircle->setBlendFunc({GL_SRC_ALPHA, GL_ONE}); // Additive blending
    this->addChild(m_explosionCircle, 1);
    
    // ─────────────────────────────────────────────────────────────────
    // GLOW CENTRAL (pour la rétraction)
    // ─────────────────────────────────────────────────────────────────
    m_glowCenter = CCSprite::create("GJ_gradientBG.png");
    m_glowCenter->setPosition(m_center);
    m_glowCenter->setScale(0.0f);
    m_glowCenter->setOpacity(0);
    m_glowCenter->setColor(ccc3(255, 150, 50));
    m_glowCenter->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
    this->addChild(m_glowCenter, 5);
    
    // ─────────────────────────────────────────────────────────────────
    // FLASH LAYER
    // ─────────────────────────────────────────────────────────────────
    m_flashLayer = CCLayerColor::create(ccc4(255, 255, 255, 0));
    this->addChild(m_flashLayer, 200);
    
    // ─────────────────────────────────────────────────────────────────
    // LETTRES "STARGRIND" (cachées au début)
    // ─────────────────────────────────────────────────────────────────
    std::string title = "STARGRIND";
    
    for (size_t i = 0; i < title.length(); i++) {
        std::string letter(1, title[i]);
        auto lbl = CCLabelBMFont::create(letter.c_str(), "goldFont.fnt");
        lbl->setPosition(m_center); // Toutes au centre au début
        lbl->setScale(1.0f);
        lbl->setOpacity(0);
        lbl->setColor(ccc3(255, 215, 0)); // Or
        this->addChild(lbl, 100);
        m_titleLetters.push_back(lbl);
    }
    
    // ─────────────────────────────────────────────────────────────────
    // SOUS-TITRE "TOURNAMENT" (caché)
    // ─────────────────────────────────────────────────────────────────
    m_subtitleLabel = CCLabelBMFont::create("TOURNAMENT", "bigFont.fnt");
    m_subtitleLabel->setPosition(ccp(m_center.x, m_center.y - 60));
    m_subtitleLabel->setScale(0.5f);
    m_subtitleLabel->setOpacity(0);
    m_subtitleLabel->setColor(ccc3(180, 180, 220));
    this->addChild(m_subtitleLabel, 100);
    
    // ─────────────────────────────────────────────────────────────────
    // BOUTON ENTER (caché)
    // ─────────────────────────────────────────────────────────────────
    auto enterSpr = ButtonSprite::create("ENTER", 120, true, "goldFont.fnt", "GJ_button_01.png", 40.0f, 1.0f);
    m_enterButton = CCMenuItemSpriteExtra::create(
        enterSpr,
        this,
        menu_selector(IntroLayer::onEnterClicked)
    );
    m_enterButton->setPosition(ccp(0, -120));
    m_enterButton->setOpacity(0);
    m_enterButton->setVisible(false);
    
    auto menu = CCMenu::create();
    menu->addChild(m_enterButton);
    menu->setPosition(m_center);
    this->addChild(menu, 150);
    
    // ─────────────────────────────────────────────────────────────────
    // DÉMARRER L'ANIMATION
    // ─────────────────────────────────────────────────────────────────
    this->scheduleUpdate();
    this->scheduleOnce(schedule_selector(IntroLayer::startPhase1_BlackScreen), 0.3f);
    
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// PHASE 1 : ÉCRAN NOIR (T+0.0s → T+0.5s)
// ═══════════════════════════════════════════════════════════════════════════

void IntroLayer::startPhase1_BlackScreen() {
    // Attendre un court instant dans le noir total pour créer la tension
    this->scheduleOnce(schedule_selector(IntroLayer::startPhase2_Explosion), 0.5f);
}

// ═══════════════════════════════════════════════════════════════════════════
// PHASE 2 : EXPLOSION INTÉRIEUR → EXTÉRIEUR (T+0.5s → T+1.0s)
// ═══════════════════════════════════════════════════════════════════════════

void IntroLayer::startPhase2_Explosion() {
    // ─────────────────────────────────────────────────────────────────
    // SFX : WHOOSH
    // ─────────────────────────────────────────────────────────────────
    FMODAudioEngine::sharedEngine()->playEffect("whoosh.mp3"_spr, 1.0f, 1.0f, 1.0f);
    
    // ─────────────────────────────────────────────────────────────────
    // CERCLE DE LUMIÈRE : Explose du centre vers l'extérieur
    // ─────────────────────────────────────────────────────────────────
    m_explosionCircle->setOpacity(255);
    m_explosionCircle->setScale(0.1f);
    
    m_explosionCircle->runAction(CCSequence::create(
        CCSpawn::create(
            CCEaseExponentialOut::create(CCScaleTo::create(0.35f, 25.0f)),
            CCSequence::create(
                CCFadeTo::create(0.15f, 255),
                CCFadeTo::create(0.20f, 0),
                nullptr
            ),
            nullptr
        ),
        nullptr
    ));
    
    // ─────────────────────────────────────────────────────────────────
    // SCREEN SHAKE : Intense, rapide
    // ─────────────────────────────────────────────────────────────────
    this->shakeScreen(18.0f, 0.35f, 60.0f);
    
    // ─────────────────────────────────────────────────────────────────
    // FLASH BLANC : Court et intense
    // ─────────────────────────────────────────────────────────────────
    m_flashLayer->runAction(CCSequence::create(
        CCFadeTo::create(0.02f, 200),
        CCFadeTo::create(0.15f, 0),
        nullptr
    ));
    
    // ─────────────────────────────────────────────────────────────────
    // PARTICULES D'EXPLOSION : Du centre vers les bords
    // ─────────────────────────────────────────────────────────────────
    this->spawnExplosionParticles();
    
    // ─────────────────────────────────────────────────────────────────
    // TRANSITION VERS PHASE 3
    // ─────────────────────────────────────────────────────────────────
    this->scheduleOnce(schedule_selector(IntroLayer::startPhase3_Retraction), 0.5f);
}

void IntroLayer::spawnExplosionParticles() {
    const int PARTICLE_COUNT = 120;
    const float MAX_RADIUS = 800.0f;
    
    std::vector<ccColor3B> colors = {
        ccc3(255, 200, 80),   // Or
        ccc3(255, 150, 50),   // Orange
        ccc3(255, 100, 100),  // Rouge clair
        ccc3(255, 255, 200),  // Blanc chaud
    };
    
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        auto particle = CCSprite::create("GJ_gradientBG.png");
        particle->setPosition(m_center);
        particle->setScale(0.02f + (rand() % 100) / 2000.0f);
        particle->setOpacity(255);
        particle->setColor(colors[rand() % colors.size()]);
        particle->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
        this->addChild(particle, 2);
        m_explosionParticles.push_back(particle);
        
        // Angle de départ (réparti en cercle avec variation)
        float baseAngle = (360.0f / PARTICLE_COUNT) * i;
        float angleVariation = (rand() % 40 - 20) * 0.1f;
        float angle = CC_DEGREES_TO_RADIANS(baseAngle + angleVariation);
        
        // Distance finale (variable)
        float endRadius = MAX_RADIUS * (0.7f + (rand() % 30) / 100.0f);
        
        // Position finale
        CCPoint endPos = ccp(
            m_center.x + cosf(angle) * endRadius,
            m_center.y + sinf(angle) * endRadius
        );
        
        // Durée variable pour effet naturel
        float duration = 0.4f + (rand() % 100) / 500.0f;
        
        // Animation : explosion vers l'extérieur + fade out
        particle->runAction(CCSpawn::create(
            CCEaseExponentialOut::create(CCMoveTo::create(duration, endPos)),
            CCSequence::create(
                CCDelayTime::create(duration * 0.5f),
                CCFadeOut::create(duration * 0.5f),
                CCRemoveSelf::create(),
                nullptr
            ),
            CCRotateBy::create(duration, (rand() % 720) - 360),
            nullptr
        ));
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// PHASE 3 : RÉTRACTION VERS L'INTÉRIEUR (T+1.0s → T+1.8s)
// ═══════════════════════════════════════════════════════════════════════════

void IntroLayer::startPhase3_Retraction() {
    // ─────────────────────────────────────────────────────────────────
    // GLOW CENTRAL : Apparaît et pulse
    // ─────────────────────────────────────────────────────────────────
    m_glowCenter->setScale(3.0f);
    m_glowCenter->setOpacity(0);
    
    m_glowCenter->runAction(CCSequence::create(
        CCSpawn::create(
            CCFadeTo::create(0.3f, 180),
            CCEaseExponentialIn::create(CCScaleTo::create(0.7f, 0.3f)),
            nullptr
        ),
        CCSpawn::create(
            CCFadeTo::create(0.2f, 255),
            CCScaleTo::create(0.2f, 0.5f),
            nullptr
        ),
        nullptr
    ));
    
    // ─────────────────────────────────────────────────────────────────
    // PARTICULES DE RÉTRACTION : Des bords vers le centre
    // ─────────────────────────────────────────────────────────────────
    this->spawnRetractionParticles();
    
    // ─────────────────────────────────────────────────────────────────
    // TRANSITION VERS PHASE 4
    // ─────────────────────────────────────────────────────────────────
    this->scheduleOnce(schedule_selector(IntroLayer::startPhase4_TextReveal), 0.8f);
}

void IntroLayer::spawnRetractionParticles() {
    const int PARTICLE_COUNT = 80;
    const float START_RADIUS = 600.0f;
    
    std::vector<ccColor3B> colors = {
        ccc3(255, 180, 60),   // Or
        ccc3(255, 140, 40),   // Orange
        ccc3(255, 220, 120),  // Jaune clair
        ccc3(255, 100, 80),   // Rouge-orange
    };
    
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        auto particle = CCSprite::create("GJ_gradientBG.png");
        
        // Position de départ : aux bords, en cercle
        float angle = CC_DEGREES_TO_RADIANS((360.0f / PARTICLE_COUNT) * i);
        float startRadius = START_RADIUS * (0.8f + (rand() % 40) / 100.0f);
        
        CCPoint startPos = ccp(
            m_center.x + cosf(angle) * startRadius,
            m_center.y + sinf(angle) * startRadius
        );
        
        particle->setPosition(startPos);
        particle->setScale(0.015f + (rand() % 100) / 3000.0f);
        particle->setOpacity(0);
        particle->setColor(colors[rand() % colors.size()]);
        particle->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
        this->addChild(particle, 3);
        m_retractionParticles.push_back(particle);
        
        // Délai échelonné pour effet de vague
        float delay = (rand() % 100) / 400.0f;
        float duration = 0.5f + (rand() % 100) / 400.0f;
        
        // Animation : fade in, mouvement vers centre, fade out à la fin
        particle->runAction(CCSequence::create(
            CCDelayTime::create(delay),
            CCSpawn::create(
                CCFadeIn::create(0.1f),
                CCEaseExponentialIn::create(CCMoveTo::create(duration, m_center)),
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
}

// ═══════════════════════════════════════════════════════════════════════════
// PHASE 4 : RÉVÉLATION DU TEXTE "STARGRIND" (T+1.8s → T+3.2s)
// ═══════════════════════════════════════════════════════════════════════════

void IntroLayer::startPhase4_TextReveal() {
    // ─────────────────────────────────────────────────────────────────
    // FLASH DOUX AU MOMENT DE L'APPARITION
    // ─────────────────────────────────────────────────────────────────
    m_flashLayer->runAction(CCSequence::create(
        CCFadeTo::create(0.05f, 120),
        CCFadeTo::create(0.3f, 0),
        nullptr
    ));
    
    // ─────────────────────────────────────────────────────────────────
    // FADE OUT DU GLOW CENTRAL
    // ─────────────────────────────────────────────────────────────────
    m_glowCenter->runAction(CCSequence::create(
        CCDelayTime::create(0.3f),
        CCFadeTo::create(0.5f, 0),
        nullptr
    ));
    
    // ─────────────────────────────────────────────────────────────────
    // LETTRES : Fade in + Animation du letter-spacing
    // ─────────────────────────────────────────────────────────────────
    for (size_t i = 0; i < m_titleLetters.size(); i++) {
        auto letter = m_titleLetters[i];
        
        // Fade in progressif
        letter->runAction(CCSequence::create(
            CCDelayTime::create(i * 0.03f),
            CCFadeIn::create(0.4f),
            nullptr
        ));
    }
    
    // Démarrer l'animation du letter-spacing
    m_letterSpacingProgress = 0.0f;
    m_isAnimatingLetters = true;
    
    // ─────────────────────────────────────────────────────────────────
    // SOUS-TITRE : Apparaît après le titre
    // ─────────────────────────────────────────────────────────────────
    m_subtitleLabel->runAction(CCSequence::create(
        CCDelayTime::create(1.0f),
        CCFadeIn::create(0.5f),
        nullptr
    ));
    
    // ─────────────────────────────────────────────────────────────────
    // TRANSITION VERS PHASE 5
    // ─────────────────────────────────────────────────────────────────
    this->scheduleOnce(schedule_selector(IntroLayer::startPhase5_Stabilization), 1.5f);
}

void IntroLayer::animateLetterSpacing() {
    if (!m_isAnimatingLetters) return;
    
    // Calcul de l'espacement actuel avec easing
    float easedProgress = Easing::easeOutCubic(m_letterSpacingProgress);
    float currentSpacing = m_targetLetterSpacing * easedProgress;
    
    // Positionner chaque lettre
    float totalWidth = (m_titleLetters.size() - 1) * currentSpacing;
    float startX = m_center.x - totalWidth / 2.0f;
    
    for (size_t i = 0; i < m_titleLetters.size(); i++) {
        auto letter = m_titleLetters[i];
        float targetX = startX + i * currentSpacing;
        letter->setPositionX(targetX);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// PHASE 5 : STABILISATION (T+3.2s+)
// ═══════════════════════════════════════════════════════════════════════════

void IntroLayer::startPhase5_Stabilization() {
    m_isAnimatingLetters = false;
    
    // ─────────────────────────────────────────────────────────────────
    // PULSE LÉGER SUR LES LETTRES
    // ─────────────────────────────────────────────────────────────────
    for (size_t i = 0; i < m_titleLetters.size(); i++) {
        auto letter = m_titleLetters[i];
        float delay = i * 0.05f;
        
        // Cycle de couleurs RGB
        letter->runAction(CCSequence::create(
            CCDelayTime::create(delay),
            CCRepeatForever::create(CCSequence::create(
                CCTintTo::create(0.5f, 255, 215, 0),   // Or
                CCTintTo::create(0.5f, 255, 150, 50),  // Orange
                CCTintTo::create(0.5f, 255, 100, 150), // Rose
                CCTintTo::create(0.5f, 150, 100, 255), // Violet
                CCTintTo::create(0.5f, 100, 200, 255), // Cyan
                CCTintTo::create(0.5f, 255, 215, 0),   // Retour or
                nullptr
            )),
            nullptr
        ));
        
        // Pulse scale léger
        letter->runAction(CCRepeatForever::create(CCSequence::create(
            CCEaseInOut::create(CCScaleTo::create(1.5f, 1.05f), 2.0f),
            CCEaseInOut::create(CCScaleTo::create(1.5f, 1.0f), 2.0f),
            nullptr
        )));
    }
    
    // ─────────────────────────────────────────────────────────────────
    // SOUS-TITRE : Pulse léger
    // ─────────────────────────────────────────────────────────────────
    m_subtitleLabel->runAction(CCRepeatForever::create(CCSequence::create(
        CCFadeTo::create(1.5f, 180),
        CCFadeTo::create(1.5f, 255),
        nullptr
    )));
    
    // ─────────────────────────────────────────────────────────────────
    // BOUTON ENTER : Apparaît avec bounce
    // ─────────────────────────────────────────────────────────────────
    m_enterButton->setVisible(true);
    m_enterButton->setOpacity(0);
    m_enterButton->setScale(0.5f);
    
    m_enterButton->runAction(CCSpawn::create(
        CCFadeIn::create(0.5f),
        CCEaseBackOut::create(CCScaleTo::create(0.5f, 1.0f)),
        nullptr
    ));
    
    // Glow pulsant sur le bouton
    m_enterButton->runAction(CCRepeatForever::create(CCSequence::create(
        CCEaseInOut::create(CCScaleTo::create(1.0f, 1.1f), 2.0f),
        CCEaseInOut::create(CCScaleTo::create(1.0f, 1.0f), 2.0f),
        nullptr
    )));
}

// ═══════════════════════════════════════════════════════════════════════════
// SCREEN SHAKE
// ═══════════════════════════════════════════════════════════════════════════

void IntroLayer::shakeScreen(float intensity, float duration, float frequency) {
    CCPoint originalPos = this->getPosition();
    int shakeCount = (int)(duration * frequency);
    
    CCArray* actions = CCArray::create();
    
    for (int i = 0; i < shakeCount; i++) {
        // Intensité décroissante
        float factor = 1.0f - ((float)i / shakeCount);
        factor = factor * factor; // Courbe quadratique pour un decay plus naturel
        
        float offsetX = ((rand() % 200) - 100) / 100.0f * intensity * factor;
        float offsetY = ((rand() % 200) - 100) / 100.0f * intensity * factor;
        
        actions->addObject(CCMoveTo::create(1.0f / frequency, 
            ccp(originalPos.x + offsetX, originalPos.y + offsetY)));
    }
    
    // Retour à la position originale
    actions->addObject(CCEaseOut::create(CCMoveTo::create(0.1f, originalPos), 2.0f));
    
    this->runAction(CCSequence::create(actions));
}

// ═══════════════════════════════════════════════════════════════════════════
// UPDATE LOOP
// ═══════════════════════════════════════════════════════════════════════════

void IntroLayer::update(float dt) {
    // Animation du letter-spacing
    if (m_isAnimatingLetters) {
        m_letterSpacingProgress += dt / 1.2f; // 1.2 secondes pour l'animation complète
        m_letterSpacingProgress = std::min(m_letterSpacingProgress, 1.0f);
        
        this->animateLetterSpacing();
        
        if (m_letterSpacingProgress >= 1.0f) {
            m_isAnimatingLetters = false;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// CALLBACKS
// ═══════════════════════════════════════════════════════════════════════════

void IntroLayer::onEnterClicked(CCObject* sender) {
    FMODAudioEngine::sharedEngine()->playEffect("click.mp3"_spr);
    
    // Flash de sortie
    m_flashLayer->runAction(CCSequence::create(
        CCFadeTo::create(0.3f, 255),
        CCCallFunc::create(this, callfunc_selector(IntroLayer::onIntroComplete)),
        nullptr
    ));
}

void IntroLayer::onIntroComplete() {
    auto scene = LobbyLayer::scene();
    CCDirector::sharedDirector()->replaceScene(
        CCTransitionFade::create(0.5f, scene, ccc3(255, 255, 255))
    );
}