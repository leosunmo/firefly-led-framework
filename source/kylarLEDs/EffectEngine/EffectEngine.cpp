#include "EffectEngine.h"
#include <stdio.h>
#include <cstdlib>
EffectEngine::EffectEngine(){

}

void EffectEngine::clear() {
    for (Effect* eff : effects) {
        delete(eff);
    }
    for (Effect* eff : effectsQueue){
        delete(eff);
    }
    effects.clear();
    effectsQueue.clear();
}

void EffectEngine::clearPattern(uint32_t pattern_id) {
    std::vector<Effect *>::iterator it = effects.begin();
    while (it != effects.end()) {
        if ((*it)->ID == pattern_id) {
            delete *it;
            it = effects.erase(it);
        } else {
            ++it;
        }
    }

    it = effectsQueue.begin();
    while (it != effectsQueue.end()) {
        if ((*it)->ID == pattern_id) {
            delete *it;
            it = effectsQueue.erase(it);
        } else {
            ++it;
        }
    }
}

void EffectEngine::setActivePattern(uint32_t patternIndex) {
    activePattern = patternIndex;
}

void EffectEngine::apply(Effect *effect) {
    effect->ID = activePattern;
    effects.push_back(effect);
}

void EffectEngine::queueApply(Effect *effect) {
    effect->ID = activePattern;
    effectsQueue.push_back(effect);
}

void EffectEngine::run() {
    // This function essentially just calls run on every effect
    // It will also remove the effects that are "done"
    std::vector<Effect *>::iterator it = effects.begin();
    while (it != effects.end())
    {
        Effect* effect = *it;
        printf("EffectEngine::run() - Processing Effect with ID: %d\n", effect->ID);
        if (effect->isDone()) {
            printf("EffectEngine::run() - Deleting Effect with ID: %d\n", effect->ID);
            delete effect;
            it = effects.erase(it);
            continue;
        } else {
            effect->run();
            it++;
        }
    }
    for (Effect* eff : effectsQueue) {
        effects.push_back(eff);
    }
    effectsQueue.clear();
}