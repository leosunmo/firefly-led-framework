#include "ChromaWave.h"
#include <math.h>

/**
 * ==================================================================
 * ChromaWave Pattern - Parameter and Mode Reference
 * ==================================================================
 * 
 * OVERVIEW
 * --------
 * ChromaWave is a sound-reactive LED pattern that uses the TwoTone
 * effect to blend colors based on audio input. It persists settings
 * between instances and provides a user interface through the
 * FireFly input system.
 * 
 * INTERACTIVE PARAMETERS
 * ---------------------
 * hue1: double (0.0-1.0) - Base color shown when audio is quiet
 *       Controlled via HUE event with index=0
 *       Persist between pattern instances
 * 
 * hue2: double (0.0-1.0) - Reactive color mixed in with increasing audio
 *       Controlled via HUE event with index=1
 *       Persist between pattern instances
 * 
 * decayRate: float (0.5-1.0) - Controls how quickly effect fades
 *       Controlled via SPEED event (0-100 mapped to 0.5-1.0)
 *       Higher values = slower fade
 *       Persist between pattern instances
 * 
 * PERSISTED SETTINGS
 * -----------------
 * attackRate: How quickly the effect responds to audio peaks (0.01-0.99)
 *       Default: 0.4 (lower = faster attack) 
 * 
 * frequencyBand: Which frequency band to respond to (0-3)
 *       0=LOW (bass), 1=MID, 2=HIGH, 3=FULL spectrum
 *       Default: 0 (LOW)
 * 
 * audioCurve: How audio is processed (0-3)
 *       0=LINEAR, 1=SQUARE, 2=CUBIC, 3=LOGARITHMIC
 *       Default: 1 (SQUARE)
 * 
 * saturation: Color saturation (0.0-1.0)
 *       Default: 1.0 (full saturation)
 * 
 * audioThreshold: Minimum audio level to trigger effects (0.0-0.5)
 *       Default: 0.01
 * 
 * beatSensitivity: How sensitive beat detection is (0.5-5.0)
 *       Default: 1.5
 * 
 * beatReaction: How strongly visuals react to beats (0.0-1.0)
 *       Default: 0.8
 * 
 * visualizationMode: Which visualization to use (0-3)
 *       0=COLOR_PULSE, 1=BEAT_FLASH, 2=BEAT_EXPAND, 3=SPECTRUM_FLOW
 *       Default: 0 (COLOR_PULSE)
 * 
 * rgbMode: Whether to use RGB direct interpolation (true/false)
 *       Default: true (enabled for smoother color transitions)
 * ==================================================================
 */

// Initialize the static settings with default values
ChromaWaveSettings ChromaWave::persistentSettings;

ChromaWave::ChromaWave() : twoToneEffect(nullptr)
{
    // Load settings from persistent storage
    loadSettings();
}

// Load settings from persistent storage to local variables
void ChromaWave::loadSettings()
{
    hue1 = persistentSettings.hue1;
    hue2 = persistentSettings.hue2;
    attackRate = persistentSettings.attackRate;
    decayRate = persistentSettings.decayRate;
    frequencyBand = persistentSettings.frequencyBand;
    audioCurve = persistentSettings.audioCurve;
    saturation = persistentSettings.saturation;
    audioThreshold = persistentSettings.audioThreshold;
    beatSensitivity = persistentSettings.beatSensitivity;
    beatReaction = persistentSettings.beatReaction;
    visualizationMode = persistentSettings.visualizationMode;
    rgbMode = persistentSettings.rgbMode;
}

