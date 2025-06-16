#include "ChromaWave.h"
#include <math.h>

ChromaWave::ChromaWave()
{
    this->twoToneEffect = new TwoTone();
}

void ChromaWave::init()
{
    // Initialize the TwoTone effect
    twoToneEffect->init();
    
    // Apply the effect to the engine
    Effect::engine->apply(twoToneEffect);
    
    // Initialize with default hues
    twoToneEffect->setHues(hue, hue2);
    
    // Initialize with default audio processing parameters
    twoToneEffect->setAttackRate(attackRate);
    twoToneEffect->setDecayRate(decayRate);
    twoToneEffect->setFrequencyBand(static_cast<FrequencyBand>(frequencyBand));
    twoToneEffect->setAudioCurve(static_cast<AudioCurve>(audioCurve));
    twoToneEffect->setSaturation(saturation);
    twoToneEffect->setAudioThreshold(audioThreshold);
    
    // Initialize beat detection parameters
    twoToneEffect->setBeatSensitivity(beatSensitivity);
    twoToneEffect->setBeatReaction(beatReaction);
    twoToneEffect->setVisualizationMode(static_cast<VisualizationMode>(visualizationMode));
}

void ChromaWave::run()
{
    // Get audio levels from the microphone
    // We'll pass the raw audio to the effect, and let it process based on its settings
    double lowFreq = Microphone::getLowNormal();
    
    // Feed the raw microphone value to the effect
    // The TwoTone effect will process it based on frequency band and curve settings
    twoToneEffect->micVal = lowFreq;
}

void ChromaWave::setHue(double h)
{
    // First hue comes from standard hue parameter
    hue = h;
    if (twoToneEffect)
    {
        twoToneEffect->setHue1(hue);
    }
}

void ChromaWave::setSecondHue(double h)
{
    // Second hue is a custom parameter
    hue2 = h;
    if (twoToneEffect)
    {
        twoToneEffect->setHue2(hue2);
    }
}

void ChromaWave::setBrightness(float b)
{
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
    }
}

void ChromaWave::setPeriod(float seconds)
{
    if (twoToneEffect)
    {
        twoToneEffect->setPeriod(seconds);
    }
}

void ChromaWave::setReactivity(float value)
{
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
    
    if (twoToneEffect)
    {
        twoToneEffect->setAttackRate(value);
    }
}

void ChromaWave::setDecayRate(float value)
{
    // Store locally for initialization
    decayRate = value;
    
    if (twoToneEffect)
    {
        twoToneEffect->setDecayRate(value);
    }
}

void ChromaWave::setFrequencyBand(int bandIndex)
{
    // Store locally for initialization
    frequencyBand = bandIndex;
    
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
    
    if (twoToneEffect)
    {
        twoToneEffect->setSaturation(value);
    }
}

void ChromaWave::setAudioThreshold(float value)
{
    // Store locally for initialization
    audioThreshold = value;
    
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
    
    if (twoToneEffect)
    {
        twoToneEffect->setBeatSensitivity(value);
    }
}

void ChromaWave::setBeatReaction(float value)
{
    // Store locally for initialization
    beatReaction = value;
    
    if (twoToneEffect)
    {
        twoToneEffect->setBeatReaction(value);
    }
}

void ChromaWave::setVisualizationMode(int modeIndex)
{
    // Store locally for initialization
    visualizationMode = modeIndex;
    
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
    // Clean up is handled by the TwoTone effect's destructor
    // Nothing special to clean up here
}
