#include "RGBBackground.hpp"
#include "../../core/Config.hpp"
namespace Stargrind::UI {
RGBBackground* RGBBackground::create() {
    auto ret = new RGBBackground();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}
bool RGBBackground::init() {
    if (!CCLayerColor::initWithColor({30, 30, 50, 255})) return false;
    
    m_lerpSpeed = Config::RGB_LERP_SPEED;
    scheduleUpdate();
    
    return true;
}
void RGBBackground::update(float dt) {
    if (m_paused) return;
    
    // Incrémenter la teinte lentement
    m_hue += dt * m_lerpSpeed * 10.0f;
    if (m_hue >= 360.0f) m_hue -= 360.0f;
    
    // Convertir HSV -> RGB avec saturation et valeur basses pour un effet subtil
    auto color = hsvToRgb(m_hue, 0.3f, 0.15f);
    setColor(color);
}
ccColor3B RGBBackground::hsvToRgb(float h, float s, float v) {
    float c = v * s;
    float x = c * (1 - std::abs(std::fmod(h / 60.0f, 2) - 1));
    float m = v - c;
    
    float r, g, b;
    
    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }
    
    return {
        static_cast<GLubyte>((r + m) * 255),
        static_cast<GLubyte>((g + m) * 255),
        static_cast<GLubyte>((b + m) * 255)
    };
}
} // namespace Stargrind::UI