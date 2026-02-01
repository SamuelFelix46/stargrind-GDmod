#include "API.hpp"
#include "../core/Config.hpp"
namespace Stargrind::Network {
API* API::s_instance = nullptr;
API* API::get() {
    if (!s_instance) {
        s_instance = new API();
    }
    return s_instance;
}
web::WebRequest API::createRequest() {
    web::WebRequest req;
    req.header("Content-Type", "application/json");
    req.header("apikey", Config::SUPABASE_KEY);
    req.header("Authorization", "Bearer " + Config::SUPABASE_KEY);
    return req;
}
matjson::Value API::parseResponse(const std::string& response) {
    std::string error;
    auto result = matjson::parse(response, error);
    if (!result) {
        log::error("JSON parse error: {}", error);
        return matjson::Value();
    }
    return result.value();
}
void API::joinQueue(QueueCallback onSuccess, ErrorCallback onError) {
    auto req = createRequest();
    
    matjson::Value body;
    body["player_name"] = Utils::getPlayerName();
    body["gd_id"] = Utils::getPlayerId();
    req.bodyJSON(body);
    
    auto listener = new EventListener<web::WebTask>;
    listener->bind([this, onSuccess, onError, listener](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                auto json = parseResponse(res->string().unwrapOr("{}"));
                if (json.contains("success")) {
                    int queueCount = json["queue_count"].asInt().unwrapOr(1);
                    int playersNeeded = json["players_needed"].asInt().unwrapOr(9);
                    onSuccess(queueCount, playersNeeded);
                } else {
                    onError(json["error"].asString().unwrapOr("Unknown error"));
                }
            } else {
                onError("Network error: " + std::to_string(res->code()));
            }
        }
        delete listener;
    });
    
    listener->setFilter(req.post(Config::API_BASE + "/join"));
}
void API::leaveQueue(SuccessCallback onSuccess, ErrorCallback onError) {
    auto req = createRequest();
    
    matjson::Value body;
    body["gd_id"] = Utils::getPlayerId();
    req.bodyJSON(body);
    
    auto listener = new EventListener<web::WebTask>;
    listener->bind([onSuccess, onError, listener](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                onSuccess("Left queue");
            } else {
                onError("Failed to leave queue");
            }
        }
        delete listener;
    });
    
    listener->setFilter(req.post(Config::API_BASE + "/leave-queue"));
}
void API::checkStatus(QueueCallback onWaiting, MatchFoundCallback onMatch, ErrorCallback onError) {
    auto req = createRequest();
    std::string url = Config::API_BASE + "/status?gd_id=" + Utils::getPlayerId();
    
    auto listener = new EventListener<web::WebTask>;
    listener->bind([this, onWaiting, onMatch, onError, listener](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                auto json = parseResponse(res->string().unwrapOr("{}"));
                std::string status = json["status"].asString().unwrapOr("WAITING");
                
                if (status == "READY") {
                    MatchInfo match;
                    match.match_id = json["match_id"].asString().unwrapOr("");
                    match.host_id = json["host_id"].asString().unwrapOr("");
                    match.your_team = json["your_team"].asString().unwrapOr("A");
                    
                    // Parse teams
                    if (json.contains("team_a")) {
                        for (const auto& p : json["team_a"].asArray().unwrap()) {
                            PlayerInfo player;
                            player.player_name = p["player_name"].asString().unwrapOr("");
                            player.gd_id = p["gd_id"].asString().unwrapOr("");
                            player.team = "A";
                            match.team_a.push_back(player);
                        }
                    }
                    
                    if (json.contains("team_b")) {
                        for (const auto& p : json["team_b"].asArray().unwrap()) {
                            PlayerInfo player;
                            player.player_name = p["player_name"].asString().unwrapOr("");
                            player.gd_id = p["gd_id"].asString().unwrapOr("");
                            player.team = "B";
                            match.team_b.push_back(player);
                        }
                    }
                    
                    onMatch(match);
                } else {
                    int queueCount = json["queue_count"].asInt().unwrapOr(1);
                    onWaiting(queueCount, 10 - queueCount);
                }
            } else {
                onError("Status check failed");
            }
        }
        delete listener;
    });
    
    listener->setFilter(req.get(url));
}
void API::updateScore(const std::string& matchId, int starsGained, SuccessCallback onSuccess, ErrorCallback onError) {
    auto req = createRequest();
    
    matjson::Value body;
    body["match_id"] = matchId;
    body["gd_id"] = Utils::getPlayerId();
    body["stars_count"] = starsGained;
    req.bodyJSON(body);
    
    auto listener = new EventListener<web::WebTask>;
    listener->bind([onSuccess, onError, listener](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                onSuccess("Score updated");
            } else {
                onError("Score update failed");
            }
        }
        delete listener;
    });
    
    listener->setFilter(req.post(Config::API_BASE + "/update-score"));
}
void API::getScores(const std::string& matchId, ScoresCallback onSuccess, ErrorCallback onError) {
    auto req = createRequest();
    std::string url = Config::API_BASE + "/get-scores?match_id=" + matchId;
    
    auto listener = new EventListener<web::WebTask>;
    listener->bind([this, onSuccess, onError, listener](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                auto json = parseResponse(res->string().unwrapOr("{}"));
                
                MatchInfo scores;
                if (json.contains("team_a")) {
                    scores.team_a_total = json["team_a"]["total_stars"].asInt().unwrapOr(0);
                    for (const auto& p : json["team_a"]["players"].asArray().unwrap()) {
                        PlayerInfo player;
                        player.player_name = p["player_name"].asString().unwrapOr("");
                        player.stars_count = p["stars_count"].asInt().unwrapOr(0);
                        player.team = "A";
                        scores.team_a.push_back(player);
                    }
                }
                
                if (json.contains("team_b")) {
                    scores.team_b_total = json["team_b"]["total_stars"].asInt().unwrapOr(0);
                    for (const auto& p : json["team_b"]["players"].asArray().unwrap()) {
                        PlayerInfo player;
                        player.player_name = p["player_name"].asString().unwrapOr("");
                        player.stars_count = p["stars_count"].asInt().unwrapOr(0);
                        player.team = "B";
                        scores.team_b.push_back(player);
                    }
                }
                
                scores.leader = json["leader"].asString().unwrapOr("TIE");
                onSuccess(scores);
            } else {
                onError("Failed to get scores");
            }
        }
        delete listener;
    });
    
    listener->setFilter(req.get(url));
}
void API::endMatch(const std::string& matchId, SuccessCallback onSuccess, ErrorCallback onError) {
    auto req = createRequest();
    
    matjson::Value body;
    body["match_id"] = matchId;
    req.bodyJSON(body);
    
    auto listener = new EventListener<web::WebTask>;
    listener->bind([this, onSuccess, onError, listener](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                auto json = parseResponse(res->string().unwrapOr("{}"));
                std::string winner = json["winning_team"].asString().unwrapOr("TIE");
                onSuccess("Winner: Team " + winner);
            } else {
                onError("Failed to end match");
            }
        }
        delete listener;
    });
    
    listener->setFilter(req.post(Config::API_BASE + "/end-match"));
}
void API::voteDifficulty(const std::string& matchId, int difficulty, VoteCallback onSuccess, ErrorCallback onError) {
    auto req = createRequest();
    
    matjson::Value body;
    body["match_id"] = matchId;
    body["gd_id"] = Utils::getPlayerId();
    body["difficulty"] = difficulty;
    req.bodyJSON(body);
    
    auto listener = new EventListener<web::WebTask>;
    listener->bind([this, onSuccess, onError, listener](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                auto json = parseResponse(res->string().unwrapOr("{}"));
                int currentVotes = json["current_votes"].asInt().unwrapOr(1);
                int totalPlayers = json["total_players"].asInt().unwrapOr(10);
                onSuccess(currentVotes, totalPlayers);
            } else {
                onError("Vote failed");
            }
        }
        delete listener;
    });
    
    listener->setFilter(req.post(Config::API_BASE + "/vote-difficulty"));
}
void API::getLevels(const std::string& matchId, LevelsCallback onSuccess, ErrorCallback onError) {
    auto req = createRequest();
    std::string url = Config::API_BASE + "/get-match-levels?match_id=" + matchId;
    
    auto listener = new EventListener<web::WebTask>;
    listener->bind([this, onSuccess, onError, listener](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                auto json = parseResponse(res->string().unwrapOr("{}"));
                
                std::vector<LevelInfo> levels;
                if (json.contains("levels")) {
                    for (const auto& l : json["levels"].asArray().unwrap()) {
                        LevelInfo level;
                        level.id = l["id"].asInt().unwrapOr(0);
                        level.name = l["name"].asString().unwrapOr("");
                        level.stars = l["stars"].asInt().unwrapOr(0);
                        levels.push_back(level);
                    }
                }
                
                std::string difficulty = json["difficulty_category"].asString().unwrapOr("hard");
                onSuccess(levels, difficulty);
            } else {
                onError("Failed to get levels");
            }
        }
        delete listener;
    });
    
    listener->setFilter(req.get(url));
}
void API::getLeaderboard(LeaderboardCallback onSuccess, ErrorCallback onError) {
    auto req = createRequest();
    std::string url = Config::API_BASE + "/get-leaderboard?gd_id=" + Utils::getPlayerId();
    
    auto listener = new EventListener<web::WebTask>;
    listener->bind([this, onSuccess, onError, listener](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                auto json = parseResponse(res->string().unwrapOr("{}"));
                
                std::vector<LeaderboardEntry> leaderboard;
                if (json.contains("leaderboard")) {
                    for (const auto& p : json["leaderboard"].asArray().unwrap()) {
                        LeaderboardEntry entry;
                        entry.rank = p["rank"].asInt().unwrapOr(0);
                        entry.player_name = p["player_name"].asString().unwrapOr("");
                        entry.gd_id = p["gd_id"].asString().unwrapOr("");
                        entry.total_xp = p["total_xp"].asInt().unwrapOr(0);
                        entry.level = p["level"].asInt().unwrapOr(0);
                        entry.matches_played = p["matches_played"].asInt().unwrapOr(0);
                        entry.matches_won = p["matches_won"].asInt().unwrapOr(0);
                        entry.win_rate = p["win_rate"].asInt().unwrapOr(0);
                        leaderboard.push_back(entry);
                    }
                }
                
                PlayerStats* localPlayer = nullptr;
                if (json.contains("local_player")) {
                    localPlayer = new PlayerStats();
                    auto lp = json["local_player"];
                    localPlayer->rank = lp["rank"].asInt().unwrapOr(0);
                    localPlayer->player_name = lp["player_name"].asString().unwrapOr("");
                    localPlayer->total_xp = lp["total_xp"].asInt().unwrapOr(0);
                    localPlayer->level = lp["level"].asInt().unwrapOr(0);
                    localPlayer->matches_played = lp["matches_played"].asInt().unwrapOr(0);
                    localPlayer->matches_won = lp["matches_won"].asInt().unwrapOr(0);
                }
                
                onSuccess(leaderboard, localPlayer);
                if (localPlayer) delete localPlayer;
            } else {
                onError("Failed to get leaderboard");
            }
        }
        delete listener;
    });
    
    listener->setFilter(req.get(url));
}
void API::kickAfk(const std::string& matchId, const std::string& gdId, const std::string& reason, SuccessCallback onSuccess, ErrorCallback onError) {
    auto req = createRequest();
    
    matjson::Value body;
    body["match_id"] = matchId;
    body["gd_id"] = gdId;
    body["reason"] = reason;
    req.bodyJSON(body);
    
    auto listener = new EventListener<web::WebTask>;
    listener->bind([onSuccess, onError, listener](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                onSuccess("Player kicked");
            } else {
                onError("Failed to kick player");
            }
        }
        delete listener;
    });
    
    listener->setFilter(req.post(Config::API_BASE + "/kick-afk"));
}
} // namespace Stargrind::Network