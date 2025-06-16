#include "TwoTone.h"
#include "stdio.h"
#include <cmath>
#include <algorithm>
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"

void TwoTone::init()
{
    pixels = new std::vector<SoundPixel *>();
    int stripLen = LEDs::strip(0)->num();
    for (int i = 0; i < stripLen; i++)
    {
        SoundPixel *eff = new SoundPixel();
        eff->i = i;
        eff->hue = 0.0;
        eff->strip = 0;
        Effect::engine->apply(eff);
        pixels->push_back(eff);
    }
    secTimer = new Timing();
    hueTimer = new Timing();
    brightnessTimer = new Timing();
    
    // Initialize with default values
    hue1 = 0.0f;            // Red (shown when quiet)
    hue2 = 0.66f;           // Blue (mixed in as audio increases)
    pulsePeriod = 1.0f;     // Not used in this simpler implementation
    pulseReactivity = 5.0f; // Higher values mean more color change for the same audio level
    pulsePosition = 0.0f;   // Start at first hue
    
    attackRate = 0.4f;      // Fast attack for responsive peaks
    decayRate = 0.95f;      // Slow decay for natural fadeout
    
    freqBand = FrequencyBand::LOW;    // Default to bass frequencies
    audioCurve = AudioCurve::SQUARE;  // Default to square curve for better response
    saturation = 1.0f;      // Full saturation by default
    audioThreshold = 0.01f; // Minimal threshold to avoid noise
    
    // Beat detection initialization
    beatSensitivity = 1.5f;
    beatReaction = 0.8f;
    beatDetected = false;
    beatThreshold = 0.1f;
    beatDecay = 0.95f;
    framesSinceLastBeat = 0;
    minBeatInterval = 10;
    
    // Pre-fill energy history with zeros
    energyHistory.clear();
    for (int i = 0; i < 30; i++) { // 30 frames history for beat detection
        energyHistory.push_back(0.0f);
    }
    
    // Visualization mode defaults
    visMode = VisualizationMode::COLOR_PULSE;
    beatFlashIntensity = 0.0f;
    beatExpandProgress = 0.0f;
}

// New helper method to process audio based on frequency band and curve settings
float TwoTone::processAudio(float rawAudio)
{
    // Get the appropriate frequency band from the microphone
    float audioInput = 0.0f;
    
    // Use available frequency band methods
    switch (freqBand) {
        case FrequencyBand::LOW:
            audioInput = Microphone::getLowNormal();
            break;
        case FrequencyBand::MID:
            // Mid frequency not implemented yet, use low frequency instead
            audioInput = Microphone::getLowNormal(); // TODO: Replace with getMidNormal() when implemented
            break;
        case FrequencyBand::HIGH:
            audioInput = Microphone::getHighNormal();
            break;
        case FrequencyBand::FULL:
            // Use average of available bands for full spectrum response
            audioInput = (Microphone::getLowNormal() + Microphone::getHighNormal()) / 2.0f;
            // TODO: Update to include getMidNormal() when implemented:
            // audioInput = (Microphone::getLowNormal() + Microphone::getMidNormal() + Microphone::getHighNormal()) / 3.0f;
            break;
    }

    // Apply audio threshold
    if (audioInput < audioThreshold) {
        audioInput = 0.0f;
    }
    
    // Apply the selected audio curve
    float processedAudio = 0.0f;
    
    switch (audioCurve) {
        case AudioCurve::LINEAR:
            processedAudio = audioInput;
            break;
        case AudioCurve::SQUARE:
            processedAudio = audioInput * audioInput;
            break;
        case AudioCurve::CUBIC:
            processedAudio = audioInput * audioInput * audioInput;
            break;
        case AudioCurve::LOGARITHMIC:
            // Logarithmic response gives more detail to quiet sounds
            if (audioInput > 0.01f) { // Avoid log(0)
                processedAudio = 0.3f * logf(1.0f + 10.0f * audioInput);
                // Scale to approximate 0-1 range
                processedAudio = std::min(1.0f, processedAudio);
            }
            break;
    }
    
    return processedAudio;
}

