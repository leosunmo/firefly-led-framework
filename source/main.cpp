#include "stdio.h"
#include <vector>
#include <string>
#include <map>
#include "Effects/Effect.h"
#include "kylarLEDs/LEDInterface/LEDs.h"
#include "kylarLEDs/EffectEngine/EffectEngine.h"
#include "kylarLEDs/Controllers/FireFlyV2/FireFlyV2Controller.h"
#include "kylarLEDs/Communication/UARTManager.h"
#include "Patterns/PatternTypes.h" 
// #include "Patterns/Examples/ExamplePattern.h"
// #include "Patterns/Examples/FireFlies.h"
// #include "Patterns/Examples/FireFliesSame.h"
// #include "Patterns/Wireless/WirelessPattern.h"
#include "Patterns/SoundReactive/Shakeel.h"
#include "Patterns/SoundReactive/Bounce.h"
#include "Patterns/SoundReactive/ShakeelFlash.h"
#include "Patterns/SoundReactive/ShakeelFlashBall.h"
// #include "Patterns/SoundReactive/SpaceX.h"
// #include "Patterns/SoundReactive/FireV0.h"
// #include "Patterns/SoundReactive/HeartPattern.h"
// #include "Patterns/SoundReactive/CirclesPattern.h"
// #include "Patterns/SoundReactive/Raindrop.h"
#include "Patterns/SoundReactive/ChromaWave.h"
#include "kylarLEDs/Utility/ExecTimer.h"
#include <malloc.h>
#include "pico/time.h"
#include "config.h"
#include "pico/multicore.h"


using namespace std;
int main(){
    stdio_init_all();

    if(DEBUG_DELAY_MAIN) {
        sleep_ms(5000); // Delay for 5 seconds to allow for serial connection
    }

    // Initialize framework infrastructure
    Controller *ledController = new FireFlyV2Controller();

    // Initialize the effect engine
    EffectEngine *effectEngine = new EffectEngine();
    LEDs::init(NUM_STRIPS); // Initializing # of outputs
    LEDs::setNum(NUM_LEDS); // Setting all strips to 120 LEDs
    
    Effect::giveEngine(effectEngine);
    
    // Map for direct access to patterns by ID
    std::map<uint8_t, Pattern*> *patternMap = new std::map<uint8_t, Pattern*>();
    
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

    // Define and register patterns with the map

    // ChromaWave - ID 1
    Pattern* chromaWave = new ChromaWave();
    patternMap->insert({static_cast<uint8_t>(FireFly::PatternType::CHROMA_WAVE), chromaWave});

    // ShakeelFlash - ID 2
    Pattern* shakeelFlash = new ShakeelFlash();
    patternMap->insert({static_cast<uint8_t>(FireFly::PatternType::SHAKEEL_FLASH), shakeelFlash});
    
    // ShakeelFlashBall - ID 3
    Pattern* shakeelFlashBall = new ShakeelFlashBall();
    patternMap->insert({static_cast<uint8_t>(FireFly::PatternType::SHAKEEL_FLASH_BALL), shakeelFlashBall});
    
    // Default to the first pattern in the map if available, otherwise use SHAKEEL_FLASH
    uint8_t defaultPatternId = static_cast<uint8_t>(FireFly::PatternType::SHAKEEL_FLASH);
    
    // Make sure the pattern map contains the default pattern
    if (patternMap->find(defaultPatternId) == patternMap->end() && !patternMap->empty()) {
        // If default pattern ID isn't in the map, use the first available pattern
        defaultPatternId = patternMap->begin()->first;
        printf("Default pattern ID not found, using first available ID: %d\n", defaultPatternId);
    }
    
    uint8_t currentPatternId = defaultPatternId;
    uint8_t nextPatternId = currentPatternId;
    
    // Safety check before accessing the pattern
    if (patternMap->find(currentPatternId) == patternMap->end()) {
        printf("ERROR: Initial pattern ID %d not found in pattern map\n", currentPatternId);
        return 1; // Exit with error
    }
    
    Pattern *currentPattern = patternMap->at(currentPatternId); // Get pattern directly by ID
    Pattern *nextPattern = currentPattern;
    
    // Print available pattern IDs for reference
    printf("Available pattern IDs:\n");
    for (auto& pair : *patternMap) {
        printf("  ID %d: %s\n", pair.first, FireFly::getPatternName(static_cast<FireFly::PatternType>(pair.first)));
    }
    
    // Give the controller access to the pattern selection variables
    FireFlyV2Controller* ffController = static_cast<FireFlyV2Controller*>(ledController);
    ffController->givePatternMap(patternMap, &nextPatternId);

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
        // Process any UART input at the start of each loop iteration
        FireFly::UARTManager::getInstance().processInput();
        
        if(DEBUG_PRINT_MAIN){
             // mem usage:
            struct mallinfo mi = mallinfo();
            printf("Total allocated space (bytes):  %d\n", mi.uordblks); // max is about 238516 bytes ( unless there is ghost memory )~
            printf("Total free space (bytes):       %d\n", mi.fordblks); // Free space left (in the current block?)
        }
        if(DEBUG_PRINT_MEMORY){
            // Print memory usage
            struct mallinfo mi = mallinfo();
            printf("Total allocated space (bytes):  %d\n", mi.uordblks); // max is about 238516 bytes ( unless there is ghost memory )~
            printf("total space allocated from system (bytes):       %d\n", mi.arena); // Free space left (in the current block?)

        }
        
        // Check if pattern ID has changed
        if(currentPatternId != nextPatternId){
            // Pattern ID has changed, switch to the new pattern
            currentPattern->release();                      // Finish the current pattern
            effectEngine->clear();                          // Clear the effects
            LEDs::clear();                                  // Clear the LEDs
            LEDs::output();                                 // Output the off LEDs
            LEDs::useGlobalBrightnessControl(false, NULL);  // Clear any global brightness control
            
            // Check if the requested pattern exists in the map
            if(patternMap->count(nextPatternId) > 0) {
                printf("Switching from pattern ID %d to pattern ID %d\n", 
                       currentPatternId, nextPatternId);
                
                nextPattern = patternMap->at(nextPatternId); // Get pattern directly by ID
                currentPattern = nextPattern;               // Set the current pattern to be the next
                currentPatternId = nextPatternId;           // Update current pattern ID
                
                currentPattern->init();                     // Init the new current pattern
            } else {
                // Pattern ID not found, fall back to the current pattern
                printf("Warning: Pattern ID %d not found in pattern map\n", nextPatternId);
                nextPatternId = currentPatternId;           // Reset to current pattern ID
            }
        } else {
            // No pattern change, continue running current pattern
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
        }
    }
    

    return 0;
}
