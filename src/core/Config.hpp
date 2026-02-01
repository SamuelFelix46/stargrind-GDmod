#pragma once
#include <Geode/Geode.hpp>
#include <string>
using namespace geode::prelude;
namespace Stargrind {
// ============================================
// CONFIGURATION GLOBALE
// ============================================
namespace Config {
    // API Backend
    inline const std::string API_BASE = "https://mvxirroptplzjxsfyamz.supabase.co/functions/v1";
    inline const std::string SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im12eGlycm9wdHBsemp4c2Z5YW16Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjkxODc5ODMsImV4cCI6MjA4NDc2Mzk4M30.RiQ1l0fiuIN-8WWHgxtlQfmBoVyYUNAtVIzZ2eUl9SM";
    
    // Matchmaking
    inline constexpr int PLAYERS_PER_MATCH = 10;
    inline constexpr int PLAYERS_PER_TEAM = 5;
    inline constexpr float HEARTBEAT_INTERVAL = 5.0f;     // Secondes
    inline constexpr float SCORE_SYNC_INTERVAL = 30.0f;   // Secondes
    inline constexpr float LEADERBOARD_CACHE_TIME = 172800.0f; // 48h en secondes
    
    // Match
    inline constexpr float MATCH_DURATION = 1800.0f;      // 30 minutes
    inline constexpr float VOTE_TIMEOUT = 30.0f;          // Temps pour voter
    inline constexpr float VERSUS_SCREEN_DURATION = 3.0f; // Durée écran versus
    inline constexpr int LEVELS_TO_SELECT = 3;
    
    // XP Rewards
    inline constexpr int XP_WIN_BASE = 100;
    inline constexpr int XP_MVP_BONUS = 30;
    inline constexpr int XP_TOP2_BONUS = 10;
    inline constexpr int XP_TOP3_BONUS = 2;
    
    // UI Animation
    inline constexpr float RGB_LERP_SPEED = 0.5f;
    inline constexpr float HOVER_SCALE = 1.15f;
    inline constexpr float BUTTON_ANIM_DURATION = 0.15f;
    
    // Links
    inline const std::string COFFEE_URL = "https://buymeacoffee.com/votrenom";
}
// Helpers
namespace Utils {
    inline std::string getPlayerName() {
        return GJAccountManager::sharedState()->m_username;
    }
    
    inline std::string getPlayerId() {
        return std::to_string(GJAccountManager::sharedState()->m_accountID);
    }
    
    inline int getCurrentStars() {
        return GameStatsManager::sharedState()->getStat("6");
    }
    
    inline int calculateLevel(int xp) {
        return static_cast<int>(std::sqrt(xp / 100.0f));
    }
}
