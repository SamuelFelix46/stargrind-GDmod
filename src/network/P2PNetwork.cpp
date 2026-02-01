#include "P2PNetwork.hpp"
#include "../core/Config.hpp"
#include <sstream>
#include <enet/enet.h>

namespace Stargrind::Network {

P2PNetwork* P2PNetwork::s_instance = nullptr;
static ENetHost* g_enetHost = nullptr;

P2PNetwork* P2PNetwork::get() {
    if (!s_instance) {
        s_instance = new P2PNetwork();
    }
    return s_instance;
}

bool P2PNetwork::initialize(int port) {
    if (m_running) return true;

    if (enet_initialize() != 0) {
        log::error("Erreur lors de l'initialisation d'ENet !");
        return false;
    }

    m_port = port;
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = (enet_uint16)port;

    // Création du client/serveur P2P (32 slots, 2 canaux)
    g_enetHost = enet_host_create(&address, 32, 2, 0, 0);

    if (g_enetHost == nullptr) {
        log::error("Impossible de créer le host ENet sur le port {}", port);
        return false;
    }

    m_running = true;
    log::info("Réseau P2P démarré sur le port {}", port);
    return true;
}

void P2PNetwork::shutdown() {
    if (!m_running) return;

    disconnectAll();
    
    if (g_enetHost) {
        enet_host_destroy(g_enetHost);
        g_enetHost = nullptr;
    }
    
    enet_deinitialize();
    m_running = false;
    log::info("Réseau P2P arrêté.");
}

void P2PNetwork::connectToPeers(const std::vector<PeerInfo>& peers) {
    if (!g_enetHost) return;

    for (const auto& peer : peers) {
        // Ne pas se connecter à soi-même
        if (peer.gd_id == Config::Utils::getPlayerId()) continue;
        if (m_peers.count(peer.gd_id)) continue;

        ENetAddress address;
        enet_address_set_host(&address, peer.ip_address.c_str());
        address.port = (enet_uint16)peer.port;

        ENetPeer* enetPeer = enet_host_connect(g_enetHost, &address, 2, 0);
        if (enetPeer) {
            m_peers[peer.gd_id] = peer;
            log::info("Tentative de connexion vers : {} ({})", peer.player_name, peer.ip_address);
        }
    }
}

void P2PNetwork::disconnectAll() {
    for (auto const& [id, peer] : m_peers) {
        // En théorie, on devrait déconnecter chaque ENetPeer ici
        // Mais vider la map et shutdown le host suffit pour un clean stop
    }
    m_peers.clear();
    m_scores.clear();
}

void P2PNetwork::broadcastMessage(const P2PMessage& message) {
    if (!g_enetHost) return;

    std::string data = serializeMessage(message);
    ENetPacket* packet = enet_packet_create(data.c_str(), data.size() + 1, ENET_PACKET_FLAG_RELIABLE);
    
    enet_host_broadcast(g_enetHost, 0, packet);
}

void P2PNetwork::sendToPeer(const std::string& gd_id, const P2PMessage& message) {
    // Note: Pour implémenter l'envoi ciblé, il faudrait stocker le pointeur ENetPeer 
    // dans la structure PeerInfo. Pour l'instant, on broadcast.
    broadcastMessage(message); 
}

void P2PNetwork::processIncomingMessages() {
    if (!m_running || !g_enetHost) return;

    ENetEvent event;
    // Timeout 0 pour ne pas bloquer le rendu du jeu
    while (enet_host_service(g_enetHost, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE: {
                if (event.packet->data) {
                    std::string data(reinterpret_cast<char*>(event.packet->data));
                    handleMessage(deserializeMessage(data));
                }
                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_CONNECT: {
                log::info("Connexion physique établie avec un peer.");
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                log::info("Un peer s'est déconnecté physiquement.");
                break;
            }
            default: break;
        }
    }
    
    // Vérification automatique des Timeouts (optionnel selon tes besoins)
    checkHeartbeats(0.1f); 
}

void P2PNetwork::handleMessage(const P2PMessage& message) {
    switch (message.type) {
        case P2PMessage::Type::HEARTBEAT: {
            if (m_peers.count(message.sender_id)) {
                m_peers[message.sender_id].lastHeartbeat = message.timestamp;
                m_peers[message.sender_id].isConnected = true;
            }
            break;
        }
        
        case P2PMessage::Type::SCORE_UPDATE: {
            std::istringstream iss(message.payload);
            std::string s_stars, s_level, s_progress;
            
            ScoreData score;
            score.gd_id = message.sender_id;
            
            if (std::getline(iss, s_stars, '|') && 
                std::getline(iss, s_level, '|') && 
                std::getline(iss, s_progress, '|')) {
                
                score.stars_count = std::stoi(s_stars);
                score.current_level_id = std::stoi(s_level);
                score.progress = std::stof(s_progress);
                
                if (m_peers.count(message.sender_id)) {
                    score.player_name = m_peers[message.sender_id].player_name;
                }
                
                m_scores[score.gd_id] = score;
                
                if (m_onScoreUpdate) m_onScoreUpdate(getAllScores());
            }
            break;
        }

        case P2PMessage::Type::SYNC_REQUEST: {
            if (m_isHost) {
                P2PMessage res;
                res.type = P2PMessage::Type::SYNC_RESPONSE;
                res.sender_id = Config::Utils::getPlayerId();
                res.timestamp = static_cast<float>(time(nullptr));
                
                std::ostringstream oss;
                for (auto const& [id, sd] : m_scores) {
                    oss << id << ":" << sd.stars_count << ";";
                }
                res.payload = oss.str();
                broadcastMessage(res);
            }
            break;
        }

        default: break;
    }

    if (m_onMessage) m_onMessage(message);
}

void P2PNetwork::sendScoreUpdate(int stars, int levelId, float progress) {
    P2PMessage msg;
    msg.type = P2PMessage::Type::SCORE_UPDATE;
    msg.sender_id = Config::Utils::getPlayerId();
    msg.timestamp = static_cast<float>(time(nullptr));
    
    std::ostringstream oss;
    oss << stars << "|" << levelId << "|" << progress;
    msg.payload = oss.str();
    
    broadcastMessage(msg);
}

void P2PNetwork::sendHeartbeat() {
    P2PMessage msg;
    msg.type = P2PMessage::Type::HEARTBEAT;
    msg.sender_id = Config::Utils::getPlayerId();
    msg.timestamp = static_cast<float>(time(nullptr));
    broadcastMessage(msg);
}

std::string P2PNetwork::serializeMessage(const P2PMessage& message) {
    std::ostringstream oss;
    oss << static_cast<int>(message.type) << "|"
        << message.sender_id << "|"
        << message.timestamp << "|"
        << message.payload;
    return oss.str();
}

P2PMessage P2PNetwork::deserializeMessage(const std::string& data) {
    P2PMessage msg;
    std::istringstream iss(data);
    std::string s_type, s_ts;

    if (std::getline(iss, s_type, '|') &&
        std::getline(iss, msg.sender_id, '|') &&
        std::getline(iss, s_ts, '|') &&
        std::getline(iss, msg.payload)) {
        
        msg.type = static_cast<P2PMessage::Type>(std::stoi(s_type));
        msg.timestamp = std::stof(s_ts);
    }
    return msg;
}

std::vector<ScoreData> P2PNetwork::getAllScores() const {
    std::vector<ScoreData> res;
    for (auto const& [id, data] : m_scores) res.push_back(data);
    return res;
}

void P2PNetwork::checkHeartbeats(float dt) {
    float now = static_cast<float>(time(nullptr));
    for (auto& [id, peer] : m_peers) {
        if (peer.isConnected && (now - peer.lastHeartbeat > 10.0f)) {
            peer.isConnected = false;
            log::warn("Peer {} déconnecté (Timeout)", peer.player_name);
            if (m_onPeerDisconnected) m_onPeerDisconnected(id);
        }
    }
}

} // namespace Stargrind::Network