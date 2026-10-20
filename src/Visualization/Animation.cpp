#include "Visualization/Animation.h"
#include <cmath>

namespace AutomataViz {

AnimationController::AnimationController()
    : isAutoPlaying(false), stepDuration(1.2f), timeAccumulator(0.0f), pulseTime(0.0f) {}

void AnimationController::update(float dtSeconds) {
    pulseTime += dtSeconds * 3.0f;

    if (isAutoPlaying) {
        timeAccumulator += dtSeconds;
        if (timeAccumulator >= stepDuration) {
            timeAccumulator = 0.0f;
            if (onStepCallback) {
                onStepCallback();
            }
        }
    }
}

void AnimationController::play() {
    isAutoPlaying = true;
    timeAccumulator = 0.0f;
}

void AnimationController::pause() {
    isAutoPlaying = false;
}

void AnimationController::togglePlayPause() {
    isAutoPlaying = !isAutoPlaying;
    if (isAutoPlaying) {
        timeAccumulator = 0.0f;
    }
}

void AnimationController::reset() {
    isAutoPlaying = false;
    timeAccumulator = 0.0f;
}

void AnimationController::setSpeed(float secondsPerStep) {
    if (secondsPerStep > 0.05f) {
        stepDuration = secondsPerStep;
    }
}

float AnimationController::getPulseAlpha() const {
    return 0.5f + 0.5f * std::sin(pulseTime);
}

} // namespace AutomataViz
