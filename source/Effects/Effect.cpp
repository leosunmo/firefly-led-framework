#include "Effect.h"
#include "stdio.h"

EffectEngine* Effect::engine;
Effect::Effect(){
    
}

Effect::~Effect(){
    
}

bool Effect::isDone(){
    return this->done;
}

//static
void Effect::giveEngine(EffectEngine * effectEngine){
    Effect::engine = effectEngine;
}