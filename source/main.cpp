
#include "stdio.h"
#include <vector>
#include <string>
#include "Effects/Effect.h"
#include "kylarLEDs/LEDInterface/LEDs.h"
#include "kylarLEDs/EffectEngine/EffectEngine.h"
#include "kylarLEDs/Controllers/FireFlyV2/FireFlyV2Controller.h"
#include "Patterns/Examples/ExamplePattern.h"
#include "Patterns/Examples/FireFlies.h"
#include "Patterns/Examples/FireFliesSame.h"
#include "Patterns/Wireless/WirelessPattern.h"
#include "Patterns/SoundReactive/Shakeel.h"
#include "Patterns/SoundReactive/Bounce.h"
#include "Patterns/SoundReactive/ShakeelFlash.h"
#include "Patterns/SoundReactive/SpaceX.h"
#include "Patterns/SoundReactive/FireV0.h"
#include "Patterns/SoundReactive/HeartPattern.h"
#include "Patterns/SoundReactive/CirclesPattern.h"
#include "Patterns/SoundReactive/Raindrop.h"
#include "kylarLEDs/Utility/ExecTimer.h"
#include <malloc.h>
#include "pico/time.h"
#include "config.h"
#include "pico/multicore.h"


using namespace std;
int main(){
    if(DEBUG_DELAY_MAIN) {
        sleep_ms(5000);
    }
    
    // Initialize framework infrastructure
    Controller *ledController = new FireFlyV2Controller();
    
    EffectEngine *effectEngine = new EffectEngine();
    LEDs::init(NUM_STRIPS); // Initializing # of outputs
    LEDs::setNum(NUM_LEDS); // Setting all strips to 120 LEDs
    
    Effect::giveEngine(effectEngine);
    
    vector<Pattern*> *patterns = new vector<Pattern*>();
    //Push back all the patterns you want!
    //ADD YOUR PATTERNS HERE!
    // Example pattern configurations
    BounceConfig config1 = {
        .solidBar = false,
        .splitBar = true,
        .highLowInput = false,
    };

    BounceConfig config2 = {
        .solidBar = false,
        .splitBar = true,
        .highLowInput = true,
    };
    BounceConfig config3 = {
        .solidBar = false,
        .splitBar = true,
        .highLowInput = true,
        .maxThickness = 3,
    };

    // Add patterns to the vector
    patterns->push_back(new Bounce(config1));
    patterns->push_back(new Bounce(config2));
    patterns->push_back(new Bounce(config3));
    // patterns->push_back(new Bounce(true));
    // patterns->push_back(new Shakeel());
    // patterns->push_back(new FireV0(false));
    // patterns->push_back(new FireV0(true));
    // patterns->push_back(new Raindrop());
    // patterns->push_back(new SpaceX());
    // patterns->push_back(new HeartPattern());
    // patterns->push_back(new CirclesPattern());
    // patterns->push_back(new ShakeelFlash());


    //Initialize main loop variables
    uint32_t numPatterns = patterns->size();
    uint32_t currentPatternIndex = 0;
    uint32_t nextPatternIndex = 0;
    Pattern *currentPattern = patterns->at(currentPatternIndex);
    Pattern *nextPattern = patterns->at(nextPatternIndex);
    
    //Give the ledController access to the nextPatternIndex
    //Thus it can write to it when its button is pressed and change the pattern
    ledController->givePatternIndex(&nextPatternIndex);

    //Give the LEDStrips the controller
    //This way the LEDStrip can call the output function to the controller
    //The LEDStrip can also ask the controller for hue/brightness info
    //Which is based on the sensors the controller accesses
    LEDs::giveController(ledController);
    Timing::giveControllerForTiming(ledController);

    //Initialize first pattern
    currentPattern->init();

    ExecTimer *timer = new ExecTimer();
    // multicore_lockout_victim_init();        // This tells core0 to stop when data flashing on Core1 starts
    //Main loop
    while(1){
        if(DEBUG_PRINT_MAIN){
             // mem usage:
            struct mallinfo mi = mallinfo();
            printf("Total allocated space (bytes):  %d\n", mi.uordblks); // max is about 238516 bytes ( unless there is ghost memory )~
            printf("Total free space (bytes):       %d\n", mi.fordblks); // Free space left (in the current block?)
        }
        if(currentPatternIndex == nextPatternIndex){
            //We are remaining on the same pattern
            timer->start("mainloop");
            currentPattern->run();  // Allow pattern to create effects
            timer->add("currentPattern->run()");
            effectEngine->run();    // Run each effect to generate LED Changes
            timer->add("effectEngine->run()");
            LEDs::clear();          // Clear LEDs between runs
            timer->add("LEDs::clear()");
            LEDs::apply();          // Apply changes by collapsing LED Changes
            timer->add("LEDs::apply()");
            LEDs::output();         // Output to strip via controller
            timer->add("LEDs::output()");
            if(DEBUG_PRINT_MAIN) timer->print();
        }else{
            //We are changing pattern
            currentPattern->release();                      //Finish the current pattern
            effectEngine->clear();                          //Clear the effects
            LEDs::clear();                                  //Clear the LEDs
            LEDs::output();                                 //Output the off LEDs
            LEDs::useGlobalBrightnessControl(false, NULL);  //Clear any global brightness control
            nextPatternIndex %= numPatterns;                //Protect from out of bounds
            nextPattern = patterns->at(nextPatternIndex);   //Get the next pattern
            currentPattern = nextPattern;                   //Set the current pattern to be the next
            currentPatternIndex = nextPatternIndex;         //Set current pattern index to the new one
            currentPattern->init();                         //Init the new current pattern
        }
    }
    

    return 0;
}
