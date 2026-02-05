#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <algorithm>

using namespace geode::prelude;

struct PlayerResult {
    std::string odID;
    std::string displayName;
    int stars;
    int team; // 0 ou 1
    bool isWinner;
    int mvpRank; // 0 = pas MVP, 1-3 = rang MVP
    int xpGained;
};

class RewardSystem {
public:
    // ═══════════════════════════════════════════════════════════════
    // CONSTANTES XP
    // ═══════════════════════════════════════════════════════════════
    static constexpr int WINNER_TEAM_XP = 100;  // Équipe gagnante
    static constexpr int MVP_1ST_XP = 30;       // Top 1 étoiles (global)
    static constexpr int MVP_2ND_XP = 10;       // Top 2 étoiles
    static constexpr int MVP_3RD_XP = 2;        // Top 3 étoiles
    static constexpr int PARTICIPATION_XP = 5;  // Juste pour avoir joué
    
    // ═══════════════════════════════════════════════════════════════
    // CALCUL DES RÉCOMPENSES
    // ═══════════════════════════════════════════════════════════════
    
    static std::vector<PlayerResult> calculateRewards(
        std::vector<PlayerResult>& players,
        int winningTeam
    ) {
        // Marquer les gagnants
        for (auto& player : players) {
            player.isWinner = (player.team == winningTeam);
        }
        
        // Trier par étoiles (décroissant) pour déterminer les MVP
        std::vector<PlayerResult*> sortedByStars;
        for (auto& p : players) {
            sortedByStars.push_back(&p);
        }
        
        std::sort(sortedByStars.begin(), sortedByStars.end(), 
            [](PlayerResult* a, PlayerResult* b) {
                return a->stars > b->stars;
            }
        );
        
        // Assigner les rangs MVP (top 3 global)
        for (size_t i = 0; i < sortedByStars.size() && i < 3; i++) {
            sortedByStars[i]->mvpRank = i + 1;
        }
        
        // Calculer l'XP pour chaque joueur
        for (auto& player : players) {
            player.xpGained = calculatePlayerXP(player);
        }
        
        return players;
    }
    
    static int calculatePlayerXP(const PlayerResult& player) {
        int xp = PARTICIPATION_XP;
        
        // Bonus équipe gagnante
        if (player.isWinner) {
            xp += WINNER_TEAM_XP;
        }
        
        // Bonus MVP
        switch (player.mvpRank) {
            case 1: xp += MVP_1ST_XP; break;
            case 2: xp += MVP_2ND_XP; break;
            case 3: xp += MVP_3RD_XP; break;
        }
        
        return xp;
    }
    
    // ═══════════════════════════════════════════════════════════════
    // PERSISTENCE
    // ═══════════════════════════════════════════════════════════════
    
    static void savePlayerXP(const std::string& odID, int totalXP) {
        // Sauvegarder via l'API serveur
        // POST /api/player/{odID}/xp
    }
    
    static int getPlayerTotalXP(const std::string& odID) {
        // Récupérer depuis l'API
        // GET /api/player/{odID}
        return 0;
    }
};