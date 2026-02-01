#pragma once
#include <Geode/Geode.hpp>
#include "../network/API.hpp"
#include <functional>
using namespace geode::prelude;
namespace Stargrind::Logic {
enum class SessionState {
    IDLE,
    IN_QUEUE,
    VERSUS_SCREEN,
    VOTING,
    LOADING_LEVELS,
    PLAYING,
    LAST_TRY,
    SPECTATING,
    RESULTS
};
class GameSession {
public:
    static GameSession* get();
    
    // State Management
    SessionState getState() const { return m_state; }
    void setState(SessionState state);
    
    // Queue
    void joinQueue();
    void leaveQueue();
    void startPolling();
    void stopPolling();
    
    // Match
    void startMatch(const Network::MatchInfo& match);
    void endMatch();
    
    // Gameplay
    void setBaseStars(int stars);
    int getBaseStars() const { return m_baseStars; }
    int getStarsGained() const;
    void syncScore();
    void skipLevel();
    
    // Timer
    void startMatchTimer();
    float getTimeRemaining() const { return m_timeRemaining; }
    bool isTimeUp() const { return m_timeRemaining <= 0; }
    void onLastTryComplete(bool completed);
    
    // Spectating
    void enterSpectatorMode();
    bool isSpectating() const { return m_state == SessionState::SPECTATING; }
    
    // Match Info
    const Network::MatchInfo& getMatchInfo() const { return m_currentMatch; }
    bool isHost() const;
    std::string getMyTeam() const { return m_currentMatch.your_team; }
    
    // Callbacks
    using StateCallback = std::function<void(SessionState)>;
    using MatchCallback = std::function<void(const Network::MatchInfo&)>;
    using TimeCallback = std::function<void(float)>;
    
    void setOnStateChange(StateCallback cb) { m_onStateChange = cb; }
    void setOnMatchFound(MatchCallback cb) { m_onMatchFound = cb; }
    void setOnScoresUpdated(MatchCallback cb) { m_onScoresUpdated = cb; }
    void setOnTimeUpdate(TimeCallback cb) { m_onTimeUpdate = cb; }
    
private:
    GameSession() = default;
    static GameSession* s_instance;
    
    SessionState m_state = SessionState::IDLE;
    Network::MatchInfo m_currentMatch;
    
    int m_baseStars = 0;
    float m_timeRemaining = 0;
    bool m_pollingActive = false;
    bool m_scoreSyncActive = false;
    
    StateCallback m_onStateChange;
    MatchCallback m_onMatchFound;
    MatchCallback m_onScoresUpdated;
    TimeCallback m_onTimeUpdate;
    
    void pollStatus();
    void syncScoreLoop();
    void updateTimer(float dt);
};
} // namespace Stargrind::Logic