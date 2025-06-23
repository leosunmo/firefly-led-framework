#include "UARTManager.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>

namespace FireFly
{

    UARTManager &UARTManager::getInstance()
    {
        static UARTManager instance;
        return instance;
    }

    UARTManager::UARTManager()
    {
        // Create the parser with a callback to our static handler
        parser = new UARTMessageParser(&UARTManager::handleMessage);
    }

    UARTManager::~UARTManager()
    {
        if (parser)
        {
            delete parser;
            parser = nullptr;
        }
    }

    void UARTManager::init(uart_inst_t *uart, uint baudRate, uint txPin, uint rxPin)
    {
        if (uartInst)
        {
            // Already initialized
            printf("UART Manager already initialized\n");
            return;
        }

        uartInst = uart;

        // Initialize UART with the specified baud rate
        uint setRate = 0; // Reset to 0 before setting
        setRate = uart_init(uartInst, baudRate);

        // Set the data bits, stop bits and parity
        uart_set_format(uartInst, 8, 1, UART_PARITY_NONE);

        // Set the GPIO pins for UART
        gpio_set_function(txPin, GPIO_FUNC_UART); // TX
        gpio_set_function(rxPin, GPIO_FUNC_UART); // RX

        // Configure UART FIFO
        uart_set_fifo_enabled(uartInst, true);

        // Set flow control (if needed)
        uart_set_hw_flow(uartInst, false, false); // No CTS/RTS

        printf("UART Manager initialized on uart%d (tx: %u, rx: %u) at %u baud\n",
               uartInst == uart0 ? 0 : 1, txPin, rxPin, setRate);
    }

    void UARTManager::processInput()
    {
        if (!uartInst)
        {
            // Not initialized
            printf("UART Manager not initialized! Cannot process input.\n");
            return;
        }

        static bool activity = false;
        bool hasData = false;

        // Read all available UART data and process it
        while (uart_is_readable(uartInst))
        {
            // Set activity flag - we'll update LED once after all bytes processed
            activity = true;
            hasData = true;

            // Get the byte
            uint8_t ch = uart_getc(uartInst);
            // Process through the normal parser
            parser->processByte(ch);
        }

        // Update status LED based on activity
        if (statusLEDCallback)
        {
            if (activity && hasData)
            {
                // Flash the LED on if we got data
                statusLEDCallback(200);
                activity = false;
            }
            else
            {
                // Keep it dimly lit when idle
                statusLEDCallback(1);
            }
        }
    }

    void UARTManager::sendMessage(const UARTMessage &msg)
    {
        if (!uartInst)
        {
            printf("UART Manager not initialized! Cannot send message.\n");
            return;
        }

        // Write the message directly to UART
        uart_write_blocking(uartInst, (const uint8_t *)&msg, sizeof(UARTMessage));
    }

    void UARTManager::sendCommand(CommandType cmdType, uint32_t value)
    {
        // Create a properly formatted message
        UARTMessage msg = {
            .start = MESSAGE_START,
            .cmdType = cmdType,
            .value = value,
            .end = MESSAGE_END};

        sendMessage(msg);
    }

    void UARTManager::handleMessage(const UARTMessage &msg)
    {
        // Forward the message to the InputManager for processing
        InputManager::getInstance().processUARTMessage(msg);
    }

} // namespace FireFly
