#include "FireFlyV2Controller.h"
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "pico/time.h"
#include "../../../config.h"

absolute_time_t FireFlyV2Controller::channel_end_times[NUM_STRIPS];
strip_t FireFlyV2Controller::strips[NUM_STRIPS];
FireFlyV2Controller::FireFlyV2Controller()
{
    // Make sure we set the clock before we initiate communication
    // as it affects for example the UART baud rate.
    if (OVERCLOCK)
    {
        // How to overclock (or underclock!)
        if (!set_sys_clock_khz(250000, false))
        {
            printf("system clock 250MHz failed\n");
        }
        else
        {
            printf("system clock now 250MHz\n");
        }
    }
    setStatusLED(255);
    initCommunication();
    initBrightness();
    if (MICROPHONE_ENABLE)
    {
        initMicrophone();
    }
    initOutput();

    this->timing = new Timing();

    setStatusLED(10);

    // Set up the pattern index increment on button press
    auto &inputManager = FireFly::InputManager::getInstance();
    inputManager.subscribe(FireFly::InputEventType::PATTERN, [this](const FireFly::InputEvent &event)
                           {
            printf("Pattern button pressed\n");
            (*this->patternIndex)++; });
}

/**
 * Brightness 0 to 255
 */
void FireFlyV2Controller::setStatusLED(uint8_t brightness)
{
    // Brightness is 0 to 255
    static uint8_t initted = 0;
    if (initted == 0)
    {
        // Set GPIO 2 to PWM function
        gpio_set_function(status_led, GPIO_FUNC_PWM);
    }
    // Find the PWM slice number that corresponds to GPIO 2
    uint slice_num = pwm_gpio_to_slice_num(status_led);

    // Set PWM clock divider (optional)
    pwm_set_clkdiv(slice_num, 4.0f);

    // Set the PWM frequency and duty cycle
    pwm_set_wrap(slice_num, 255);                                               // Wrap value for 8-bit resolution
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(status_led), brightness); // 50% duty cycle

    // Enable the PWM output
    pwm_set_enabled(slice_num, true);
}

void FireFlyV2Controller::initDMA(PIO pio, uint sm)
{
}

// Used in interrupt
void FireFlyV2Controller::handleDMA()
{
    // Loops through the strips and checks if the channel is done. If it is, it sets the channel_end_times to the current time.
    uint8_t dma_chan;
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        dma_chan = strips[i].dma_chan;
        if (dma_channel_get_irq1_status(dma_chan))
        {
            dma_channel_acknowledge_irq1(dma_chan);
            channel_end_times[i] = get_absolute_time();
        }
    }
}

void FireFlyV2Controller::initOutput()
{

    // initDMA();

    uint offset = pio_add_program(pio, &ws2812_program);
    irq_set_exclusive_handler(DMA_IRQ_1, &FireFlyV2Controller::handleDMA);
    irq_set_enabled(DMA_IRQ_1, true);
    // Init 4 strips
    for (uint8_t i = 0; i < NUM_STRIPS; i++)
    {
        strips[i].pin = PX_pins[i];
        strips[i].sm = PX_sms[i];
        strips[i].dma_chan = dma_claim_unused_channel(true);
        dma_channel_set_irq1_enabled(strips[i].dma_chan, true);
        uint8_t PX_pin = PX_pins[i];
        gpio_init(PX_pin);
        gpio_set_dir(PX_pin, GPIO_OUT);
        gpio_put(PX_pin, 0);

        uint8_t PX_sm = PX_sms[i];
        // 800kHz, 8 bit transfers
        ws2812_program_init(pio, strips[i].sm, offset, strips[i].pin, 800000, 8);
    }

    // Make an array to flip bits for output
    uint8_t b;
    for (int i = 0; i < 256; i++)
    {
        b = i;
        b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
        b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
        b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
        bitflipLUT[i] = b;
    }
}

void FireFlyV2Controller::initCommunication()
{
    // Initialize the UARTManager
    auto &uartManager = FireFly::UARTManager::getInstance();
    uartManager.init(uart1, UART_BAUD_RATE, UART_TX_PIN, UART_RX_PIN); // UART1, 19200 baud, TX on GPIO 4, RX on GPIO 5

    // Set the status LED callback
    uartManager.setStatusLEDCallback([this](uint8_t brightness)
                                     { this->setStatusLED(brightness); });

    printf("Communication established\n");
}

