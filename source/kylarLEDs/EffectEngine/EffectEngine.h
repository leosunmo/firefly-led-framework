#pragma once
#include "../../Effects/Effect.h"
#include <vector>
class Effect;
class EffectEngine{
    /*
        • Manage all effects
        • Combine every loop
        • Output using LED output
    */
    public:
        EffectEngine();
        void run();
        void clear();
        void clearPattern(uint32_t pattern_id);
        void apply(Effect *effect);
        void queueApply(Effect* effect);
        void setActivePattern(uint32_t patternIndex);
        std::vector<Effect *> effects;
	    std::vector<Effect *> effectsQueue;
    private:
        uint32_t activePattern;
};