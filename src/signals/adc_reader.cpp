#include "adc_reader.h"

#include <cstdint>

// SPI backend stub — replace with pigpio spiOpen/spiXfer or wiringPiSPISetup
// when targeting Pi hardware.
namespace spi {

static int open(int /*channel*/, int /*speed*/) { return 0; }
static void close(int /*fd*/) {}
static uint16_t read_mcp3008(int /*fd*/, int /*channel*/) { return 0; }

} // namespace spi

AdcReader::AdcReader(int spi_channel, int fuel_channel, int temp_channel)
    : m_spi_channel(spi_channel), m_fuel_ch(fuel_channel), m_temp_ch(temp_channel) {}

AdcReader::~AdcReader() {
    if (m_spi_fd >= 0)
        spi::close(m_spi_fd);
}

bool AdcReader::init() {
    m_spi_fd = spi::open(m_spi_channel, 1'000'000); // 1 MHz
    return m_spi_fd >= 0;
}

uint16_t AdcReader::read_fuel_raw() const {
    return read_channel(m_fuel_ch);
}

uint16_t AdcReader::read_temp_raw() const {
    return read_channel(m_temp_ch);
}

uint16_t AdcReader::read_channel(int channel) const {
    return spi::read_mcp3008(m_spi_fd, channel);
}
