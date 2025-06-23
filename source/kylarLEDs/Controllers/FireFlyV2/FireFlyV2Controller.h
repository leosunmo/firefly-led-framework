#pragma once
#include "../Controller.h"
#include "rp2040_pio.h"
#include "../Sensors/Potentiometer/Potentiometer.h"
#include "../Sensors/Encoder/Encoder.h"
#include "../Sensors/Button/Button.h"
#include "../Sensors/Microphone/Microphone.h"
#include "../../Utility/Timing.h"
#include "../../Communication/UARTManager.h"
#include "../../Input/InputManager.h"
#include <map>
#include "../../../Patterns/Pattern.h"

#define PX_PINS 3
#define UART_BAUD_RATE 19200
#define UART_TX_PIN 4
#define UART_RX_PIN 5

typedef struct {
    uint8_t pin;
    uint8_t sm;
    uint8_t dma_chan;
    uint8_t outPointer[NUM_LEDS*3];
} strip_t;

class FireFlyV2Controller : public Controller {
    public:
        FireFlyV2Controller();
        //using Controller::Controller;
        void outputLEDs(uint8_t strip, uint8_t *leds, uint32_t N); // leds is an array, N is the length
        uint64_t getCurrentTimeMillis();
        uint64_t getCurrentTimeMicros();
        double getHue();
        double getBrightness();
        static void handleDMA();
        void givePatternMap(std::map<uint8_t, Pattern*>* map, uint8_t* nextPatternId);
    protected:
        void initCommunication();
        void initBrightness(); // Change the brightness via potentiometer
        void initOutput();
        void initMicrophone();
    private:
        void initDMA(PIO pio, uint sm);
        void setStatusLED(uint8_t brightness);

        double brightness = 0.0;
        double hueValue = 0.0;

        static strip_t strips[NUM_STRIPS];
        // uint8_t PX_pins[PX_PINS] = {14, 15, 16, 9};
        uint8_t PX_pins[PX_PINS] = {1, 25};//, 15};
        uint8_t PX_sms[PX_PINS] = {0, 1};///,2};
        uint8_t bitflipLUT[256];
        uint8_t status_led = 2; // GPIO 2 for status LED
        static absolute_time_t channel_end_times[NUM_STRIPS];
        
        //uint8_t PX_pin = 17; // 16 for 1, 17 for 2
        //uint8_t PX_sm = 0;
        PIO pio = pio0;
        //dma_channel_config *c;

        //PINS:
        uint8_t pot_gpio = 27;
        uint8_t boot_button = 22;

        
        // Inputs:
        Potentiometer *analogPot;
        Timing *timing;
        Microphone *microphone;
        
        // Pattern management
        uint8_t *patternId = nullptr;
        std::map<uint8_t, Pattern*>* patternMap = nullptr;
};