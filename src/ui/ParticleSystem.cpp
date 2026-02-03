#include "ParticleSystem.hpp"

CCParticleSystemQuad* ParticleSystem::createStarfield() {
    auto particles = CCParticleSystemQuad::create();
    
    particles->setEmitterMode(kCCParticleModeGravity);
    particles->setTotalParticles(100);
    particles->setDuration(-1);
    
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    particles->setPosVar(ccp(winSize.width / 2, winSize.height / 2));
    
    particles->setLife(3.0f);
    particles->setLifeVar(1.0f);
    
    particles->setStartSize(8.0f);
    particles->setStartSizeVar(4.0f);
    particles->setEndSize(2.0f);
    
    particles->setStartColor(ccc4f(1.0f, 0.9f, 0.7f, 1.0f));
    particles->setStartColorVar(ccc4f(0.1f, 0.1f, 0.2f, 0.0f));
    particles->setEndColor(ccc4f(1.0f, 1.0f, 1.0f, 0.0f));
    
    particles->setGravity(ccp(0, -20));
    particles->setSpeed(50.0f);
    particles->setSpeedVar(30.0f);
    particles->setAngle(90.0f);
    particles->setAngleVar(360.0f);
    
    particles->setBlendAdditive(true);
    particles->setEmissionRate(30.0f);
    
    return particles;
}

CCParticleSystemQuad* ParticleSystem::createSparkles() {
    auto particles = CCParticleSystemQuad::create();
    
    particles->setEmitterMode(kCCParticleModeGravity);
    particles->setTotalParticles(50);
    particles->setDuration(-1);
    
    particles->setPosVar(ccp(150, 150));
    
    particles->setLife(1.0f);
    particles->setLifeVar(0.5f);
    
    particles->setStartSize(12.0f);
    particles->setStartSizeVar(6.0f);
    particles->setEndSize(0.0f);
    
    particles->setStartColor(ccc4f(1.0f, 1.0f, 1.0f, 1.0f));
    particles->setEndColor(ccc4f(1.0f, 0.8f, 0.5f, 0.0f));
    
    particles->setGravity(ccp(0, 0));
    particles->setSpeed(100.0f);
    particles->setSpeedVar(50.0f);
    particles->setAngle(90.0f);
    particles->setAngleVar(360.0f);
    
    particles->setBlendAdditive(true);
    particles->setEmissionRate(25.0f);
    
    return particles;
}

CCParticleSystemQuad* ParticleSystem::createVictoryBurst() {
    auto particles = CCParticleSystemQuad::create();
    
    particles->setEmitterMode(kCCParticleModeGravity);
    particles->setTotalParticles(250);
    particles->setDuration(0.3f);
    
    particles->setLife(1.5f);
    particles->setLifeVar(0.5f);
    
    particles->setStartSize(18.0f);
    particles->setEndSize(6.0f);
    
    particles->setStartColor(ccc4f(1.0f, 0.9f, 0.2f, 1.0f));
    particles->setStartColorVar(ccc4f(0.2f, 0.3f, 0.5f, 0.0f));
    particles->setEndColor(ccc4f(1.0f, 0.5f, 0.2f, 0.0f));
    
    particles->setGravity(ccp(0, -250));
    particles->setSpeed(350.0f);
    particles->setSpeedVar(100.0f);
    particles->setAngle(90.0f);
    particles->setAngleVar(60.0f);
    
    particles->setBlendAdditive(true);
    particles->setEmissionRate(500.0f);
    
    return particles;
}

CCParticleSystemQuad* ParticleSystem::createAmbientGlow() {
    auto particles = CCParticleSystemQuad::create();
    
    particles->setEmitterMode(kCCParticleModeGravity);
    particles->setTotalParticles(40);
    particles->setDuration(-1);
    
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    particles->setPosVar(ccp(winSize.width / 2, winSize.height / 2));
    
    particles->setLife(6.0f);
    particles->setLifeVar(2.0f);
    
    particles->setStartSize(30.0f);
    particles->setEndSize(50.0f);
    
    particles->setStartColor(ccc4f(0.5f, 0.2f, 0.8f, 0.25f));
    particles->setEndColor(ccc4f(0.2f, 0.4f, 0.8f, 0.0f));
    
    particles->setGravity(ccp(0, 15));
    particles->setSpeed(15.0f);
    particles->setAngle(90.0f);
    particles->setAngleVar(180.0f);
    
    particles->setBlendAdditive(true);
    particles->setEmissionRate(4.0f);
    
    return particles;
}

CCParticleSystemQuad* ParticleSystem::createConfetti() {
    auto particles = CCParticleSystemQuad::create();
    
    particles->setEmitterMode(kCCParticleModeGravity);
    particles->setTotalParticles(150);
    particles->setDuration(-1);
    
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    particles->setPosition(ccp(winSize.width / 2, winSize.height + 50));
    particles->setPosVar(ccp(winSize.width / 2, 0));
    
    particles->setLife(4.0f);
    particles->setLifeVar(1.0f);
    
    particles->setStartSize(10.0f);
    particles->setStartSizeVar(5.0f);
    particles->setEndSize(8.0f);
    
    // Couleurs variées
    particles->setStartColor(ccc4f(1.0f, 0.5f, 0.5f, 1.0f));
    particles->setStartColorVar(ccc4f(0.5f, 0.5f, 0.5f, 0.0f));
    particles->setEndColor(ccc4f(0.5f, 0.5f, 1.0f, 0.8f));
    
    particles->setGravity(ccp(0, -100));
    particles->setSpeed(50.0f);
    particles->setSpeedVar(30.0f);
    particles->setAngle(-90.0f);
    particles->setAngleVar(30.0f);
    
    particles->setEmissionRate(30.0f);
    
    return particles;
}