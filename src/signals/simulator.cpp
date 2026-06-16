#include "signals/simulator.hpp"

#include <algorithm>
#include <cmath>

Simulator::Simulator() : m_start(Clock::now()) {}

float Simulator::elapsed_s() const {
    return std::chrono::duration_cast<Seconds>(Clock::now() - m_start).count();
}

DashboardData Simulator::tick() {
    float t = elapsed_s();

    DashboardData data;
    data.rpm           = simulate_rpm(t);
    data.speed_kmh     = simulate_speed(t);
    data.fuel_percent  = simulate_fuel(t);
    data.coolant_temp_c = simulate_coolant_temp(t);
    return data;
}

float Simulator::simulate_rpm(float t) const {
    // 0–5s: cold idle at ~1200 RPM
    // 5–15s: rev up to 6000
    // 15–30s: driving cycle with gear shifts (RPM drops + climbs)
    // 30s+: loops
    float phase = std::fmod(t, 30.0f);

    if (phase < 5.0f) {
        // Cold idle — slight fluctuation
        return 1200.0f + 80.0f * std::sin(phase * 2.0f);
    }
    if (phase < 15.0f) {
        // Linear rev-up 1200 → 6500
        float p = (phase - 5.0f) / 10.0f;
        return 1200.0f + p * 5300.0f + 100.0f * std::sin(phase * 8.0f);
    }
    // Gear shift cycle: sawtooth between 2500 and 5500
    float p = std::fmod(phase - 15.0f, 5.0f) / 5.0f;
    return 2500.0f + p * 3000.0f;
}

float Simulator::simulate_speed(float t) const {
    // Mirrors RPM loosely: stationary during idle, accelerates then cruises
    float phase = std::fmod(t, 30.0f);

    if (phase < 5.0f)  return 0.0f;
    if (phase < 15.0f) {
        float p = (phase - 5.0f) / 10.0f;
        return p * 100.0f; // 0 → 100 km/h
    }
    // Cruise with slight variation
    return 80.0f + 15.0f * std::sin((phase - 15.0f) * 0.5f);
}

float Simulator::simulate_fuel(float t) const {
    // Starts at 75%, drains very slowly over time
    float drain = t * 0.002f; // ~0.12% per minute — cosmetic for dev
    return std::clamp(75.0f - drain, 0.0f, 100.0f);
}

float Simulator::simulate_coolant_temp(float t) const {
    // Cold start at 20°C, warms up to 90°C over ~5 minutes, holds there
    constexpr float T_COLD    = 20.0f;
    constexpr float T_NORMAL  = 90.0f;
    constexpr float WARMUP_S  = 300.0f;

    if (t >= WARMUP_S) return T_NORMAL;

    float p = t / WARMUP_S;
    // Exponential-like warm-up curve
    return T_COLD + (T_NORMAL - T_COLD) * (1.0f - std::exp(-5.0f * p));
}