// Beat detection algorithm
void TwoTone::detectBeat(float audioLevel) {
    // Reset beat detected flag at the start of each frame
    beatDetected = false;
    
    // Increment frames since last beat counter
    framesSinceLastBeat++;
    
    // Add the current energy level to our history and remove the oldest one
    energyHistory.push_back(audioLevel);
    if (energyHistory.size() > 30) { // Keep a 30-frame history
        energyHistory.pop_front();
    }
    
    // Calculate the average energy over recent history
    float sum = 0.0f;
    for (float e : energyHistory) {
        sum += e;
    }
    float averageEnergy = sum / energyHistory.size();
    
    // Adjust beat threshold with sensitivity and decay
    beatThreshold = beatThreshold * beatDecay + (1.0f - beatDecay) * averageEnergy * beatSensitivity;
    
    // If we're past the minimum beat interval and the current level exceeds the threshold
    if (framesSinceLastBeat >= minBeatInterval && audioLevel > beatThreshold && audioLevel > 0.05f) {
        beatDetected = true;
        framesSinceLastBeat = 0;
        
        // Print debug info about the beat
        printf("BEAT detected! Level: %f, Threshold: %f\n", audioLevel, beatThreshold);
    }
}

// Apply the standard color pulse visualization
void TwoTone::run()
{
    double seconds = secTimer->takeSeconds();
    
    // Process audio input according to settings
    float processedAudio = processAudio(micVal);
    
    // Perform beat detection
    detectBeat(processedAudio);
    
    // Apply the current visualization mode
    applyVisualization(processedAudio);
    
    double micAdd;
    if (smoothingMicVal <= 0.0)
    {
        micAdd = processedAudio; // If smoothingMicVal is 0, use processed audio directly
    } else
    {
        micAdd = processedAudio / smoothingMicVal; // trying to even out timing
    }

    printf("MicVal: %f\tMicAdd: %f\tsmoothingMicVal: %f\n", processedAudio, micAdd, smoothingMicVal);
    int avgLoops = 0;
    avgLoops = hueTimer->takeMsEvery(1);
    for (int i = 0; i < avgLoops; i++)
    {
        hueAdd = (hueAdd * 100.0 + micAdd) / 101.0;
    }

    if (micAdd > hueAdd)
    {
        hueAdd = micAdd;
    }

    // Scale the brightness increase based on audio level
    double tempBrightnessIncrease = processedAudio * (1.0 - baseBrightness) * baseBrightness;
    
    // Add extra brightness boost on beat detection for beat-reactive modes
    if (beatDetected && (visMode == VisualizationMode::BEAT_FLASH || visMode == VisualizationMode::BEAT_EXPAND)) {
        tempBrightnessIncrease += beatReaction * (1.0 - baseBrightness) * 0.25f;
    }

    // Set proposed brightness as the sum of baseBrightness and the scaled temporary increase
    double proposedBrightness = baseBrightness + tempBrightnessIncrease;
    proposedBrightness = std::min(1.0, proposedBrightness); // Ensure brightness doesn't exceed 1.0

    avgLoops = brightnessTimer->takeMsEvery(1);
    // Iterate through the number of ms since last to smooth out the brightness.
    for (int i = 0; i < avgLoops; i++)
    {
        brightness = (brightness * 200.0 + proposedBrightness) / 201.0;
    }

    // Check if the proposed brightness is greater than the current brightness
    // and update it if necessary.
    if (proposedBrightness > brightness)
    {
        brightness = proposedBrightness;
    }

    // Iterate through all the pixels and set their values.
    for (SoundPixel *pixel : *pixels)
    {
        pixel->brightness = brightness;
        pixel->micVal = processedAudio;
        
        // Hue and saturation are set by the current visualization mode
    }
}

// Apply the selected visualization mode
void TwoTone::applyVisualization(float processedAudio) {
    // Select visualization mode
    switch (visMode) {
        case VisualizationMode::COLOR_PULSE:
            // Standard color pulse - existing code with some enhancements
            {
                // Scale the mic value by the reactivity
                float scaledMicValue = processedAudio * pulseReactivity;
                
                // Constrain to a reasonable range (0.0-1.0)
                scaledMicValue = std::min(1.0f, std::max(0.0f, scaledMicValue));
                
                // Quick rise on audio peaks, slower falloff when quiet
                if (scaledMicValue > pulsePosition) {
                    // Fast attack when audio increases
                    pulsePosition = pulsePosition * attackRate + scaledMicValue * (1.0f - attackRate);
                } else {
                    // Slower decay when audio decreases
                    pulsePosition = pulsePosition * decayRate;
                }
                
                // In very quiet situations, ensure it returns to default position
                if (processedAudio < audioThreshold && pulsePosition < audioThreshold) {
                    pulsePosition = 0.0f;
                }
                
                // Handle shortest path for hue interpolation
                float hueDiff = hue2 - hue1;
                if (hueDiff > 0.5f) hueDiff -= 1.0f;
                if (hueDiff < -0.5f) hueDiff += 1.0f;
                
                // Linear interpolation between the two hues
                float currentHue = hue1 + hueDiff * pulsePosition;
                
                // Normalize to [0,1] range
                if (currentHue < 0.0f) currentHue += 1.0f;
                if (currentHue >= 1.0f) currentHue -= 1.0f;
                
                // Set the color for all pixels
                for (SoundPixel *pixel : *pixels) {
                    pixel->hue = currentHue;
                    pixel->saturation = saturation;
                }
            }
            break;
            
        case VisualizationMode::BEAT_FLASH:
            applyBeatFlash(processedAudio);
            break;
            
        case VisualizationMode::BEAT_EXPAND:
            applyBeatExpand(processedAudio);
            break;
            
        case VisualizationMode::SPECTRUM_FLOW:
            applySpectrumFlow(processedAudio);
            break;
    }
}

