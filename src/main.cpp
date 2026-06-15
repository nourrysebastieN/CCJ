#include "display/dashboard.h"
#include "signals/adc_reader.h"
#include "signals/pulse_reader.h"
#include "signals/signal_processor.h"

#include <chrono>
#include <thread>

int main() {
    // GPIO pin assignments (BCM numbering)
    constexpr int RPM_GPIO = 17;
    constexpr int VSS_GPIO = 27;

    // MCP3008 SPI channel and input channels
    constexpr int SPI_CHANNEL  = 0;
    constexpr int FUEL_CH      = 0;
    constexpr int TEMP_CH      = 1;

    PulseReader    pulse(RPM_GPIO, VSS_GPIO);
    AdcReader      adc(SPI_CHANNEL, FUEL_CH, TEMP_CH);
    SignalProcessor processor;
    Dashboard      dashboard(800, 480); // typical 7" Pi display resolution

    if (!adc.init() || !dashboard.init())
        return 1;

    pulse.start();

    constexpr auto frame_time = std::chrono::milliseconds(16); // ~60 fps

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
    return 0;
}
