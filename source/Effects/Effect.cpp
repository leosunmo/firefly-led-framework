#include "Effect.h"
#include "stdio.h"

EffectEngine* Effect::engine;
Effect::Effect(){
    
}

Effect::~Effect(){
    printf("Effect::~Effect() - Deleting Effect with ID: %d\n", ID);
}

bool Effect::isDone(){
    return this->done;
}

//static
void Effect::giveEngine(EffectEngine * effectEngine){
    Effect::engine = effectEngine;
}