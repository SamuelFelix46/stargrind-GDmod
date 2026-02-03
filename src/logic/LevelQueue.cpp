#include "LevelQueue.hpp"
#include "GameSession.hpp"

LevelQueue* LevelQueue::s_instance = nullptr;

LevelQueue* LevelQueue::get() {
    if (!s_instance) {
        s_instance = new LevelQueue();
    }
    return s_instance;
}

void LevelQueue::destroy() {
    if (s_instance) {
        s_instance->stop();
        delete s_instance;
        s_instance = nullptr;
    }
}

void LevelQueue::init(int averageDifficulty) {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    
    // Vider la queue
    while (!m_levelQueue.empty()) {
        m_levelQueue.pop();
    }
    
    m_targetDifficulty = averageDifficulty;
    m_currentIndex = 0;
    m_currentLevel = nullptr;
    m_shouldStop = false;
    
    // Précharger les premiers niveaux
    fetchLevelsByDifficulty(averageDifficulty);
}

void LevelQueue::start() {
    if (m_downloadThread.joinable()) {
        m_shouldStop = true;
        m_cv.notify_all();
        m_downloadThread.join();
    }
    
    m_shouldStop = false;
    m_downloadThread = std::thread(&LevelQueue::downloadWorker, this);
}

void LevelQueue::stop() {
    m_shouldStop = true;
    m_cv.notify_all();
    
    if (m_downloadThread.joinable()) {
        m_downloadThread.join();
    }
}

void LevelQueue::reset() {
    stop();
    
    std::lock_guard<std::mutex> lock(m_queueMutex);
    while (!m_levelQueue.empty()) {
        m_levelQueue.pop();
    }
    m_currentLevel = nullptr;
    m_currentIndex = 0;
}

void LevelQueue::downloadWorker() {
    while (!m_shouldStop) {
        // Vérifier si on a besoin de plus de niveaux
        int queueSize = getQueueSize();
        
        if (queueSize < m_minQueueSize && !m_isDownloading) {
            m_isDownloading = true;
            
            // Récupérer des niveaux dans le main thread (GD API)
            Loader::get()->queueInMainThread([this]() {
                fetchLevelsByDifficulty(m_targetDifficulty);
                m_isDownloading = false;
            });
        }
        
        // Attendre un peu avant de revérifier
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_cv.wait_for(lock, std::chrono::seconds(2), [this]() {
            return m_shouldStop.load();
        });
    }
}

void LevelQueue::fetchLevelsByDifficulty(int difficulty) {
    auto glm = GameLevelManager::sharedState();
    
    // Créer l'objet de recherche
    auto searchObj = GJSearchObject::create(SearchType::Search);
    
    // Mapper difficulté (1-10) vers filtres GD
    int gdDiff = 0;
    bool isDemon = false;
    
    if (difficulty <= 2) {
        gdDiff = 1; // Easy
    } else if (difficulty <= 4) {
        gdDiff = 2; // Normal
    } else if (difficulty <= 5) {
        gdDiff = 3; // Hard
    } else if (difficulty <= 6) {
        gdDiff = 4; // Harder
    } else if (difficulty <= 7) {
        gdDiff = 5; // Insane
    } else {
        isDemon = true; // Demon+
    }
    
    searchObj->m_difficulty = gdDiff;
    searchObj->m_demonFilter = isDemon ? 1 : 0;
    searchObj->m_starFilter = true; // Seulement les niveaux étoilés
    
    // TODO: Implémenter le callback pour recevoir les niveaux
    // glm->getOnlineLevels(searchObj);
    
    // Pour le prototype, simuler avec des niveaux locaux
    log::info("Fetching levels with difficulty: {}", difficulty);
}

void LevelQueue::downloadLevelData(QueuedLevel& ql) {
    if (!ql.level) return;
    
    auto glm = GameLevelManager::sharedState();
    
    // Télécharger les données du niveau
    // glm->downloadLevel(ql.levelID, false);
    
    ql.isDownloaded = true;
}

void LevelQueue::downloadLevelMusic(QueuedLevel& ql) {
    if (!ql.level) return;
    
    // Récupérer l'ID de la musique
    int songID = ql.level->m_songID;
    
    if (songID > 0) {
        // Télécharger la musique custom
        // MusicDownloadManager::sharedState()->downloadSong(songID);
    }
    
    ql.isMusicReady = true;
}

GJGameLevel* LevelQueue::getNextLevel() {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    
    if (m_levelQueue.empty()) {
        log::warn("Level queue is empty!");
        return nullptr;
    }
    
    auto& next = m_levelQueue.front();
    m_currentLevel = next.level;
    m_levelQueue.pop();
    m_currentIndex++;
    
    // Notifier qu'on a besoin de plus de niveaux
    m_cv.notify_one();
    
    return m_currentLevel;
}

GJGameLevel* LevelQueue::peekNextLevel() const {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    
    if (m_levelQueue.empty()) {
        return nullptr;
    }
    
    return m_levelQueue.front().level;
}

void LevelQueue::skipCurrent() {
    // Son de skip
    FMODAudioEngine::sharedEngine()->playEffect("skip.mp3"_spr);
    
    // Notifier la session
    GameSession::get()->onSkip();
    
    // Passer au niveau suivant
    auto nextLevel = getNextLevel();
    
    if (nextLevel && m_onNextReady) {
        // Transition fluide dans le main thread
        Loader::get()->queueInMainThread([this, nextLevel]() {
            m_onNextReady(nextLevel);
        });
    }
}

void LevelQueue::onLevelComplete() {
    // Son de victoire
    FMODAudioEngine::sharedEngine()->playEffect("level_complete.mp3"_spr);
    
    // Ajouter les étoiles au score
    if (m_currentLevel) {
        int stars = m_currentLevel->m_stars;
        GameSession::get()->onLevelComplete(stars);
    }
    
    // Charger le niveau suivant après un court délai
    auto nextLevel = getNextLevel();
    
    if (nextLevel && m_onNextReady) {
        Loader::get()->queueInMainThread([this, nextLevel]() {
            // Petit délai pour l'effet visuel
            auto scene = CCDirector::sharedDirector()->getRunningScene();
            if (scene) {
                scene->runAction(CCSequence::create(
                    CCDelayTime::create(0.5f),
                    CCCallFunc::create([this, nextLevel]() {
                        m_onNextReady(nextLevel);
                    }),
                    nullptr
                ));
            }
        });
    }
}

void LevelQueue::onLevelFail() {
    GameSession::get()->onLevelFail();
}

bool LevelQueue::isNextReady() const {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    
    if (m_levelQueue.empty()) {
        return false;
    }
    
    const auto& next = m_levelQueue.front();
    return next.isDownloaded && next.isMusicReady;
}

int LevelQueue::getQueueSize() const {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return static_cast<int>(m_levelQueue.size());
}