void ChromaWave::init()
{
    // Ensure local variables are loaded from persistent settings
    loadSettings();
    
    // Create a new TwoTone effect
    twoToneEffect = new TwoTone();
    
    // Initialize the TwoTone effect
    twoToneEffect->init();
    
    // Apply the effect to the engine
    Effect::engine->apply(twoToneEffect);
    
    // Apply settings from persistent storage
    twoToneEffect->setHues(hue1, hue2);
    
    // Initialize audio processing parameters with persisted values
    twoToneEffect->setAttackRate(attackRate);
    twoToneEffect->setDecayRate(decayRate);
    twoToneEffect->setFrequencyBand(static_cast<FrequencyBand>(frequencyBand));
    twoToneEffect->setAudioCurve(static_cast<AudioCurve>(audioCurve));
    twoToneEffect->setSaturation(saturation);
    twoToneEffect->setAudioThreshold(audioThreshold);
    
    // Initialize beat detection parameters with persisted values
    twoToneEffect->setBeatSensitivity(beatSensitivity);
    twoToneEffect->setBeatReaction(beatReaction);
    
    // Set visualization mode from persisted value
    VisualizationMode mode;
    switch(visualizationMode) {
        case 1: mode = VisualizationMode::BEAT_FLASH; break;
        case 2: mode = VisualizationMode::BEAT_EXPAND; break;
        case 3: mode = VisualizationMode::SPECTRUM_FLOW; break;
        default: mode = VisualizationMode::COLOR_PULSE; break;
    }
    twoToneEffect->setVisualizationMode(mode);
    
    // Set color interpolation mode
    twoToneEffect->enableRGBMode(rgbMode);
    
    // Set up input manager subscriptions
    auto& inputManager = FireFly::InputManager::getInstance();
    
    // Subscribe to HUE events
    inputManager.subscribe(FireFly::InputEventType::HUE, [this](const FireFly::InputEvent &event) {
        if (event.type == FireFly::InputType::VALUE_CHANGE) {
            // Convert the 0-360 degree value to 0-1 range
            double hueValue = event.value / 360.0;
            // Keep hue in 0-1 range
            if (hueValue > 1.0) hueValue = 1.0;
            if (hueValue < 0.0) hueValue = 0.0;
            
            // Check the hue index to determine which hue to set
            if (event.index == 0) {
                // Index 0 is for primary hue
                printf("ChromaWave - Setting primary hue: %f (index=%d)\n", hueValue, event.index);
                this->setHue1(hueValue);
            } else if (event.index == 1) {
                // Index 1 is for secondary hue
                printf("ChromaWave - Setting secondary hue: %f (index=%d)\n", hueValue, event.index);
                this->setHue2(hueValue);
            } else {
                printf("ChromaWave - Unknown hue index: %d\n", event.index);
            }
        }
    });

    // Subscribe to SPEED events for decay rate control
    inputManager.subscribe(FireFly::InputEventType::SPEED, [this](const FireFly::InputEvent &event) {
        if (event.type == FireFly::InputType::VALUE_CHANGE) {
            // Speed is a 0-100 value, map to 0.5 - 1.0
            float decay = event.value / 100.0f; // Normalize to 0-1
            printf("ChromaWave - Setting decay: %f\n", decay);
            this->setDecayRate(decay);
        }
    });

        // if (event.type == FireFly::InputType::VALUE_CHANGE) {
        //     // Speed is a 0-100 value, map to 0.1 - 1.0
        //     float speed = event.value / 1000.0f; // Normalize to 0-1
        //     printf("ChromaWave - Setting speed: %f\n", speed);
        //     this->setSpeed(speed);
        // }
}

void ChromaWave::run()
{
    // Safety check
    if (!twoToneEffect) {
        return;
    }
    
    // Get audio levels from the microphone
    // We'll pass the raw audio to the effect, and let it process based on its settings
    double lowFreq = Microphone::getLowNormal();
    
    // Feed the raw microphone value to the effect
    // The TwoTone effect will process it based on frequency band and curve settings
    twoToneEffect->micVal = lowFreq;
}

void ChromaWave::setHue1(double h)
{
    // First hue is the base color
    hue1 = h;
    // Update persistent settings
    persistentSettings.hue1 = h;
    persistentSettings.userModified = true;
    
    if (twoToneEffect)
    {
        twoToneEffect->setHue1(hue1);
    }
}

void ChromaWave::setHue2(double h)
{
    // Second hue is the reactive color
    hue2 = h;
    // Update persistent settings
    persistentSettings.hue2 = h;
    persistentSettings.userModified = true;
    
    if (twoToneEffect)
    {
        twoToneEffect->setHue2(hue2);
    }
}

void ChromaWave::setBrightness(float b)
{
    // Note: We don't have a local brightness variable, it's managed by TwoTone
    // But we'll mark settings as modified
    persistentSettings.userModified = true;
    
    if (twoToneEffect)
    {
        twoToneEffect->baseBrightness = b;
    }
}

void ChromaWave::setSpeed(float speed)
{
    // Map the speed parameter to audio reactivity
    // Higher speed = more color change for the same audio level
    if (twoToneEffect)
    {
        // Map speed (typically 0-1) to reactivity (1.0 - 10.0)
        float reactivity = 1.0f + (speed * 9.0f);
        twoToneEffect->setReactivity(reactivity);
        // We don't persist this directly as it's handled through the reactivity setting
    }
}

void ChromaWave::setReactivity(float value)
{
    // Note: We don't have a local reactivity variable, it's managed by TwoTone
    // But we'll use persistentSettings to track the value for brightness
    persistentSettings.userModified = true;
    
    if (twoToneEffect)
    {
        twoToneEffect->setReactivity(value);
    }
}

// Audio processing parameters

void ChromaWave::setAttackRate(float value)
{
    // Store locally for initialization
    attackRate = value;
    // Update persistent settings
    persistentSettings.attackRate = value;
    persistentSettings.userModified = true;
    
    if (twoToneEffect)
    {
        twoToneEffect->setAttackRate(value);
    }
}

void ChromaWave::setDecayRate(float value)
{
    // Store locally for initialization
    decayRate = value;
    // Update persistent settings
    persistentSettings.decayRate = value;
    persistentSettings.userModified = true;
    
    if (twoToneEffect)
    {
        twoToneEffect->setDecayRate(value);
    }
}

