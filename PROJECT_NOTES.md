# CCJ — Honda Civic EK Digital Dashboard

## Project Goal

Replace or augment the Honda Civic EK (6th gen, ~1996–2000) analog instrument cluster with a C++ digital dashboard running on a Raspberry Pi, reading the original OEM sensor signals for a plug-and-play installation.

## Architecture

```
Car sensors → Raspberry Pi (signal reading + processing) → Digital display (C++ / SDL2)
```

The Pi taps into the same wiring harness the OEM cluster uses — no ECU modification required.

## OEM Signal Mapping

| Gauge / Input     | Signal Type                        | Pi Interface                        |
|-------------------|------------------------------------|-------------------------------------|
| Tachometer (RPM)  | Ignition pulse (~12V square wave)  | GPIO interrupt (pulse counting)     |
| Speedometer (VSS) | VSS pulse                          | GPIO interrupt (pulse counting)     |
| Fuel gauge        | Resistive sender (~10–110 Ω)       | ADC (e.g. MCP3008 via SPI)          |
| Coolant temp      | Resistive sender / NTC thermistor  | ADC (e.g. MCP3008 via SPI)          |
| Warning lights    | Ground-switched                    | GPIO digital input                  |

> **Note:** Honda EK ECUs use a proprietary K-line diagnostic protocol, not standard OBD-II (varies by year/market).

## Hardware

- **Dev platform:** Raspberry Pi
- **Final microcontroller:** TBD
- **ADC:** External required (Pi has no built-in ADC) — MCP3008 or similar over SPI
- **Display:** TBD (SDL2 targets Pi framebuffer)

## Planned Project Structure

```
ccj/
├── src/
│   ├── main.cpp
│   ├── signals/
│   │   ├── pulse_reader.cpp      # RPM + VSS via GPIO interrupts
│   │   ├── adc_reader.cpp        # Fuel + temp via SPI/ADC
│   │   └── signal_processor.cpp  # Convert raw values → human-readable
│   ├── display/
│   │   ├── dashboard.cpp         # Main render loop
│   │   └── gauges/               # Individual gauge widgets
│   └── obd/                      # Placeholder — future OBD-II support
├── CMakeLists.txt
└── config/
    └── ek_calibration.json       # Sensor calibration curves
```

## Roadmap

### Phase 1 — OEM Signal Reading (current)
- [ ] GPIO pulse reader for RPM and VSS
- [ ] SPI/ADC reader for fuel and coolant temp
- [ ] Signal processor with EK-specific calibration
- [ ] SDL2 dashboard render loop
- [ ] Basic gauge widgets (RPM, speed, fuel, temp)

### Phase 2 — OBD Support (future)
- [ ] Honda K-line / OBD-II interface
- [ ] CEL (Check Engine Light) scanning
- [ ] DTC reading and troubleshooting display

## Notes

- Resistive senders require a voltage divider circuit to convert resistance → voltage for ADC input
- GPIO inputs need level shifting (car signals are 12V, Pi GPIO is 3.3V)
- Calibration curves for fuel/temp senders will need to be mapped against EK factory service manual values
