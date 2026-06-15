#pragma once

#include <cstdint>

// Reads resistive sender values (fuel, coolant temp) via an external SPI ADC
// (e.g. MCP3008). Returns raw 10-bit ADC counts; signal_processor converts
// these to physical units using calibration curves.
class AdcReader {
public:
    // spi_channel: SPI bus (0 or 1 on Pi)
    // fuel_channel / temp_channel: MCP3008 input channels (0–7)
    AdcReader(int spi_channel, int fuel_channel, int temp_channel);
    ~AdcReader();

    bool init();

    uint16_t read_fuel_raw() const;
    uint16_t read_temp_raw() const;

private:
    int m_spi_channel;
    int m_fuel_ch;
    int m_temp_ch;
    int m_spi_fd{-1};

    uint16_t read_channel(int channel) const;
};
