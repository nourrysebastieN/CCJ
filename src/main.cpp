#include "display/dashboard.hpp"
#include "display/layout.hpp"
#include "display/gauges/bar_gauge_factory.hpp"
#include "display/gauges/analog_gauge_factory.hpp"
#include "signals/signal_processor.hpp"

#ifdef CCJ_SIMULATE
#include "signals/simulator.hpp"
#else
#include "signals/adc_reader.hpp"
#include "signals/pulse_reader.hpp"
#endif

#include <chrono>
#include <thread>

int main() {
    Dashboard dashboard(1280, 480,
                        std::make_unique<AnalogGaugeFactory>("assets"),
                        Layout::grid_2x2(1280, 480));
    if (!dashboard.init())
        return 1;

    constexpr auto frame_time = std::chrono::milliseconds(16); // ~60 fps

#ifdef CCJ_SIMULATE
    Simulator sim;

    while (dashboard.is_running()) {
        auto t0 = std::chrono::steady_clock::now();
        dashboard.update(sim.tick());
        dashboard.render();
        std::this_thread::sleep_until(t0 + frame_time);
    }

#else
    constexpr int RPM_GPIO    = 17;
    constexpr int VSS_GPIO    = 27;
    constexpr int SPI_CHANNEL = 0;
    constexpr int FUEL_CH     = 0;
    constexpr int TEMP_CH     = 1;

    PulseReader     pulse(RPM_GPIO, VSS_GPIO);
    AdcReader       adc(SPI_CHANNEL, FUEL_CH, TEMP_CH);
    SignalProcessor processor;

    if (!adc.init())
        return 1;

    pulse.start();

    while (dashboard.is_running()) {
        auto t0 = std::chrono::steady_clock::now();

        DashboardData data = processor.process(
            pulse.rpm(),
            pulse.speed_kmh(),
            adc.read_fuel_raw(),
            adc.read_temp_raw()
        );

        dashboard.update(data);
        dashboard.render();

        std::this_thread::sleep_until(t0 + frame_time);
    }

    pulse.stop();
#endif

    return 0;
}
