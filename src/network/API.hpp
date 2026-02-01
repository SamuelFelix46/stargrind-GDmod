#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <functional>
#include <string>
#include <vector>
using namespace geode::prelude;
namespace Stargrind::Network {
// ============================================
// STRUCTURES DE DONNÉES
// ============================================
struct PlayerInfo {
    std::string player_name;
    std::string gd_id;
    std::string team;
    int stars_count = 0;
};
struct MatchInfo {
    std::string match_id;
    std::string host_id;
    std::string your_team;
    std::vector<PlayerInfo> team_a;
    std::vector<PlayerInfo> team_b;
    int team_a_total = 0;
    int team_b_total = 0;
    std::string leader;
};
struct LevelInfo {
    int id;
    std::string name;
    int stars;
};
struct LeaderboardEntry {
    int rank;
    std::string player_name;
    std::string gd_id;
    int total_xp;
    int level;
    int matches_played;
    int matches_won;
    int win_rate;
};
struct PlayerStats {
    int rank;
    std::string player_name;
    int total_xp;
    int level;
    int matches_played;
    int matches_won;
    int total_stars;
};
// ============================================
// CALLBACKS
// ============================================
using SuccessCallback = std::function<void(const std::string& message)>;
using ErrorCallback = std::function<void(const std::string& error)>;
using QueueCallback = std::function<void(int queueCount, int playersNeeded)>;
using MatchFoundCallback = std::function<void(const MatchInfo& match)>;
using ScoresCallback = std::function<void(const MatchInfo& scores)>;
using LeaderboardCallback = std::function<void(const std::vector<LeaderboardEntry>& leaderboard, const PlayerStats* localPlayer)>;
using LevelsCallback = std::function<void(const std::vector<LevelInfo>& levels, const std::string& difficulty)>;
using VoteCallback = std::function<void(int currentVotes, int totalPlayers)>;
// ============================================
// CLASSE API
// ============================================
class API {
public:
    static API* get();
    
    // Queue Management
    void joinQueue(QueueCallback onSuccess, ErrorCallback onError);
    void leaveQueue(SuccessCallback onSuccess, ErrorCallback onError);
    void checkStatus(QueueCallback onWaiting, MatchFoundCallback onMatch, ErrorCallback onError);
    
    // Match Operations
    void updateScore(const std::string& matchId, int starsGained, SuccessCallback onSuccess, ErrorCallback onError);
    void getScores(const std::string& matchId, ScoresCallback onSuccess, ErrorCallback onError);
    void endMatch(const std::string& matchId, SuccessCallback onSuccess, ErrorCallback onError);
    
    // Voting
    void voteDifficulty(const std::string& matchId, int difficulty, VoteCallback onSuccess, ErrorCallback onError);
    void getLevels(const std::string& matchId, LevelsCallback onSuccess, ErrorCallback onError);
    
    // Leaderboard
    void getLeaderboard(LeaderboardCallback onSuccess, ErrorCallback onError);
    
    // AFK Management
    void kickAfk(const std::string& matchId, const std::string& gdId, const std::string& reason, SuccessCallback onSuccess, ErrorCallback onError);
    
private:
    API() = default;
    static API* s_instance;
    
    web::WebRequest createRequest();
    matjson::Value parseResponse(const std::string& response);
};
