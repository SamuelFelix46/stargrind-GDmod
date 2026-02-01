#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <unordered_map>
#include <vector>
using namespace geode::prelude;
namespace Stargrind {
class Localization {
public:
    static Localization* get();
    
    void loadLanguage(const std::string& langCode);
    std::string getText(int lineId) const;
    std::string getText(const std::string& key) const;
    
    std::string getCurrentLanguage() const { return m_currentLang; }
    std::vector<std::string> getAvailableLanguages() const;
    
private:
    Localization() = default;
    static Localization* s_instance;
    
    std::string m_currentLang = "en";
    std::unordered_map<int, std::string> m_lineTexts;
    std::unordered_map<std::string, std::string> m_keyTexts;
    
    void parseFile(const std::string& content);
};
// Macro pour accéder facilement aux textes
#define LOC(id) Stargrind::Localization::get()->getText(id)
#define LOC_KEY(key) Stargrind::Localization::get()->getText(key)
} // namespace Stargrind