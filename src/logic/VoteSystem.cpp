#include "VoteSystem.hpp"
#include "../core/Config.hpp"
#include "../core/Localization.hpp"
namespace Stargrind::Logic {
VoteSystem* VoteSystem::s_instance = nullptr;
VoteSystem* VoteSystem::get() {
    if (!s_instance) {
        s_instance = new VoteSystem();
    }
    return s_instance;
}
void VoteSystem::startVoting(const std::string& matchId, int playerCount) {
    m_matchId = matchId;
    m_votingActive = true;
    m_hasVoted = false;
    m_timeRemaining = Config::VOTE_TIMEOUT;
    
    // Reset des votes
    m_voteResult = VoteResult();
    m_voteResult.requiredVotes = playerCount;
    
    log::info("Voting started for match: {} ({} players)", matchId, playerCount);
}
void VoteSystem::castVote(DifficultyLevel difficulty) {
    if (!m_votingActive || m_hasVoted) return;
    
    m_hasVoted = true;
    m_lastVote = difficulty;
    
    Network::API::get()->voteDifficulty(
        m_matchId,
        static_cast<int>(difficulty),
        [this](int currentVotes, int totalPlayers) {
            m_voteResult.totalVotes = currentVotes;
            m_voteResult.voteCounts[static_cast<int>(m_lastVote)]++;
            
            if (m_onVoteUpdated) {
                m_onVoteUpdated(m_voteResult);
            }
            
            // Si tout le monde a voté
            if (currentVotes >= m_voteResult.requiredVotes) {
                calculateResult();
            }
        },
        [this](const std::string& error) {
            log::error("Vote failed: {}", error);
            m_hasVoted = false; // Permettre de réessayer
        }
    );
    
    log::info("Voted for difficulty: {}", static_cast<int>(difficulty));
}
void VoteSystem::skipVote() {
    // Skip = vote NORMAL par défaut
    castVote(DifficultyLevel::NORMAL);
}
void VoteSystem::update(float dt) {
    if (!m_votingActive) return;
    
    m_timeRemaining -= dt;
    
    if (m_timeRemaining <= 0) {
        // Timeout - forcer le vote par défaut si pas voté
        if (!m_hasVoted) {
            skipVote();
        }
        
        if (m_onVoteTimeout) {
            m_onVoteTimeout();
        }
        
        // Calculer le résultat avec les votes actuels
        calculateResult();
    }
}
void VoteSystem::reset() {
    m_votingActive = false;
    m_hasVoted = false;
    m_timeRemaining = 0;
    m_voteResult = VoteResult();
    m_levelSelection = LevelSelection();
}
void VoteSystem::calculateResult() {
    m_votingActive = false;
    
    // Déterminer la difficulté la plus votée
    DifficultyLevel selected = getMostVotedDifficulty();
    m_voteResult.selectedDifficulty = selected;
    
    log::info("Vote complete! Selected difficulty: {}", static_cast<int>(selected));
    
    // Récupérer les niveaux du serveur
    fetchLevels();
}
DifficultyLevel VoteSystem::getMostVotedDifficulty() {
    // Calculer la moyenne pondérée des votes
    int totalWeight = 0;
    int weightedSum = 0;
    
    for (int i = 0; i < 7; i++) {
        weightedSum += m_voteResult.voteCounts[i] * i;
        totalWeight += m_voteResult.voteCounts[i];
    }
    
    if (totalWeight == 0) {
        return DifficultyLevel::NORMAL;
    }
    
    int average = weightedSum / totalWeight;
    return static_cast<DifficultyLevel>(average);
}
void VoteSystem::fetchLevels() {
    Network::API::get()->getLevels(
        m_matchId,
        [this](const std::vector<Network::LevelInfo>& levels, const std::string& difficulty) {
            m_levelSelection.levels = levels;
            m_levelSelection.isReady = false;
            
            // Mapper la difficulté string vers enum
            if (difficulty == "easy") m_levelSelection.difficulty = DifficultyLevel::EASY;
            else if (difficulty == "normal") m_levelSelection.difficulty = DifficultyLevel::NORMAL;
            else if (difficulty == "hard") m_levelSelection.difficulty = DifficultyLevel::HARD;
            else if (difficulty == "harder") m_levelSelection.difficulty = DifficultyLevel::HARDER;
            else if (difficulty == "insane") m_levelSelection.difficulty = DifficultyLevel::INSANE;
            else if (difficulty == "demon") m_levelSelection.difficulty = DifficultyLevel::DEMON;
            else m_levelSelection.difficulty = DifficultyLevel::EXTREME_DEMON;
            
            log::info("Received {} levels for difficulty: {}", levels.size(), difficulty);
            
            // Précharger les niveaux
            preloadLevels();
            
            if (m_onVoteComplete) {
                m_onVoteComplete(m_levelSelection.difficulty, levels);
            }
        },
        [this](const std::string& error) {
            log::error("Failed to fetch levels: {}", error);
        }
    );
}
void VoteSystem::preloadLevels() {
    auto glm = GameLevelManager::sharedState();
    
    for (const auto& level : m_levelSelection.levels) {
        downloadLevel(level.id);
    }
}
void VoteSystem::downloadLevel(int levelId) {
    auto glm = GameLevelManager::sharedState();
    
    // Vérifier si le niveau est déjà téléchargé
    auto savedLevel = glm->getSavedLevel(levelId);
    if (savedLevel) {
        log::info("Level {} already downloaded", levelId);
        return;
    }
    
    // Télécharger le niveau
    glm->downloadLevel(levelId, false);
    log::info("Downloading level: {}", levelId);
}
} // namespace Stargrind::Logic