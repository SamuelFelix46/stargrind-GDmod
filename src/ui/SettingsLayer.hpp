#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "Components/RGBBackground.hpp"
using namespace geode::prelude;
namespace Stargrind::UI {
class SettingsLayer : public Popup<> {
protected:
    bool setup() override;
    
    void onSave(CCObject*);
    void onToggleLDM(CCObject*);
    void onToggleShake(CCObject*);
    void onLanguageChange(CCObject*);
    void onClose(CCObject*) override;
    
    void updateToggleVisual(CCMenuItemToggler* toggle, bool state);
    void updateLanguageDisplay();
    
    RGBBackground* m_background = nullptr;
    CCMenuItemToggler* m_ldmToggle = nullptr;
    CCMenuItemToggler* m_shakeToggle = nullptr;
    CCLabelBMFont* m_languageLabel = nullptr;
    
    int m_selectedLanguageIndex = 0;
    std::vector<std::string> m_languages = {"en", "fr", "es", "de", "ru", "br", "pl"};
    std::vector<std::string> m_languageNames = {"English", "Français", "Español", "Deutsch", "Русский", "Português", "Polski"};
    
public:
    static SettingsLayer* create();
};