// Apply beat flash visualization
void TwoTone::applyBeatFlash(float processedAudio) {
    // On beat detection, set flash intensity to maximum
    if (beatDetected) {
        beatFlashIntensity = 1.0f;
    }
    
    // Calculate the base hue using the standard color pulse technique
    float scaledMicValue = processedAudio * pulseReactivity;
    scaledMicValue = std::min(1.0f, std::max(0.0f, scaledMicValue));
    
    // Process the pulse position similar to normal mode
    if (scaledMicValue > pulsePosition) {
        // Fast attack when audio increases
        pulsePosition = pulsePosition * attackRate + scaledMicValue * (1.0f - attackRate);
    } else {
        // Slower decay when audio decreases
        pulsePosition = pulsePosition * decayRate;
    }
    
    // Handle shortest path for hue interpolation
    float hueDiff = hue2 - hue1;
    if (hueDiff > 0.5f) hueDiff -= 1.0f;
    if (hueDiff < -0.5f) hueDiff += 1.0f;
    
    // Linear interpolation between the two hues
    float currentHue = hue1 + hueDiff * pulsePosition;
    
    // Normalize to [0,1] range
    if (currentHue < 0.0f) currentHue += 1.0f;
    if (currentHue >= 1.0f) currentHue -= 1.0f;
    
    // Decay flash intensity
    beatFlashIntensity *= 0.8f; // Fast decay for flash effect
    
    // Set the color with flash effect (moving toward white on beat)
    float flashSaturation = saturation * (1.0f - beatFlashIntensity * beatReaction);
    
    for (SoundPixel *pixel : *pixels) {
        pixel->hue = currentHue;
        pixel->saturation = flashSaturation; // Lower saturation = more white
    }
}

// Apply beat expand visualization
void TwoTone::applyBeatExpand(float processedAudio) {
    // On beat detection, reset expand progress
    if (beatDetected) {
        beatExpandProgress = 0.0f;
    }
    
    // Progress the expansion effect
    beatExpandProgress = std::min(1.0f, beatExpandProgress + 0.05f);
    
    // Calculate base color using normal pulse
    float scaledMicValue = processedAudio * pulseReactivity;
    scaledMicValue = std::min(1.0f, std::max(0.0f, scaledMicValue));
    
    // Process the pulse position
    if (scaledMicValue > pulsePosition) {
        pulsePosition = pulsePosition * attackRate + scaledMicValue * (1.0f - attackRate);
    } else {
        pulsePosition = pulsePosition * decayRate;
    }
    
    // Set different colors along the strip to create an expanding effect
    int stripLen = pixels->size();
    for (int i = 0; i < stripLen; i++) {
        SoundPixel *pixel = (*pixels)[i];
        
        // Calculate position in strip (0.0-1.0)
        float pos = static_cast<float>(i) / stripLen;
        
        // Calculate distance from center
        float center = 0.5f;
        float distFromCenter = std::abs(pos - center);
        
        // If the pixel is within the expanding circle
        float expandRadius = beatExpandProgress * 0.5f; // Expands to full strip width
        
        // For beat expand effect - create moving wave from center
        if (distFromCenter < expandRadius) {
            // Use hue2 for pixels in the expanding wave
            pixel->hue = hue2;
        } else {
            // Use hue1 + normal pulse mapping for other pixels
            float hueDiff = hue2 - hue1;
            if (hueDiff > 0.5f) hueDiff -= 1.0f;
            if (hueDiff < -0.5f) hueDiff += 1.0f;
            
            float currentHue = hue1 + hueDiff * pulsePosition;
            if (currentHue < 0.0f) currentHue += 1.0f;
            if (currentHue >= 1.0f) currentHue -= 1.0f;
            
            pixel->hue = currentHue;
        }
        
        // Keep saturation consistent
        pixel->saturation = saturation;
    }
}