uint64_t FireFlyV2Controller::getCurrentTimeMicros()
{
    // absolute_time_t new_time = get_absolute_time(); // Microseconds
    uint64_t micros = to_us_since_boot(get_absolute_time());
    return micros;
}

uint64_t FireFlyV2Controller::getCurrentTimeMillis()
{
    // absolute_time_t new_time = get_absolute_time(); // Microseconds
    uint64_t millis = to_us_since_boot(get_absolute_time()) / 1000;
    return millis;
}

void FireFlyV2Controller::outputLEDs(uint8_t strip_i, uint8_t *leds, uint32_t N)
{
    uint32_t numBytes = N * 3; // N is numLEDs, which each require 3 bytes
    uint8_t *pixels = leds;
    strip_t *strip = &(strips[strip_i]);

    int sm = strip->sm;
    int dma_chan = strip->dma_chan;
    dma_channel_wait_for_finish_blocking(dma_chan);
    // add a 200 us wait here, because the led strip needs a reset after each write
    // This will be skipped if the strip had already been finished for long enough
    // Therefore the performance will not be affected
    int64_t diff = 0;
    while ((diff = absolute_time_diff_us(channel_end_times[strip_i], get_absolute_time())) < 200)
    {
        // The condition is checking the difference between the current time and the last time the channel was finished.
    }

    // uint32_t bytes[numBytes];
    //  This is only necessary to format data, but later can change to this being how it is inputted
    uint8_t b;
    for (int i = 0; i < numBytes; i++)
    {
        // Bit reverse for 4000 LEDs = LEDs::output() = 2282 us
        // BitflipLUT for 4000 LEDs = LEDs::output() = 1202 us
        strip->outPointer[i] = bitflipLUT[pixels[i]];
        // strip->outPointer[i] = ((uint32_t)pixels[i]) << 24; // Old method
    }

    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    dma_channel_configure(dma_chan, &c,
                          &pio->txf[sm],     // Destination pointer
                          strip->outPointer, // Source pointer
                          numBytes,          // Number of transfers
                          true               // Start immediately
    );

    // while(numBytes--){
    //     // Bits for transmission must be shifted to top 8 bits
    //     pio_sm_put_blocking(PX_pio, strip, ((uint32_t)*pixels++)<< 24);
    // }
}

double FireFlyV2Controller::getBrightness()
{
#ifdef HARDCODE_BRIGHTNESS
    return HARDCODE_BRIGHTNESS;
#endif

    if (timing->takeMsEvery(10))
    {
        // Get hardware potentiometer value
        double newPot = analogPot->getValue();

        // Get UART brightness value (0-255 normalized to 0-1)
        double uartBrightness = FireFly::InputManager::getInstance().getValue(
                                    FireFly::InputEventType::BRIGHTNESS) /
                                255.0;

        // Use UART value if it's been set, otherwise use hardware
        if (uartBrightness > 0)
        {
            brightness = (brightness * 5.0 + uartBrightness) / 6.0;
        }
        else
        {
            brightness = (brightness * 5.0 + newPot) / 6.0;
        }
    }

    return brightness * brightness; // Apply gamma correction
}

double FireFlyV2Controller::getHue()
{
    // Get UART hue value (0-255 normalized to 0-1)
    double uartHue = FireFly::InputManager::getInstance().getValue(
                         FireFly::InputEventType::HUE) /
                     255.0;

    // Get hardware encoder hue (normalized to 0-1)
    int32_t encoderValue = FireFly::InputManager::getInstance().getValue(
        FireFly::InputEventType::HUE);
    double encoderHue = (encoderValue % 360) / 360.0;

    // Use UART hue if it's been set, otherwise use encoder
    if (uartHue > 0)
    {
        return uartHue;
    }
    else
    {
        return encoderHue;
    }
}

void FireFlyV2Controller::initBrightness()
{
    this->analogPot = new Potentiometer(pot_gpio, 1);
}

void FireFlyV2Controller::givePatternIndex(uint32_t *patternIndex)
{
    this->patternIndex = patternIndex;
}

void FireFlyV2Controller::initMicrophone()
{
    // assert(HW_ADC_MIC == 1); // Commenting out because we can accept both on this controller...
    Microphone::start(ADC_MIC); // This argument does nothing! Use the config.h / HW_ADC_MIC / HW_PDM_MIC to select the microphone type.
}

// UART processing is now handled by UARTManager