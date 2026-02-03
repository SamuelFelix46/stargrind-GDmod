#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ParticleSystem {
public:
    // Champ d'étoiles (intro + fond)
    static CCParticleSystemQuad* createStarfield();
    
    // Étincelles brillantes
    static CCParticleSystemQuad* createSparkles();
    
    // Traînée pour les scores
    static CCParticleSystemQuad* createScoreTrail(ccColor3B color);
    
    // Explosion de victoire
    static CCParticleSystemQuad* createVictoryBurst();
    
    // Particules ambiantes (lobby)
    static CCParticleSystemQuad* createAmbientGlow();
    
    // Confettis (résultats)
    static CCParticleSystemQuad* createConfetti();
};