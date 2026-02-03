#include "AnimatedBackground.hpp"

AnimatedBackground* AnimatedBackground::create() {
    auto ret = new AnimatedBackground();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool AnimatedBackground::init() {
    if (!CCLayerColor::initWithColor(ccc4(20, 10, 40, 255))) {
        return false;
    }
    
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    // Gradient overlay
    m_gradientOverlay = CCSprite::create("GJ_gradientBG.png");
    m_gradientOverlay->setPosition(winSize / 2);
    m_gradientOverlay->setScaleX(winSize.width / m_gradientOverlay->getContentSize().width * 1.5f);
    m_gradientOverlay->setScaleY(winSize.height / m_gradientOverlay->getContentSize().height * 1.5f);
    m_gradientOverlay->setOpacity(60);
    m_gradientOverlay->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
    this->addChild(m_gradientOverlay, 1);
    
    // Rotation lente
    m_gradientOverlay->runAction(CCRepeatForever::create(
        CCRotateBy::create(30.0f, 360.0f)
    ));
    
    // Particules ambiantes
    m_particles = ParticleSystem::createAmbientGlow();
    m_particles->setPosition(winSize / 2);
    this->addChild(m_particles, 2);
    
    // Orbes flottants
    createOrbs(6);
    
    this->scheduleUpdate();
    
    return true;
}

void AnimatedBackground::createOrbs(int count) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    for (int i = 0; i < count; i++) {
        auto orb = CCSprite::create("GJ_gradientBG.png");
        
        float x = rand() % static_cast<int>(winSize.width);
        float y = rand() % static_cast<int>(winSize.height);
        orb->setPosition(ccp(x, y));
        
        float scale = 0.3f + (rand() % 50) / 100.0f;
        orb->setScale(scale);
        
        // Couleur aléatoire violet/bleu
        orb->setColor(ccc3(
            100 + rand() % 80,
            50 + rand() % 80,
            150 + rand() % 105
        ));
        
        orb->setOpacity(25 + rand() % 40);
        orb->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
        
        this->addChild(orb, 0);
        m_orbs.push_back(orb);
        
        // Mouvement flottant
        float duration = 6.0f + (rand() % 40) / 10.0f;
        float dx = -60 + rand() % 120;
        float dy = -60 + rand() % 120;
        
        orb->runAction(CCRepeatForever::create(CCSequence::create(
            CCMoveBy::create(duration, ccp(dx, dy)),
            CCMoveBy::create(duration, ccp(-dx, -dy)),
            nullptr
        )));
        
        // Pulsation
        orb->runAction(CCRepeatForever::create(CCSequence::create(
            CCFadeTo::create(2.0f + rand() % 20 / 10.0f, 50 + rand() % 30),
            CCFadeTo::create(2.0f + rand() % 20 / 10.0f, 20 + rand() % 20),
            nullptr
        )));
    }
}

void AnimatedBackground::update(float dt) {
    m_hue += m_speed * dt;
    if (m_hue > 1.0f) m_hue -= 1.0f;
    
    ccColor3B color = hsvToRgb(m_hue, m_saturation, m_brightness);
    this->setColor(color);
    
    // Couleur complémentaire pour l'overlay
    float compHue = m_hue + 0.5f;
    if (compHue > 1.0f) compHue -= 1.0f;
    ccColor3B overlayColor = hsvToRgb(compHue, m_saturation * 0.7f, m_brightness * 1.3f);
    m_gradientOverlay->setColor(overlayColor);
}

ccColor3B AnimatedBackground::hsvToRgb(float h, float s, float v) {
    float r, g, b;
    
    int i = static_cast<int>(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);
    
    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    
    return ccc3(
        static_cast<GLubyte>(r * 255),
        static_cast<GLubyte>(g * 255),
        static_cast<GLubyte>(b * 255)
    );
}