#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "Components/RGBBackground.hpp"
using namespace geode::prelude;
namespace Stargrind::UI {
class LobbyLayer : public Popup<> {
protected:
    bool setup() override;
    void onClose(CCObject*) override;
    void update(float dt) override;
    
    void onCancel(CCObject*);
    void updateQueueCount(int count);
    void onMatchFound();
    
    RGBBackground* m_background = nullptr;
    CCLabelBMFont* m_countLabel = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    CCNode* m_loadingCircle = nullptr;
    
    bool m_searching = false;
    float m_pollTimer = 0;
    
public:
    static LobbyLayer* create();
};
} // namespace Stargrind::UI