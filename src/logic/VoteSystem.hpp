#pragma once
#include <Geode/Geode.hpp>
#include "../network/API.hpp"
#include <functional>
#include <vector>
using namespace geode::prelude;
namespace Stargrind::Logic {
// ============================================
// NIVEAUX DE DIFFICULTÉ
// ============================================
enum class DifficultyLevel {
    EASY = 0,
    NORMAL = 1,
    HARD = 2,
    HARDER = 3,
    INSANE = 4,
    EASY_DEMON = 5,
    MEDIUM_DEMON = 6,
    HARD_DEMON = 7,
    INSANE_DEMON = 8,
    EXTREME_DEMON = 6
};
// ============================================
// STRUCTURES
// ============================================
struct VoteResult {
    int voteCounts[7] = {0}; // Par niveau de difficulté
    DifficultyLevel selectedDifficulty = DifficultyLevel::NORMAL;
    int totalVotes = 0;
    int requiredVotes = 10;
};
struct LevelSelection {
    std::vector<Network::LevelInfo> levels;
    DifficultyLevel difficulty;
    bool isReady = false;
};
// ============================================
// CALLBACKS
// ============================================
using VoteUpdatedCallback = std::function<void(const VoteResult&)>;
using VoteCompleteCallback = std::function<void(DifficultyLevel, const std::vector<Network::LevelInfo>&)>;
using LevelsLoadedCallback = std::function<void(const LevelSelection&)>;
using VoteTimeoutCallback = std::function<void()>;
// ============================================
// CLASSE VOTE SYSTEM
// ============================================
class VoteSystem {
public:
    static VoteSystem* get();
    
    // Voting
    void startVoting(const std::string& matchId, int playerCount);
    void castVote(DifficultyLevel difficulty);
    void skipVote(); // Compte comme un vote par défaut (NORMAL)
    
    // State
    bool isVotingActive() const { return m_votingActive; }
    float getTimeRemaining() const { return m_timeRemaining; }
    bool hasVoted() const { return m_hasVoted; }
    const VoteResult& getCurrentResult() const { return m_voteResult; }
    
    // Levels
    const LevelSelection& getLevelSelection() const { return m_levelSelection; }
    void preloadLevels();
    
    // Callbacks
    void setOnVoteUpdated(VoteUpdatedCallback cb) { m_onVoteUpdated = cb; }
    void setOnVoteComplete(VoteCompleteCallback cb) { m_onVoteComplete = cb; }
    void setOnLevelsLoaded(LevelsLoadedCallback cb) { m_onLevelsLoaded = cb; }
    void setOnVoteTimeout(VoteTimeoutCallback cb) { m_onVoteTimeout = cb; }
    
    // Update
    void update(float dt);
    void reset();
    
private:
    VoteSystem() = default;
    static VoteSystem* s_instance;
    
    std::string m_matchId;
    bool m_votingActive = false;
    bool m_hasVoted = false;
    float m_timeRemaining = 0;
    DifficultyLevel m_lastVote = DifficultyLevel::NORMAL;
    
    VoteResult m_voteResult;
    LevelSelection m_levelSelection;
    
    VoteUpdatedCallback m_onVoteUpdated;
    VoteCompleteCallback m_onVoteComplete;
    LevelsLoadedCallback m_onLevelsLoaded;
    VoteTimeoutCallback m_onVoteTimeout;
    
    void calculateResult();
    void fetchLevels();
    void downloadLevel(int levelId);
    DifficultyLevel getMostVotedDifficulty();
};
