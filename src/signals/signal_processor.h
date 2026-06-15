#pragma once

#include <cstdint>

struct DashboardData {
    float rpm{0.0f};
    float speed_kmh{0.0f};
    float fuel_percent{0.0f};   // 0–100
    float coolant_temp_c{0.0f};
    bool  check_engine{false};
};

// Converts raw signal values into human-readable DashboardData.
// Calibration curves are based on Honda EK factory service manual values.
class SignalProcessor {
public:
    DashboardData process(float rpm, float speed_kmh,
                          uint16_t fuel_raw, uint16_t temp_raw) const;

private:
    // Fuel sender: Honda EK — empty ~110 Ω, full ~10 Ω
    // ADC raw (10-bit, 3.3V ref) ∝ voltage across fixed resistor in divider
    float fuel_raw_to_percent(uint16_t raw) const;

    // Coolant temp sender: NTC thermistor — resistance drops as temp rises
    float temp_raw_to_celsius(uint16_t raw) const;
};
