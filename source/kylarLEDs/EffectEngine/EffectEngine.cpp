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

void EffectEngine::clearPattern(uint8_t pattern_id) {
    for (Effect* eff : effects) {
        if (eff->ID == pattern_id) {
            delete(eff);
        }
    }
    for (Effect* eff : effectsQueue){
        if (eff->ID == pattern_id) {
            delete(eff);
        }
    }
    effects.clear();
    effectsQueue.clear();
}

void EffectEngine::setActivePattern(uint8_t patternIndex) {
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
        if (effect->isDone()) {
            // erase() invalidates the iterator, use returned iterator
            delete effect;
            //delete(effect);
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