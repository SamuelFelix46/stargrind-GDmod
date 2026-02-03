#include "GameSession.hpp"
#include "../network/P2PNetwork.hpp"
#include "LevelQueue.hpp"

GameSession* GameSession::s_instance = nullptr;

GameSession* GameSession::get() {
    if (!s_instance) {
        s_instance = new GameSession();
    }
    return s_instance;
}

void GameSession::destroy() {
    delete s_instance;
    s_instance = nullptr;
}

void GameSession::startSession(int myTeamId,
                                const std::vector<std::string>& team1Members,
                                const std::vector<std::string>& team2Members) {
    m_myTeamId = myTeamId;
    
    m_team1 = TeamScore{1, 0, team1Members};
    m_team2 = TeamScore{2, 0, team2Members};
    
    // Reset stats
    m_myStats = PlayerStats{};
    m_myStats.odName = GJAccountManager::sharedState()->m_username;
    
    // Démarrer le timer
    m_startTime = std::chrono::steady_clock::now();
    m_timerPaused = false;
    
    setState(SessionState::Playing);
    
    log::info("Game session started! Team {} vs Team {}", 
              team1Members.size(), team2Members.size());
}

void GameSession::endSession() {
    setState(SessionState::Finished);
    LevelQueue::get()->stop();
    
    log::info("Game session ended. Final scores: {} vs {}", 
              m_team1.totalStars, m_team2.totalStars);
}

void GameSession::reset() {
    m_state = SessionState::NotStarted;
    m_team1 = TeamScore{};
    m_team2 = TeamScore{};
    m_myStats = PlayerStats{};
    m_myTeamId = 0;
    m_timerPaused = false;
}

int GameSession::getRemainingSeconds() const {
    if (m_timerPaused) {
        return m_pausedTimeRemaining;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - m_startTime
    ).count();
    
    int remaining = m_durationSeconds - static_cast<int>(elapsed);
    return std::max(0, remaining);
}

std::string GameSession::getFormattedTime() const {
    int total = getRemainingSeconds();
    int mins = total / 60;
    int secs = total % 60;
    
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", mins, secs);
    return std::string(buffer);
}

bool GameSession::isTimeUp() const {
    return getRemainingSeconds() <= 0;
}

void GameSession::pauseTimer() {
    if (!m_timerPaused) {
        m_pausedTimeRemaining = getRemainingSeconds();
        m_timerPaused = true;
    }
}

void GameSession::resumeTimer() {
    if (m_timerPaused) {
        // Recalculer le temps de départ
        m_startTime = std::chrono::steady_clock::now() - 
                      std::chrono::seconds(m_durationSeconds - m_pausedTimeRemaining);
        m_timerPaused = false;
    }
}

void GameSession::setState(SessionState newState) {
    if (m_state != newState) {
        m_state = newState;
        
        if (m_onStateChange) {
            m_onStateChange(newState);
        }
        
        log::debug("Session state changed to: {}", static_cast<int>(newState));
    }
}

void GameSession::addScore(int stars) {
    TeamScore& myTeam = (m_myTeamId == 1) ? m_team1 : m_team2;
    myTeam.totalStars += stars;
    
    // Envoyer via P2P
    Stargrind::Network::P2PNetwork::get()->sendScoreUpdate(
        stars,
        LevelQueue::get()->getCurrentLevel() ? 
            LevelQueue::get()->getCurrentLevel()->m_levelID : 0,
        0.0f
    );
    
    if (m_onScoreUpdate) {
        m_onScoreUpdate(m_myTeamId, myTeam.totalStars);
    }
}

void GameSession::receiveTeamScore(int teamId, int additionalStars) {
    TeamScore& team = (teamId == 1) ? m_team1 : m_team2;
    team.totalStars += additionalStars;
    
    if (m_onScoreUpdate) {
        m_onScoreUpdate(teamId, team.totalStars);
    }
}

int GameSession::getMyTeamScore() const {
    return (m_myTeamId == 1) ? m_team1.totalStars : m_team2.totalStars;
}

int GameSession::getOpponentTeamScore() const {
    return (m_myTeamId == 1) ? m_team2.totalStars : m_team1.totalStars;
}

void GameSession::onLevelComplete(int starsEarned) {
    m_myStats.starsEarned += starsEarned;
    m_myStats.levelsCompleted++;
    m_myStats.highestProgress = 100;
    
    addScore(starsEarned);
    
    log::info("Level complete! +{} stars. Total: {}", 
              starsEarned, m_myStats.starsEarned);
}

void GameSession::onLevelFail() {
    m_myStats.levelsFailed++;
}

void GameSession::onSkip() {
    m_myStats.skipsUsed++;
    m_myStats.highestProgress = 0;
    
    log::debug("Level skipped. Total skips: {}", m_myStats.skipsUsed);
}

void GameSession::onProgress(int percent) {
    if (percent > m_myStats.highestProgress) {
        m_myStats.highestProgress = percent;
    }
}

void GameSession::update(float dt) {
    if (m_state != SessionState::Playing) return;
    
    // Vérifier si le temps est écoulé
    if (isTimeUp()) {
        setState(SessionState::LastAttempt);
        
        if (m_onTimeUp) {
            m_onTimeUp();
        }
        
        log::info("Time's up! Entering last attempt mode.");
    }
}