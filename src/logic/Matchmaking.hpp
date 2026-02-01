#pragma once
#include <Geode/Geode.hpp>
#include "../network/API.hpp"
#include <functional>
using namespace geode::prelude;
namespace Stargrind::Logic {
// ============================================
// ÉTATS DU MATCHMAKING
// ============================================
enum class MatchmakingState {
    IDLE,
    SEARCHING,
    MATCH_FOUND,
    LOADING,
    READY
};
// ============================================
// CALLBACKS
// ============================================
using StateChangedCallback = std::function<void(MatchmakingState)>;
using QueueUpdatedCallback = std::function<void(int current, int needed)>;
using MatchFoundCallback = std::function<void(const Network::MatchInfo&)>;
using ErrorCallback = std::function<void(const std::string&)>;
// ============================================
// CLASSE MATCHMAKING
// ============================================
class Matchmaking {
public:
    static Matchmaking* get();
    
    // Actions
    void startSearch();
    void cancelSearch();
    void confirmReady();
    
    // State
    MatchmakingState getState() const { return m_state; }
    int getQueueCount() const { return m_queueCount; }
    int getPlayersNeeded() const { return m_playersNeeded; }
    
    // Match Info
    const Network::MatchInfo& getCurrentMatch() const { return m_currentMatch; }
    bool hasMatch() const { return !m_currentMatch.match_id.empty(); }
    
    // Callbacks
    void setOnStateChanged(StateChangedCallback cb) { m_onStateChanged = cb; }
    void setOnQueueUpdated(QueueUpdatedCallback cb) { m_onQueueUpdated = cb; }
    void setOnMatchFound(MatchFoundCallback cb) { m_onMatchFound = cb; }
    void setOnError(ErrorCallback cb) { m_onError = cb; }
    
    // Polling control
    void update(float dt);
    
private:
    Matchmaking() = default;
    static Matchmaking* s_instance;
    
    MatchmakingState m_state = MatchmakingState::IDLE;
    Network::MatchInfo m_currentMatch;
    
    int m_queueCount = 0;
    int m_playersNeeded = 10;
    
    float m_pollTimer = 0;
    float m_pollInterval = 3.0f;
    
    StateChangedCallback m_onStateChanged;
    QueueUpdatedCallback m_onQueueUpdated;
    MatchFoundCallback m_onMatchFound;
    ErrorCallback m_onError;
    
    void setState(MatchmakingState state);
    void pollStatus();
    void handleMatchFound(const Network::MatchInfo& match);
};
