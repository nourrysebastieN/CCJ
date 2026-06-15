#include "pulse_reader.h"

#include <chrono>
#include <thread>

// GPIO backend is abstracted behind these stubs so the same code compiles
// on desktop (simulation) and on Pi (pigpio or wiringPi).
// Replace with real GPIO calls when targeting hardware.
namespace gpio {

static void setup(int /*pin*/) {}
static void set_interrupt(int /*pin*/, void (*/*cb*/)(void)) {}
static void clear_interrupt(int /*pin*/) {}

} // namespace gpio

PulseReader::PulseReader(int rpm_gpio_pin, int vss_gpio_pin)
    : m_rpm_pin(rpm_gpio_pin), m_vss_pin(vss_gpio_pin) {}

PulseReader::~PulseReader() {
    stop();
}

void PulseReader::start() {
    gpio::setup(m_rpm_pin);
    gpio::setup(m_vss_pin);

    gpio::set_interrupt(m_rpm_pin, []() {});
    gpio::set_interrupt(m_vss_pin, []() {});

    m_running = true;
    std::thread([this]() { compute_loop(); }).detach();
}

void PulseReader::stop() {
    m_running = false;
    gpio::clear_interrupt(m_rpm_pin);
    gpio::clear_interrupt(m_vss_pin);
}

float PulseReader::rpm() const {
    return m_rpm.load();
}

float PulseReader::speed_kmh() const {
    return m_speed_kmh.load();
}

void PulseReader::compute_loop() {
    using namespace std::chrono;
    constexpr auto interval = milliseconds(100); // recalculate every 100ms

    while (m_running) {
        auto t0 = steady_clock::now();

        uint32_t rpm_pulses = m_rpm_pulse_count.exchange(0);
        uint32_t vss_pulses = m_vss_pulse_count.exchange(0);

        // pulses/interval → revolutions/min
        float revs_per_interval = static_cast<float>(rpm_pulses) / RPM_PULSES_PER_REV;
        m_rpm = revs_per_interval * (60000.0f / interval.count());

        // pulses/interval → km/h
        float km_per_interval = static_cast<float>(vss_pulses) / VSS_PULSES_PER_KM;
        m_speed_kmh = km_per_interval * (3600000.0f / interval.count());

        std::this_thread::sleep_until(t0 + interval);
    }
}