void ChromaWave::setFrequencyBand(int bandIndex)
{
    // Store locally for initialization
    frequencyBand = bandIndex;
    // Update persistent settings
    persistentSettings.frequencyBand = bandIndex;
    persistentSettings.userModified = true;
    
    if (twoToneEffect)
    {
        // Convert int to enum: 0=LOW, 1=MID, 2=HIGH, 3=FULL
        // NOTE: Currently LOW and HIGH bands are implemented
        // MID band uses getLowNormal() until getMidNormal() is implemented
        FrequencyBand band;
        switch(bandIndex) {
            case 1: band = FrequencyBand::MID; break;
            case 2: band = FrequencyBand::HIGH; break;
            case 3: band = FrequencyBand::FULL; break;
            default: band = FrequencyBand::LOW; break;
        }
        twoToneEffect->setFrequencyBand(band);
    }
}

void ChromaWave::setAudioCurve(int curveIndex)
{
    // Store locally for initialization
    audioCurve = curveIndex;
    // Update persistent settings
    persistentSettings.audioCurve = curveIndex;
    persistentSettings.userModified = true;
    
    if (twoToneEffect)
    {
        // Convert int to enum: 0=LINEAR, 1=SQUARE, 2=CUBIC, 3=LOGARITHMIC
        AudioCurve curve;
        switch(curveIndex) {
            case 0: curve = AudioCurve::LINEAR; break;
            case 1: curve = AudioCurve::SQUARE; break;
            case 2: curve = AudioCurve::CUBIC; break;
            case 3: curve = AudioCurve::LOGARITHMIC; break;
            default: curve = AudioCurve::SQUARE; break;
        }
        twoToneEffect->setAudioCurve(curve);
    }
}

void ChromaWave::setSaturation(float value)
{
    // Store locally for initialization
    saturation = value;
    // Update persistent settings
    persistentSettings.saturation = value;
    persistentSettings.userModified = true;
    
    if (twoToneEffect)
    {
        twoToneEffect->setSaturation(value);
    }
}

void ChromaWave::setAudioThreshold(float value)
{
    // Store locally for initialization
    audioThreshold = value;
    // Update persistent settings
    persistentSettings.audioThreshold = value;
    persistentSettings.userModified = true;
    
    if (twoToneEffect)
    {
        twoToneEffect->setAudioThreshold(value);
    }
}

// Beat detection parameters

void ChromaWave::setBeatSensitivity(float value)
{
    // Store locally for initialization
    beatSensitivity = value;
    // Update persistent settings
    persistentSettings.beatSensitivity = value;
    persistentSettings.userModified = true;
    
    if (twoToneEffect)
    {
        twoToneEffect->setBeatSensitivity(value);
    }
}

void ChromaWave::setBeatReaction(float value)
{
    // Store locally for initialization
    beatReaction = value;
    // Update persistent settings
    persistentSettings.beatReaction = value;
    persistentSettings.userModified = true;
    
    if (twoToneEffect)
    {
        twoToneEffect->setBeatReaction(value);
    }
}

void ChromaWave::setVisualizationMode(int modeIndex)
{
    // Store locally for initialization
    visualizationMode = modeIndex;
    // Update persistent settings
    persistentSettings.visualizationMode = modeIndex;
    persistentSettings.userModified = true;
    
    if (twoToneEffect)
    {
        // Convert int to enum: 0=COLOR_PULSE, 1=BEAT_FLASH, 2=BEAT_EXPAND, 3=SPECTRUM_FLOW
        VisualizationMode mode;
        switch(modeIndex) {
            case 1: mode = VisualizationMode::BEAT_FLASH; break;
            case 2: mode = VisualizationMode::BEAT_EXPAND; break;
            case 3: mode = VisualizationMode::SPECTRUM_FLOW; break;
            default: mode = VisualizationMode::COLOR_PULSE; break;
        }
        twoToneEffect->setVisualizationMode(mode);
    }
}

bool ChromaWave::isBeatDetected()
{
    if (twoToneEffect)
    {
        return twoToneEffect->isBeatDetected();
    }
    return false;
}

const char* ChromaWave::getName()
{
    return "ChromaWave";
}

void ChromaWave::release()
{
    // Set the twoToneEffect pointer to null after the effect is deleted by the EffectEngine
    // This prevents us from trying to access the deleted effect if we're reinitialized
    twoToneEffect = nullptr;
    auto& inputManager = FireFly::InputManager::getInstance();
    inputManager.unsubscribeAll(FireFly::InputEventType::HUE);
    inputManager.unsubscribeAll(FireFly::InputEventType::SPEED);
}

// Toggle RGB interpolation mode
void ChromaWave::setRGBMode(bool enabled)
{
    rgbMode = enabled;
    
    // Update persistent settings
    persistentSettings.rgbMode = enabled;
    persistentSettings.userModified = true;
    
    // Update TwoTone effect if initialized
    if (twoToneEffect)
    {
        twoToneEffect->enableRGBMode(enabled);
    }
    
    printf("ChromaWave: %s RGB interpolation mode\n", enabled ? "Enabled" : "Disabled");
}
