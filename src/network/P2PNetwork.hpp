#pragma once
#include <Geode/Geode.hpp>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

using namespace geode::prelude;

namespace Stargrind::Network {

// ============================================
// STRUCTURES P2P
// ============================================

struct PeerInfo {
    std::string gd_id;
    std::string player_name;
    std::string ip_address;
    int port;
    bool isConnected = false;
    float lastHeartbeat = 0;
};

struct P2PMessage {
    enum class Type {
        HEARTBEAT,
        SCORE_UPDATE,
        PLAYER_POSITION,
        LEVEL_SKIP,
        PLAYER_DIED,
        PLAYER_COMPLETED,
        SYNC_REQUEST,
        SYNC_RESPONSE
    };
    
    Type type;
    std::string sender_id;
    std::string payload;
    float timestamp;
};

struct ScoreData {
    std::string gd_id;
    std::string player_name;
    int stars_count;
    int current_level_id;
    float progress;
};

// ============================================
// CALLBACKS P2P
// ============================================

using PeerConnectedCallback = std::function<void(const PeerInfo& peer)>;
using PeerDisconnectedCallback = std::function<void(const std::string& gd_id)>;
using ScoreUpdateCallback = std::function<void(const std::vector<ScoreData>& scores)>;
using MessageCallback = std::function<void(const P2PMessage& message)>;

// ============================================
// CLASSE P2P NETWORK
// ============================================

class P2PNetwork {
public:
    static P2PNetwork* get();
    
    // Lifecycle
    bool initialize(int port = 7777);
    void shutdown();
    bool isRunning() const { return m_running; }
    
    // Connection Management
    void connectToPeers(const std::vector<PeerInfo>& peers);
    void disconnectFromPeer(const std::string& gd_id);
    void disconnectAll();
    
    // Hosting
    bool startHost(const std::string& matchId);
    void stopHost();
    bool isHost() const { return m_isHost; }
    
    // Messaging
    void broadcastMessage(const P2PMessage& message);
    void sendToPeer(const std::string& gd_id, const P2PMessage& message);
    void sendScoreUpdate(int starsGained, int levelId, float progress);
    void sendHeartbeat();
    
    // Sync
    void requestFullSync();
    std::vector<ScoreData> getAllScores() const;
    
    // Callbacks
    void setOnPeerConnected(PeerConnectedCallback cb) { m_onPeerConnected = cb; }
    void setOnPeerDisconnected(PeerDisconnectedCallback cb) { m_onPeerDisconnected = cb; }
    void setOnScoreUpdate(ScoreUpdateCallback cb) { m_onScoreUpdate = cb; }
    void setOnMessage(MessageCallback cb) { m_onMessage = cb; }
    
    // Info
    int getConnectedPeerCount() const;
    std::vector<PeerInfo> getConnectedPeers() const;
    
private:
    P2PNetwork() = default;
    static P2PNetwork* s_instance;
    
    bool m_running = false;
    bool m_isHost = false;
    int m_port = 7777;
    std::string m_matchId;
    
    std::unordered_map<std::string, PeerInfo> m_peers;
    std::unordered_map<std::string, ScoreData> m_scores;
    
    PeerConnectedCallback m_onPeerConnected;
    PeerDisconnectedCallback m_onPeerDisconnected;
    ScoreUpdateCallback m_onScoreUpdate;
    MessageCallback m_onMessage;
    
    // Internal
    void processIncomingMessages();
    void handleMessage(const P2PMessage& message);
    void checkHeartbeats(float dt);
    std::string serializeMessage(const P2PMessage& message);
    P2PMessage deserializeMessage(const std::string& data);
};

