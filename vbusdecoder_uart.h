#ifndef VBUS_DECODER_UART_H_
#define VBUS_DECODER_UART_H_

#include "VBUSDecoder.h"
#include "esphome.h"
using namespace esphome;

static const char *TAG = "vbus";
VBUSDecoder vb;

class VBUSDecoderUARTComponent : public Component, public uart::UARTDevice {
  protected:
    uint8_t uart_buffer_[6]{0};
    sensor::Sensor *collector_temperature_{nullptr};
    sensor::Sensor *tank_temperature_{nullptr};
    float last_collector_temperature_;
    float last_tank_temperature_;

  public:
    VBUSDecoderUARTComponent(uart::UARTComponent *parent, sensor::Sensor *collector_temperature, sensor::Sensor *tank_temperature)
        : uart::UARTDevice(parent)
        , collector_temperature_(collector_temperature)
        , tank_temperature_(tank_temperature) {}

    // Run after hardware (UART), but before WiFi and MQTT so that we can
    // send status messages to the SB1 as these components come up.
    float get_setup_priority() const override {
      return setup_priority::DATA;
    }

    // Run very late in the loop, so that other components can process
    // before we check on their state and send status to the SB1.
    float get_loop_priority() const override {
      return -50.0f;
    }

    void setup() override {
      ESP_LOGCONFIG(TAG, "Setting up VBUS Decoder UART...");

      vb.initialise();
    }

    void dump_config() override {
      ESP_LOGCONFIG(TAG, "VBUS Decoder UART");
    }

    void loop() override {
      vb.readSensor();

      float collector_temperature = vb.getS1Temp();
      float tank_temperature = vb.getS2Temp();

      if (this->last_collector_temperature_ != collector_temperature) {
        this->collector_temperature_->publish_state(collector_temperature);
        this->last_collector_temperature_ = collector_temperature;
      }

      if (this->last_tank_temperature_ != tank_temperature) {
        this->tank_temperature_->publish_state(tank_temperature);
        this->last_tank_temperature_ = tank_temperature;
      }
    }
};

#endif // VBUS_DECODER_UART_H_
