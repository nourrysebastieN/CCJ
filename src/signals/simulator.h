#pragma once

#include "signal_processor.h"

#include <chrono>

// Generates realistic Honda EK sensor values for development without hardware.
// Simulates an engine warm-up followed by a driving cycle.
class Simulator {
public:
    Simulator();

    DashboardData tick();

private:
    using Clock = std::chrono::steady_clock;
    using Seconds = std::chrono::duration<float>;

    Clock::time_point m_start;

    float elapsed_s() const;

    float simulate_rpm(float t) const;
    float simulate_speed(float t) const;
    float simulate_fuel(float t) const;
    float simulate_coolant_temp(float t) const;
};
