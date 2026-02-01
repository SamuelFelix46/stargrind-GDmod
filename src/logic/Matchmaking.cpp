#include "Matchmaking.hpp"
#include "../core/Config.hpp"
#include "../network/P2PNetwork.hpp"
namespace Stargrind::Logic {
Matchmaking* Matchmaking::s_instance = nullptr;
Matchmaking* Matchmaking::get() {
    if (!s_instance) {
        s_instance = new Matchmaking();
    }
    return s_instance;
}
void Matchmaking::setState(MatchmakingState state) {
    if (m_state == state) return;
    
    m_state = state;
    
    if (m_onStateChanged) {
        m_onStateChanged(state);
    }
    
    log::info("Matchmaking state: {}", static_cast<int>(state));
}
void Matchmaking::startSearch() {
    if (m_state != MatchmakingState::IDLE) {
        log::warn("Cannot start search - not in IDLE state");
        return;
    }
    
    setState(MatchmakingState::SEARCHING);
    m_pollTimer = 0;
    
    // Rejoindre la file d'attente
    Network::API::get()->joinQueue(
        [this](int queueCount, int playersNeeded) {
            m_queueCount = queueCount;
            m_playersNeeded = playersNeeded;
            
            if (m_onQueueUpdated) {
                m_onQueueUpdated(queueCount, playersNeeded);
            }
            
            log::info("Joined queue: {}/{}", queueCount, Config::PLAYERS_PER_MATCH);
        },
        [this](const std::string& error) {
            setState(MatchmakingState::IDLE);
            
            if (m_onError) {
                m_onError(error);
            }
            
            log::error("Failed to join queue: {}", error);
        }
    );
}
void Matchmaking::cancelSearch() {
    if (m_state != MatchmakingState::SEARCHING) return;
    
    Network::API::get()->leaveQueue(
        [this](const std::string&) {
            setState(MatchmakingState::IDLE);
            m_queueCount = 0;
            m_playersNeeded = Config::PLAYERS_PER_MATCH;
            log::info("Left queue");
        },
        [this](const std::string& error) {
            if (m_onError) {
                m_onError(error);
            }
        }
    );
}
void Matchmaking::confirmReady() {
    if (m_state != MatchmakingState::MATCH_FOUND) return;
    
    // Initialiser le réseau P2P
    Network::P2PNetwork::get()->initialize();
    
    // Se connecter aux autres joueurs
    std::vector<Network::PeerInfo> peers;
    for (const auto& player : m_currentMatch.team_a) {
        Network::PeerInfo peer;
        peer.gd_id = player.gd_id;
        peer.player_name = player.player_name;
        // Note: IP/Port seront fournis par le serveur dans une version complète
        peers.push_back(peer);
    }
    for (const auto& player : m_currentMatch.team_b) {
        Network::PeerInfo peer;
        peer.gd_id = player.gd_id;
        peer.player_name = player.player_name;
        peers.push_back(peer);
    }
    
    Network::P2PNetwork::get()->connectToPeers(peers);
    
    // Si on est l'hôte, démarrer en mode host
    if (m_currentMatch.host_id == Config::Utils::getPlayerId()) {
        Network::P2PNetwork::get()->startHost(m_currentMatch.match_id);
    }
    
    setState(MatchmakingState::READY);
}
void Matchmaking::update(float dt) {
    if (m_state != MatchmakingState::SEARCHING) return;
    
    m_pollTimer += dt;
    
    if (m_pollTimer >= m_pollInterval) {
        m_pollTimer = 0;
        pollStatus();
    }
}
void Matchmaking::pollStatus() {
    Network::API::get()->checkStatus(
        [this](int queueCount, int playersNeeded) {
            m_queueCount = queueCount;
            m_playersNeeded = playersNeeded;
            
            if (m_onQueueUpdated) {
                m_onQueueUpdated(queueCount, playersNeeded);
            }
        },
        [this](const Network::MatchInfo& match) {
            handleMatchFound(match);
        },
        [this](const std::string& error) {
            log::error("Status poll failed: {}", error);
        }
    );
}
void Matchmaking::handleMatchFound(const Network::MatchInfo& match) {
    m_currentMatch = match;
    setState(MatchmakingState::MATCH_FOUND);
    
    // Jouer le son de match trouvé
    auto path = Mod::get()->getResourcesDir() / "match_found.mp3";
    FMODAudioEngine::sharedEngine()->playEffect(path.string());
    
    if (m_onMatchFound) {
        m_onMatchFound(match);
    }
    
    log::info("Match found! ID: {}, Team: {}", match.match_id, match.your_team);
}
} // namespace Stargrind::Logic