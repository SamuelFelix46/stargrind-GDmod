#pragma once
#include <Geode/Geode.hpp>
#include <functional>
using namespace geode::prelude;
namespace Stargrind::UI {
// ============================================
// TYPES DE TRANSITIONS
// ============================================
enum class TransitionType {
    FADE,
    SLIDE_LEFT,
    SLIDE_RIGHT,
    SLIDE_UP,
    SLIDE_DOWN,
    SCALE,
    ZOOM_IN,
    ZOOM_OUT,
    FLIP_HORIZONTAL,
    FLIP_VERTICAL
};
// ============================================
// CLASSE ANIMATED TRANSITION
// ============================================
class AnimatedTransition {
public:
    using CompletionCallback = std::function<void()>;
    
    // Transition entre deux layers
    static void transition(
        CCNode* fromLayer,
        CCNode* toLayer,
        TransitionType type = TransitionType::FADE,
        float duration = 0.3f,
        CompletionCallback onComplete = nullptr
    );
    
    // Animer l'entrée d'un layer
    static void animateIn(
        CCNode* layer,
        TransitionType type = TransitionType::FADE,
        float duration = 0.3f,
        CompletionCallback onComplete = nullptr
    );
    
    // Animer la sortie d'un layer
    static void animateOut(
        CCNode* layer,
        TransitionType type = TransitionType::FADE,
        float duration = 0.3f,
        CompletionCallback onComplete = nullptr
    );
    
    // Effet de shake (pour les erreurs, impacts, etc.)
    static void shake(
        CCNode* node,
        float intensity = 5.0f,
        float duration = 0.3f,
        CompletionCallback onComplete = nullptr
    );
    
    // Effet de pulse
    static void pulse(
        CCNode* node,
        float scaleMultiplier = 1.2f,
        float duration = 0.2f
    );
    
    // Effet de bounce
    static void bounce(
        CCNode* node,
        float height = 20.0f,
        float duration = 0.4f
    );
    
