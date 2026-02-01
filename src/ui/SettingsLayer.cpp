#include "SettingsLayer.hpp"
#include "../core/Config.hpp"
#include "../core/Localization.hpp"
#include "Components/HoverButton.hpp"
namespace Stargrind::UI {
SettingsLayer* SettingsLayer::create() {
    auto ret = new SettingsLayer();
    if (ret->initAnchored(350.f, 280.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}
bool SettingsLayer::setup() {
    setTitle(LOC_KEY("SETTINGS_TITLE"));
    
    auto winSize = m_mainLayer->getContentSize();
    auto center = winSize / 2;
    
    // RGB Background subtil
    m_background = RGBBackground::create();
    m_background->setContentSize(winSize);
    m_background->setLerpSpeed(0.2f);
    m_background->setZOrder(-10);
    m_mainLayer->addChild(m_background);
    
    auto menu = CCMenu::create();
    menu->setPosition({0, 0});
    m_mainLayer->addChild(menu);
    
    float yPos = 180;
    float labelX = 80;
    float toggleX = 280;
    
    // LDM Toggle
    auto ldmLabel = CCLabelBMFont::create(LOC_KEY("SETTINGS_LDM").c_str(), "goldFont.fnt");
    ldmLabel->setPosition({labelX, yPos});
    ldmLabel->setScale(0.5f);
    ldmLabel->setAnchorPoint({0, 0.5f});
    m_mainLayer->addChild(ldmLabel);
    
    auto ldmOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    auto ldmOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    m_ldmToggle = CCMenuItemToggler::create(ldmOff, ldmOn, this, menu_selector(SettingsLayer::onToggleLDM));
    m_ldmToggle->setPosition({toggleX, yPos});
    m_ldmToggle->setScale(0.8f);
    m_ldmToggle->toggle(Mod::get()->getSettingValue<bool>("ldm"));
    menu->addChild(m_ldmToggle);
    
    yPos -= 45;
    
    // Shake Toggle
    auto shakeLabel = CCLabelBMFont::create(LOC_KEY("SETTINGS_SHAKE").c_str(), "goldFont.fnt");
    shakeLabel->setPosition({labelX, yPos});
    shakeLabel->setScale(0.5f);
    shakeLabel->setAnchorPoint({0, 0.5f});
    m_mainLayer->addChild(shakeLabel);
    
    auto shakeOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    auto shakeOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    m_shakeToggle = CCMenuItemToggler::create(shakeOff, shakeOn, this, menu_selector(SettingsLayer::onToggleShake));
    m_shakeToggle->setPosition({toggleX, yPos});
    m_shakeToggle->setScale(0.8f);
    // Note: L'option "disable-shake" est inversée (true = désactivé)
    m_shakeToggle->toggle(!Mod::get()->getSettingValue<bool>("disable-shake"));
    menu->addChild(m_shakeToggle);
    
    yPos -= 45;
    
    // Language Selector
    auto langLabel = CCLabelBMFont::create(LOC_KEY("SETTINGS_LANGUAGE").c_str(), "goldFont.fnt");
    langLabel->setPosition({labelX, yPos});
    langLabel->setScale(0.5f);
    langLabel->setAnchorPoint({0, 0.5f});
    m_mainLayer->addChild(langLabel);
    
    // Trouver l'index de la langue actuelle
    auto currentLang = Localization::get()->getCurrentLanguage();
    for (size_t i = 0; i < m_languages.size(); i++) {
        if (m_languages[i] == currentLang) {
            m_selectedLanguageIndex = i;
            break;
        }
    }
    
    // Flèches de navigation
    auto leftArrow = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
    leftArrow->setScale(0.6f);
    auto leftBtn = CCMenuItemSpriteExtra::create(leftArrow, this, menu_selector(SettingsLayer::onLanguageChange));
    leftBtn->setPosition({toggleX - 50, yPos});
    leftBtn->setTag(-1); // Direction gauche
    menu->addChild(leftBtn);
    
    m_languageLabel = CCLabelBMFont::create(m_languageNames[m_selectedLanguageIndex].c_str(), "chatFont.fnt");
    m_languageLabel->setPosition({toggleX, yPos});
    m_languageLabel->setScale(0.6f);
    m_mainLayer->addChild(m_languageLabel);
    
    auto rightArrow = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
    rightArrow->setScale(0.6f);
    auto rightBtn = CCMenuItemSpriteExtra::create(rightArrow, this, menu_selector(SettingsLayer::onLanguageChange));
    rightBtn->setPosition({toggleX + 50, yPos});
    rightBtn->setTag(1); // Direction droite
    menu->addChild(rightBtn);
    
    // Boutons Save / Cancel
    auto saveBtn = HoverButton::create(
        LOC_KEY("SETTINGS_SAVE"),
        this,
        menu_selector(SettingsLayer::onSave),
        "goldFont.fnt",
        0.6f
    );
    saveBtn->setPosition({center.x - 60, 40});
    saveBtn->setNormalColor({100, 255, 100});
    saveBtn->setHoverColor({150, 255, 150});
    menu->addChild(saveBtn);
    
    auto cancelBtn = HoverButton::create(
        LOC_KEY("SETTINGS_CANCEL"),
        this,
        menu_selector(SettingsLayer::onClose),
        "goldFont.fnt",
        0.6f
    );
    cancelBtn->setPosition({center.x + 60, 40});
    cancelBtn->setNormalColor({255, 100, 100});
    cancelBtn->setHoverColor({255, 150, 150});
    menu->addChild(cancelBtn);
    
    return true;
}
void SettingsLayer::onToggleLDM(CCObject*) {
    // Toggle géré automatiquement par CCMenuItemToggler
}
void SettingsLayer::onToggleShake(CCObject*) {
    // Toggle géré automatiquement par CCMenuItemToggler
}
void SettingsLayer::onLanguageChange(CCObject* sender) {
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    int direction = btn->getTag();
    
    m_selectedLanguageIndex += direction;
    
    // Wrap around
    if (m_selectedLanguageIndex < 0) {
        m_selectedLanguageIndex = m_languages.size() - 1;
    } else if (m_selectedLanguageIndex >= static_cast<int>(m_languages.size())) {
        m_selectedLanguageIndex = 0;
    }
    
    updateLanguageDisplay();
}
void SettingsLayer::updateLanguageDisplay() {
    m_languageLabel->setString(m_languageNames[m_selectedLanguageIndex].c_str());
    
    // Animation
    m_languageLabel->stopAllActions();
    m_languageLabel->setScale(0.8f);
    m_languageLabel->runAction(CCScaleTo::create(0.1f, 0.6f));
}
void SettingsLayer::onSave(CCObject*) {
    // Sauvegarder les paramètres
    Mod::get()->setSettingValue("ldm", m_ldmToggle->isToggled());
    Mod::get()->setSettingValue("disable-shake", !m_shakeToggle->isToggled());
    Mod::get()->setSettingValue("language", m_languages[m_selectedLanguageIndex]);
    
    // Recharger la langue
    Localization::get()->loadLanguage(m_languages[m_selectedLanguageIndex]);
    
    log::info("Settings saved");
    
    onClose(nullptr);
}
void SettingsLayer::onClose(CCObject* sender) {
    m_background->setPaused(true);
    Popup::onClose(sender);
}
