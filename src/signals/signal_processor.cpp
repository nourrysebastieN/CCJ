#include "signal_processor.h"

#include <algorithm>
#include <cmath>

DashboardData SignalProcessor::process(float rpm, float speed_kmh,
                                       uint16_t fuel_raw, uint16_t temp_raw) const {
    DashboardData data;
    data.rpm          = rpm;
    data.speed_kmh    = speed_kmh;
    data.fuel_percent = fuel_raw_to_percent(fuel_raw);
    data.coolant_temp_c = temp_raw_to_celsius(temp_raw);
    return data;
}

float SignalProcessor::fuel_raw_to_percent(uint16_t raw) const {
    // Voltage divider: 3.3V — 100Ω fixed — sender — GND
    // ADC raw 0–1023 maps to 0–3.3V
    // Sender resistance: ~110Ω (empty) → ~10Ω (full)
    constexpr float V_REF    = 3.3f;
    constexpr float R_FIXED  = 100.0f;
    constexpr float ADC_MAX  = 1023.0f;

    float v_adc = (static_cast<float>(raw) / ADC_MAX) * V_REF;
    if (v_adc <= 0.0f) return 0.0f;

    float r_sender = R_FIXED * (V_REF - v_adc) / v_adc;

    constexpr float R_EMPTY = 110.0f;
    constexpr float R_FULL  = 10.0f;

    float percent = (R_EMPTY - r_sender) / (R_EMPTY - R_FULL) * 100.0f;
    return std::clamp(percent, 0.0f, 100.0f);
}

float SignalProcessor::temp_raw_to_celsius(uint16_t raw) const {
    // Steinhart-Hart approximation for Honda EK coolant temp sender (NTC)
    // Calibration points from factory service manual:
    //   20°C  ≈ 2500 Ω
    //   80°C  ≈  300 Ω
    //   110°C ≈  140 Ω
    constexpr float V_REF   = 3.3f;
    constexpr float R_FIXED = 2200.0f; // pull-up resistor
    constexpr float ADC_MAX = 1023.0f;

    float v_adc = (static_cast<float>(raw) / ADC_MAX) * V_REF;
    if (v_adc <= 0.0f || v_adc >= V_REF) return 0.0f;

    float r_sender = R_FIXED * v_adc / (V_REF - v_adc);

    // Steinhart-Hart coefficients (fitted to EK sender curve)
    constexpr float A = 1.40e-3f;
    constexpr float B = 2.37e-4f;
    constexpr float C = 9.90e-8f;

    float log_r = std::log(r_sender);
    float temp_k = 1.0f / (A + B * log_r + C * log_r * log_r * log_r);
    return temp_k - 273.15f;
}
