#pragma once
#include <Geode/Geode.hpp>
using namespace geode::prelude;
namespace Stargrind::UI {
class RGBBackground : public CCLayerColor {
public:
    static RGBBackground* create();
    bool init() override;
    void update(float dt) override;
    
    void setLerpSpeed(float speed) { m_lerpSpeed = speed; }
    void setPaused(bool paused) { m_paused = paused; }
    
private:
    float m_hue = 0.0f;
    float m_lerpSpeed = 0.5f;
    bool m_paused = false;
    
    ccColor3B hsvToRgb(float h, float s, float v);
};
