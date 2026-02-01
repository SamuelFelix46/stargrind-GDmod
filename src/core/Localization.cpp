#include "Localization.hpp"
#include "Config.hpp"
#include <fstream>
#include <sstream>
namespace Stargrind {
Localization* Localization::s_instance = nullptr;
Localization* Localization::get() {
    if (!s_instance) {
        s_instance = new Localization();
        // Charger la langue par défaut depuis les settings
        auto lang = Mod::get()->getSettingValue<std::string>("language");
        s_instance->loadLanguage(lang);
    }
    return s_instance;
}
void Localization::loadLanguage(const std::string& langCode) {
    m_currentLang = langCode;
    m_lineTexts.clear();
    m_keyTexts.clear();
    
    // Construire le chemin vers le fichier de langue
    auto path = Mod::get()->getResourcesDir() / "lang" / (langCode + ".txt");
    
    std::ifstream file(path);
    if (!file.is_open()) {
        log::warn("Could not open language file: {}", path.string());
        // Fallback vers anglais
        if (langCode != "en") {
            loadLanguage("en");
        }
        return;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    parseFile(buffer.str());
    
    log::info("Loaded language: {} ({} entries)", langCode, m_lineTexts.size() + m_keyTexts.size());
}
void Localization::parseFile(const std::string& content) {
    std::istringstream stream(content);
    std::string line;
    int lineNum = 1;
    
    while (std::getline(stream, line)) {
        // Ignorer les lignes vides et commentaires
        if (line.empty() || line[0] == '#') {
            lineNum++;
            continue;
        }
        
        // Format: KEY=Value ou juste Value (pour accès par numéro de ligne)
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            m_keyTexts[key] = value;
        }
        
        // Toujours stocker par numéro de ligne aussi
        m_lineTexts[lineNum] = (pos != std::string::npos) ? line.substr(pos + 1) : line;
        lineNum++;
    }
}
std::string Localization::getText(int lineId) const {
    auto it = m_lineTexts.find(lineId);
    if (it != m_lineTexts.end()) {
        return it->second;
    }
    return fmt::format("[Missing: Line {}]", lineId);
}
std::string Localization::getText(const std::string& key) const {
    auto it = m_keyTexts.find(key);
    if (it != m_keyTexts.end()) {
        return it->second;
    }
    return fmt::format("[Missing: {}]", key);
}
std::vector<std::string> Localization::getAvailableLanguages() const {
    return {"en", "fr", "es", "de"};
}
