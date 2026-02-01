#include "GameSession.hpp"
#include "../core/Config.hpp"
namespace Stargrind::Logic {
GameSession* GameSession::s_instance = nullptr;
GameSession* GameSession::get() {
    if (!s_instance) {
        s_instance = new GameSession();
    }
    return s_instance;
}
void GameSession::setState(SessionState state) {
    m_state = state;
    if (m_onStateChange) {
        m_onStateChange(state);
    }
}
void GameSession::joinQueue() {
    if (m_state != SessionState::IDLE) return;
    
    Network::API::get()->joinQueue(
        [this](int queueCount, int playersNeeded) {
            setState(SessionState::IN_QUEUE);
            startPolling();
            log::info("Joined queue: {}/10", queueCount);
        },
        [](const std::string& error) {
            log::error("Failed to join queue: {}", error);
        }
    );
}
void GameSession::leaveQueue() {
    stopPolling();
    
    Network::API::get()->leaveQueue(
        [this](const std::string&) {
            setState(SessionState::IDLE);
        },
        [](const std::string& error) {
            log::error("Failed to leave queue: {}", error);
        }
    );
}
void GameSession::startPolling() {
    if (m_pollingActive) return;
    m_pollingActive = true;
    pollStatus();
}
void GameSession::stopPolling() {
    m_pollingActive = false;
}
void GameSession::pollStatus() {
    if (!m_pollingActive || m_state != SessionState::IN_QUEUE) return;
    
    Network::API::get()->checkStatus(
        [this](int queueCount, int playersNeeded) {
            // Toujours en attente, continuer le polling
            if (m_pollingActive) {
                // Schedule next poll in 5 seconds
                // (Utilisez le scheduler de Cocos2d)
            }
        },
        [this](const Network::MatchInfo& match) {
            // Match trouvé!
            stopPolling();
            startMatch(match);
        },
        [](const std::string& error) {
            log::error("Status check failed: {}", error);
        }
    );
}
void GameSession::startMatch(const Network::MatchInfo& match) {
    m_currentMatch = match;
    
    // Passer à l'écran versus
    setState(SessionState::VERSUS_SCREEN);
    
    if (m_onMatchFound) {
        m_onMatchFound(match);
    }
    
    // Sauvegarder les étoiles de base
    m_baseStars = Config::Utils::getCurrentStars();
}
void GameSession::endMatch() {
    if (!isHost()) {
        log::warn("Only host can end match");
        return;
    }
    
    Network::API::get()->endMatch(
        m_currentMatch.match_id,
        [this](const std::string& result) {
            setState(SessionState::RESULTS);
            log::info("Match ended: {}", result);
        },
        [](const std::string& error) {
            log::error("Failed to end match: {}", error);
        }
    );
}
void GameSession::setBaseStars(int stars) {
    m_baseStars = stars;
}
int GameSession::getStarsGained() const {
    return Config::Utils::getCurrentStars() - m_baseStars;
}
void GameSession::syncScore() {
    int starsGained = getStarsGained();
    if (starsGained < 0) starsGained = 0;
    
    Network::API::get()->updateScore(
        m_currentMatch.match_id,
        starsGained,
        [](const std::string&) {
            log::info("Score synced");
        },
        [](const std::string& error) {
            log::error("Score sync failed: {}", error);
        }
    );
}
void GameSession::skipLevel() {
    // Le skip est géré côté client
    // On sync juste le score actuel
    syncScore();
}
void GameSession::startMatchTimer() {
    m_timeRemaining = Config::MATCH_DURATION;
    setState(SessionState::PLAYING);
    
    // Démarrer le timer
    // (Utilisez le scheduler de Cocos2d pour appeler updateTimer toutes les secondes)
}
void GameSession::updateTimer(float dt) {
    if (m_state != SessionState::PLAYING && m_state != SessionState::LAST_TRY) return;
    
    m_timeRemaining -= dt;
    
    if (m_onTimeUpdate) {
        m_onTimeUpdate(m_timeRemaining);
    }
    
    if (m_timeRemaining <= 0 && m_state == SessionState::PLAYING) {
        // Temps écoulé - mode "dernier essai"
        setState(SessionState::LAST_TRY);
    }
}
void GameSession::onLastTryComplete(bool completed) {
    // Appelé quand le joueur meurt ou gagne après le temps
    syncScore();
    enterSpectatorMode();
}
void GameSession::enterSpectatorMode() {
    setState(SessionState::SPECTATING);
    log::info("Entered spectator mode");
}
bool GameSession::isHost() const {
    return m_currentMatch.host_id == Config::Utils::getPlayerId();
}
} // namespace Stargrind::Logic