    // Flash (pour les notifications, succès, etc.)
    static CCNode* createFlashOverlay(
        CCNode* parent,
        const ccColor4B& color = {255, 255, 255, 200},
        float duration = 0.3f
    );
    
private:
    static CCFiniteTimeAction* createInAction(TransitionType type, float duration, const CCSize& size);
    static CCFiniteTimeAction* createOutAction(TransitionType type, float duration, const CCSize& size);
};
// ============================================
// IMPLÉMENTATION INLINE
// ============================================
inline void AnimatedTransition::transition(
    CCNode* fromLayer,
    CCNode* toLayer,
    TransitionType type,
    float duration,
    CompletionCallback onComplete
) {
    if (!fromLayer || !toLayer) return;
    
    auto parent = fromLayer->getParent();
    if (!parent) return;
    
    // Préparer le nouveau layer
    toLayer->setVisible(false);
    parent->addChild(toLayer, fromLayer->getZOrder() + 1);
    
    // Animer la sortie de l'ancien
    animateOut(fromLayer, type, duration / 2, [fromLayer, toLayer, type, duration, onComplete]() {
        fromLayer->removeFromParent();
        toLayer->setVisible(true);
        
        // Animer l'entrée du nouveau
        animateIn(toLayer, type, duration / 2, onComplete);
    });
}
inline void AnimatedTransition::animateIn(
    CCNode* layer,
    TransitionType type,
    float duration,
    CompletionCallback onComplete
) {
    if (!layer) return;
    
    auto size = CCDirector::sharedDirector()->getWinSize();
    auto action = createInAction(type, duration, size);
    
    if (onComplete) {
        layer->runAction(CCSequence::create(
            action,
            CCCallFunc::create([onComplete]() { onComplete(); }),
            nullptr
        ));
    } else {
        layer->runAction(action);
    }
}
inline void AnimatedTransition::animateOut(
    CCNode* layer,
    TransitionType type,
    float duration,
    CompletionCallback onComplete
) {
    if (!layer) return;
    
    auto size = CCDirector::sharedDirector()->getWinSize();
    auto action = createOutAction(type, duration, size);
    
    if (onComplete) {
        layer->runAction(CCSequence::create(
            action,
            CCCallFunc::create([onComplete]() { onComplete(); }),
            nullptr
        ));
    } else {
        layer->runAction(action);
    }
}
inline void AnimatedTransition::shake(
    CCNode* node,
    float intensity,
    float duration,
    CompletionCallback onComplete
) {
    if (!node) return;
    
    auto originalPos = node->getPosition();
    int shakeCount = static_cast<int>(duration / 0.05f);
    
    CCArray* actions = CCArray::create();
    
    for (int i = 0; i < shakeCount; i++) {
        float factor = 1.0f - (static_cast<float>(i) / shakeCount);
        float offsetX = (rand() % 100 - 50) / 50.0f * intensity * factor;
        float offsetY = (rand() % 100 - 50) / 50.0f * intensity * factor;
        
        actions->addObject(CCMoveTo::create(0.05f, originalPos + ccp(offsetX, offsetY)));
    }
    
    actions->addObject(CCMoveTo::create(0.05f, originalPos));
    
    if (onComplete) {
        actions->addObject(CCCallFunc::create([onComplete]() { onComplete(); }));
    }
    
    node->runAction(CCSequence::create(actions));
}
inline void AnimatedTransition::pulse(
    CCNode* node,
    float scaleMultiplier,
    float duration
) {
    if (!node) return;
    
    float originalScale = node->getScale();
    
    node->runAction(CCSequence::create(
        CCEaseSineOut::create(CCScaleTo::create(duration / 2, originalScale * scaleMultiplier)),
        CCEaseSineIn::create(CCScaleTo::create(duration / 2, originalScale)),
        nullptr
    ));
}
inline void AnimatedTransition::bounce(
    CCNode* node,
    float height,
    float duration
) {
    if (!node) return;
    
    auto originalPos = node->getPosition();
    
    node->runAction(CCSequence::create(
        CCEaseOut::create(CCMoveBy::create(duration / 2, {0, height}), 2.0f),
        CCEaseBounceOut::create(CCMoveTo::create(duration / 2, originalPos)),
        nullptr
    ));
}
inline CCNode* AnimatedTransition::createFlashOverlay(
    CCNode* parent,
    const ccColor4B& color,
    float duration
) {
    if (!parent) return nullptr;
    
    auto flash = CCLayerColor::create(color);
    flash->setContentSize(parent->getContentSize());
    flash->setOpacity(0);
    parent->addChild(flash, 1000);
    
    flash->runAction(CCSequence::create(
        CCFadeTo::create(duration / 3, color.a),
        CCFadeTo::create(duration * 2 / 3, 0),
        CCRemoveSelf::create(),
        nullptr
    ));
    
    return flash;
}
inline CCFiniteTimeAction* AnimatedTransition::createInAction(
    TransitionType type,
    float duration,
    const CCSize& size
) {
    switch (type) {
        case TransitionType::FADE:
            return CCFadeIn::create(duration);
            
        case TransitionType::SLIDE_LEFT:
            return CCSequence::create(
                CCPlace::create({size.width, 0}),
                CCEaseOut::create(CCMoveBy::create(duration, {-size.width, 0}), 2.0f),
                nullptr
            );
            
        case TransitionType::SLIDE_RIGHT:
            return CCSequence::create(
                CCPlace::create({-size.width, 0}),
                CCEaseOut::create(CCMoveBy::create(duration, {size.width, 0}), 2.0f),
                nullptr
            );
            
        case TransitionType::SLIDE_UP:
            return CCSequence::create(
                CCPlace::create({0, -size.height}),
                CCEaseOut::create(CCMoveBy::create(duration, {0, size.height}), 2.0f),
                nullptr
            );
            
        case TransitionType::SLIDE_DOWN:
            return CCSequence::create(
                CCPlace::create({0, size.height}),
                CCEaseOut::create(CCMoveBy::create(duration, {0, -size.height}), 2.0f),
                nullptr
            );
            
        case TransitionType::SCALE:
        case TransitionType::ZOOM_IN:
            return CCSequence::create(
                CCScaleTo::create(0, 0),
                CCEaseBackOut::create(CCScaleTo::create(duration, 1.0f)),
                nullptr
            );
            
        case TransitionType::ZOOM_OUT:
            return CCSequence::create(
                CCScaleTo::create(0, 2.0f),
                CCEaseOut::create(CCScaleTo::create(duration, 1.0f), 2.0f),
                nullptr
            );
            
        default:
            return CCFadeIn::create(duration);
    }
}
inline CCFiniteTimeAction* AnimatedTransition::createOutAction(
    TransitionType type,
    float duration,
    const CCSize& size
) {
    switch (type) {
        case TransitionType::FADE:
            return CCFadeOut::create(duration);
            
        case TransitionType::SLIDE_LEFT:
            return CCEaseIn::create(CCMoveBy::create(duration, {-size.width, 0}), 2.0f);
            
        case TransitionType::SLIDE_RIGHT:
            return CCEaseIn::create(CCMoveBy::create(duration, {size.width, 0}), 2.0f);
            
        case TransitionType::SLIDE_UP:
            return CCEaseIn::create(CCMoveBy::create(duration, {0, size.height}), 2.0f);
            
        case TransitionType::SLIDE_DOWN:
            return CCEaseIn::create(CCMoveBy::create(duration, {0, -size.height}), 2.0f);
            
        case TransitionType::SCALE:
        case TransitionType::ZOOM_OUT:
            return CCEaseIn::create(CCScaleTo::create(duration, 0), 2.0f);
            
        case TransitionType::ZOOM_IN:
            return CCEaseIn::create(CCScaleTo::create(duration, 2.0f), 2.0f);
            
        default:
            return CCFadeOut::create(duration);
    }
}
