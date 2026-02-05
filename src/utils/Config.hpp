#pragma once
#include <Geode/Geode.hpp>
#include <string>

using namespace geode::prelude;

class Config {
private:
    static Config* s_instance;
    
    bool m_ldmMode = false;
    bool m_disableShake = false;
    std::string m_language = "en";
    float m_musicVolume = 1.0f;
    float m_sfxVolume = 1.0f;
    
    Config() { load(); }
    
public:
    static Config* get() {
        if (!s_instance) {
            s_instance = new Config();
        }
        return s_instance;
    }
    
    // ═══════════════════════════════════════════════════════════════
    // GETTERS
    // ═══════════════════════════════════════════════════════════════
    
    bool isLDMEnabled() const { return m_ldmMode; }
    bool isShakeDisabled() const { return m_disableShake; }
    std::string getLanguage() const { return m_language; }
    float getMusicVolume() const { return m_musicVolume; }
    float getSFXVolume() const { return m_sfxVolume; }
    
    // ═══════════════════════════════════════════════════════════════
    // SETTERS
    // ═══════════════════════════════════════════════════════════════
    
    void setLDMEnabled(bool enabled) {
        m_ldmMode = enabled;
        save();
    }
    
    void setShakeDisabled(bool disabled) {
        m_disableShake = disabled;
        save();
    }
    
    void setLanguage(const std::string& lang) {
        m_language = lang;
        save();
    }
    
    void setMusicVolume(float vol) {
        m_musicVolume = std::clamp(vol, 0.0f, 1.0f);
        save();
    }
    
    void setSFXVolume(float vol) {
        m_sfxVolume = std::clamp(vol, 0.0f, 1.0f);
        save();
    }
    
    // ═══════════════════════════════════════════════════════════════
    // PERSISTENCE
    // ═══════════════════════════════════════════════════════════════
    
    void save() {
        auto mod = Mod::get();
        mod->setSavedValue("ldm_mode", m_ldmMode);
        mod->setSavedValue("disable_shake", m_disableShake);
        mod->setSavedValue("language", m_language);
        mod->setSavedValue("music_volume", m_musicVolume);
        mod->setSavedValue("sfx_volume", m_sfxVolume);
    }
    
    void load() {
        auto mod = Mod::get();
        m_ldmMode = mod->getSavedValue<bool>("ldm_mode", false);
        m_disableShake = mod->getSavedValue<bool>("disable_shake", false);
        m_language = mod->getSavedValue<std::string>("language", "en");
        m_musicVolume = mod->getSavedValue<float>("music_volume", 1.0f);
        m_sfxVolume = mod->getSavedValue<float>("sfx_volume", 1.0f);
    }
    
    void reset() {
        m_ldmMode = false;
        m_disableShake = false;
        m_language = "en";
        m_musicVolume = 1.0f;
        m_sfxVolume = 1.0f;
        save();
    }
};

// Initialisation du singleton
Config* Config::s_instance = nullptr;