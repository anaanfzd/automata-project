#ifndef AUTOMATAVIZ_ANIMATION_H
#define AUTOMATAVIZ_ANIMATION_H

#include <functional>

namespace AutomataViz {

class AnimationController {
public:
    AnimationController();

    void update(float dtSeconds);

    void play();
    void pause();
    void togglePlayPause();
    void reset();

    bool isPlaying() const { return isAutoPlaying; }

    void setSpeed(float secondsPerStep);
    float getSpeed() const { return stepDuration; }

    void setOnStepTrigger(std::function<void()> callback) { onStepCallback = callback; }

    float getPulseAlpha() const; // Oscillates between 0.0f and 1.0f for pulsing highlights

private:
    bool isAutoPlaying;
    float stepDuration;
    float timeAccumulator;
    float pulseTime;
    std::function<void()> onStepCallback;
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_ANIMATION_H
