#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <chrono>
#include <functional>

using namespace geode::prelude;

enum class SessionState {
    NotStarted,
    Countdown,     // 3, 2, 1...
    Playing,
    LastAttempt,   // Timer fini, joueur termine son essai
    Spectating,
    Finished
};

struct TeamScore {
    int teamId;
    int totalStars = 0;
    std::vector<std::string> members;
};

struct PlayerStats {
    std::string odName;
    int starsEarned = 0;
    int levelsCompleted = 0;
    int levelsFailed = 0;
    int skipsUsed = 0;
    int highestProgress = 0; // %
};

class GameSession {
private:
    static GameSession* s_instance;
    
    SessionState m_state = SessionState::NotStarted;
    
    // Timer
    std::chrono::steady_clock::time_point m_startTime;
    int m_durationSeconds = 30 * 60; // 30 minutes
    int m_pausedTimeRemaining = 0;
    bool m_timerPaused = false;
    
    // Équipes
    TeamScore m_team1;
    TeamScore m_team2;
    int m_myTeamId = 0;
    
    // Stats
    PlayerStats m_myStats;
    
    // Callbacks
    std::function<void()> m_onTimeUp;
    std::function<void(SessionState)> m_onStateChange;
    std::function<void(int teamId, int newTotal)> m_onScoreUpdate;
    
public:
    static GameSession* get();
    static void destroy();
    
    // Lifecycle
    void startSession(int myTeamId, 
                      const std::vector<std::string>& team1Members,
                      const std::vector<std::string>& team2Members);
    void endSession();
    void reset();
    
    // Timer
    int getRemainingSeconds() const;
    std::string getFormattedTime() const;
    bool isTimeUp() const;
    void pauseTimer();
    void resumeTimer();
    
    // État
    SessionState getState() const { return m_state; }
    void setState(SessionState newState);
    bool isPlaying() const { 
        return m_state == SessionState::Playing || m_state == SessionState::LastAttempt; 
    }
    
    // Scores
    void addScore(int stars);
    void receiveTeamScore(int teamId, int additionalStars);
    int getMyTeamScore() const;
    int getOpponentTeamScore() const;
    TeamScore getTeam1() const { return m_team1; }
    TeamScore getTeam2() const { return m_team2; }
    int getMyTeamId() const { return m_myTeamId; }
    
    // Actions de jeu
    void onLevelComplete(int starsEarned);
    void onLevelFail();
    void onSkip();
    void onProgress(int percent);
    
    // Stats
    PlayerStats getMyStats() const { return m_myStats; }
    
    // Callbacks
    void setOnTimeUp(std::function<void()> cb) { m_onTimeUp = cb; }
    void setOnStateChange(std::function<void(SessionState)> cb) { m_onStateChange = cb; }
    void setOnScoreUpdate(std::function<void(int, int)> cb) { m_onScoreUpdate = cb; }
    
    // Update (appelé chaque frame depuis le hook)
    void update(float dt);
};