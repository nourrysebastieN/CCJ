#pragma once

#include <atomic>
#include <cstdint>

// Reads RPM and VSS pulse signals via GPIO interrupts.
// Pulse counting runs in the background; call rpm() / speed_kmh() to get
// the latest computed values.
class PulseReader {
public:
    explicit PulseReader(int rpm_gpio_pin, int vss_gpio_pin);
    ~PulseReader();

    void start();
    void stop();

    float rpm() const;
    float speed_kmh() const;

private:
    int m_rpm_pin;
    int m_vss_pin;

    std::atomic<uint32_t> m_rpm_pulse_count{0};
    std::atomic<uint32_t> m_vss_pulse_count{0};

    std::atomic<float> m_rpm{0.0f};
    std::atomic<float> m_speed_kmh{0.0f};

    std::atomic<bool> m_running{false};

    // Pulses per revolution — EK distributor fires once per crank revolution
    static constexpr int RPM_PULSES_PER_REV = 1;
    // VSS pulses per km — Honda EK: ~4000 pulses/km (vehicle dependent)
    static constexpr int VSS_PULSES_PER_KM = 4000;

    void compute_loop();
};