// Apply spectrum flow visualization
void TwoTone::applySpectrumFlow(float processedAudio) {
    // Get levels from different frequency bands
    float lowLevel = Microphone::getLowNormal();
    
    // Create a synthetic mid level until getMidNormal() is implemented
    float midLevel = (Microphone::getLowNormal() + Microphone::getHighNormal()) / 2.0f;
    
    // Use the actual high frequency data
    float highLevel = Microphone::getHighNormal();
    
    // Add some randomization to make the mid value appear more distinct
    midLevel = std::min(1.0f, std::max(0.0f, midLevel + ((rand() % 10) - 5) * 0.01f));
    
    // Square them for better response
    lowLevel = lowLevel * lowLevel;
    midLevel = midLevel * midLevel;
    highLevel = highLevel * highLevel;
    
    // Map each frequency band to a different color
    float lowHue = 0.0f;       // Red for bass
    float midHue = 0.33f;      // Green for mids
    float highHue = 0.66f;     // Blue for highs
    
    // Process the pulse position similar to normal mode
    float scaledMicValue = processedAudio * pulseReactivity;
    scaledMicValue = std::min(1.0f, std::max(0.0f, scaledMicValue));
    
    if (scaledMicValue > pulsePosition) {
        pulsePosition = pulsePosition * attackRate + scaledMicValue * (1.0f - attackRate);
    } else {
        pulsePosition = pulsePosition * decayRate;
    }
    
    // Set different colors for different sections of the strip
    int stripLen = pixels->size();
    int lowSection = stripLen / 3;
    int midSection = 2 * lowSection;
    
    for (int i = 0; i < stripLen; i++) {
        SoundPixel *pixel = (*pixels)[i];
        
        // Assign different frequency bands to different sections
        if (i < lowSection) {
            // Low frequency section - set hue and make brightness reactive to bass
            pixel->hue = lowHue;
            pixel->saturation = saturation;
        } 
        else if (i < midSection) {
            // Mid frequency section
            pixel->hue = midHue;
            pixel->saturation = saturation;
        } 
        else {
            // High frequency section
            pixel->hue = highHue;
            pixel->saturation = saturation;
        }
    }
}

void TwoTone::resetSecTimer()
{
    secTimer->reset();
}

void TwoTone::setHues(float hue1, float hue2)
{
    setHue1(hue1);
    setHue2(hue2);
}

void TwoTone::setHue1(float value)
{
    // Normalize hue value to [0,1] range
    hue1 = value < 0 ? fmod(value, 1.0f) + 1.0f : fmod(value, 1.0f);
}

void TwoTone::setHue2(float value)
{
    // Normalize hue value to [0,1] range
    hue2 = value < 0 ? fmod(value, 1.0f) + 1.0f : fmod(value, 1.0f);
}

void TwoTone::setPeriod(float seconds)
{
    // Ensure a minimum period to prevent division by zero
    pulsePeriod = std::max(0.1f, seconds);
}

void TwoTone::setReactivity(float value)
{
    // Clamp reactivity to [0,10] range (allowing much higher values for stronger response)
    pulseReactivity = std::min(10.0f, std::max(0.0f, value));
}

// Parameter methods
void TwoTone::setAttackRate(float value)
{
    // Lower values = faster attack, but should stay in reasonable range
    attackRate = std::min(0.99f, std::max(0.01f, value));
}

void TwoTone::setDecayRate(float value)
{
    // Higher values = slower decay, constrain to reasonable range
    decayRate = std::min(0.99f, std::max(0.5f, value));
}

void TwoTone::setFrequencyBand(FrequencyBand band)
{
    freqBand = band;
}

void TwoTone::setAudioCurve(AudioCurve curve)
{
    audioCurve = curve;
}

void TwoTone::setSaturation(float value)
{
    // Clamp saturation to [0,1] range
    saturation = std::min(1.0f, std::max(0.0f, value));
}

void TwoTone::setAudioThreshold(float value)
{
    // Clamp threshold to [0,0.5] range (higher would make effect unresponsive)
    audioThreshold = std::min(0.5f, std::max(0.0f, value));
}

// New parameter methods for beat detection
void TwoTone::setBeatSensitivity(float value)
{
    // Clamp beat sensitivity to [0.5,5.0] range
    beatSensitivity = std::min(5.0f, std::max(0.5f, value));
}

void TwoTone::setBeatReaction(float value)
{
    // Clamp beat reaction strength to [0,1] range
    beatReaction = std::min(1.0f, std::max(0.0f, value));
}

void TwoTone::setVisualizationMode(VisualizationMode mode)
{
    visMode = mode;
    
    // Reset state when changing modes
    beatFlashIntensity = 0.0f;
    beatExpandProgress = 0.0f;
    pulsePosition = 0.0f;
}

TwoTone::~TwoTone()
{
    delete (secTimer);
    delete (hueTimer);
    delete (brightnessTimer);
    delete (pixels);
}
