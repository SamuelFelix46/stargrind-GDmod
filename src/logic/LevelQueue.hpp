#pragma once
#include <Geode/Geode.hpp>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

using namespace geode::prelude;

struct QueuedLevel {
    int levelID;
    GJGameLevel* level = nullptr;
    bool isDownloaded = false;
    bool isMusicReady = false;
    int difficulty; // 1-10
    int stars;
};

class LevelQueue {
private:
    static LevelQueue* s_instance;
    
    std::queue<QueuedLevel> m_levelQueue;
    mutable std::mutex m_queueMutex;
    std::atomic<bool> m_isDownloading{false};
    std::atomic<bool> m_shouldStop{false};
    std::condition_variable m_cv;
    
    GJGameLevel* m_currentLevel = nullptr;
    int m_currentIndex = 0;
    int m_targetDifficulty = 5;
    int m_minQueueSize = 3;
    
    std::thread m_downloadThread;
    
    void downloadWorker();
    void fetchLevelsByDifficulty(int difficulty);
    void downloadLevelData(QueuedLevel& ql);
    void downloadLevelMusic(QueuedLevel& ql);
    
public:
    static LevelQueue* get();
    static void destroy();
    
    void init(int averageDifficulty);
    void start();
    void stop();
    void reset();
    
    // Obtenir le prochain niveau (instantané si préchargé)
    GJGameLevel* getNextLevel();
    GJGameLevel* peekNextLevel() const;
    
    // Actions de jeu
    void skipCurrent();
    void onLevelComplete();
    void onLevelFail();
    
    // État
    bool isNextReady() const;
    int getQueueSize() const;
    int getCurrentIndex() const { return m_currentIndex; }
    GJGameLevel* getCurrentLevel() const { return m_currentLevel; }
    
    // Callbacks pour la transition
    using LevelReadyCallback = std::function<void(GJGameLevel*)>;
    void setOnNextLevelReady(LevelReadyCallback cb) { m_onNextReady = cb; }
    
private:
    LevelReadyCallback m_onNextReady;
};