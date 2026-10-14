#pragma once

// clang-format off
/* === MODULE MANIFEST V2 ===
module_description: SPI WS2812 heartbeat and fatal-error indicator
depends: []
=== END MANIFEST === */
// clang-format on

#include <array>
#include <cstdint>

#include "libxr_assert.hpp"
#include "spi.hpp"
#include "timer.hpp"

class BlinkLED_WS {
 public:
  // One WS2812, GRB/MSB first. At 6 MHz: bit=1.333 us,
  // T0H=0.333 us, T1H=0.667 us; each reset is 320 us low.
  using Frame = std::array<uint8_t, 504>;
  static Frame Encode(uint8_t red, uint8_t green, uint8_t blue) {
    Frame frame{};
    size_t pos = 240;
    for (uint8_t channel : {green, red, blue}) {
      for (int bit = 7; bit >= 0; --bit) {
        frame[pos++] = (channel & (1u << bit)) ? 0xF0 : 0xC0;
      }
    }
    return frame;
  }

  BlinkLED_WS(LibXR::SPI& spi, uint32_t blink_cycle, uint8_t red, uint8_t green,
              uint8_t blue)
      : spi_(spi),
        red_(red),
        green_(green),
        blue_(blue),
        timer_handle_(
            LibXR::Timer::CreateTask(BlinkTaskFun, this, blink_cycle)) {
    ASSERT(blink_cycle > 0);
    ASSERT(spi_.GetTxBuffer().size_ >= Frame{}.size());
    ASSERT(spi_.SetConfig(
               {LibXR::SPI::ClockPolarity::LOW, LibXR::SPI::ClockPhase::EDGE_1,
                LibXR::SPI::Prescaler::DIV_4, false}) == LibXR::ErrorCode::OK);
    ASSERT(Write(false) == LibXR::ErrorCode::OK);
    LibXR::Timer::Add(timer_handle_);
    LibXR::Timer::Start(timer_handle_);
    auto error_callback = LibXR::Callback<const char*, uint32_t>::Create(
        [](bool in_isr, BlinkLED_WS* led, const char*, uint32_t) {
          LibXR::Timer::Stop(led->timer_handle_);
          if (!in_isr) {
            while (true) {
              led->Write(false);
              LibXR::Thread::Sleep(125);
              led->Write(true);
              LibXR::Thread::Sleep(125);
              led->Write(false);
              LibXR::Thread::Sleep(500);
              led->Write(true);
              LibXR::Thread::Sleep(500);
            }
          }
        },
        this);
    LibXR::Assert::RegisterFatalErrorCallback(error_callback);
  }

  static void BlinkTaskFun(BlinkLED_WS* led) {
    if (led->Write(!led->flag_) == LibXR::ErrorCode::OK) {
      led->flag_ = !led->flag_;
    }
  }
  void OnMonitor() {}

 private:
  LibXR::ErrorCode Write(bool on) {
    auto frame = Encode(on ? red_ : 0, on ? green_ : 0, on ? blue_ : 0);
    LibXR::SPI::OperationRW op;
    return spi_.ReadAndWrite({nullptr, 0}, {frame.data(), frame.size()}, op);
  }

  LibXR::SPI& spi_;
  uint8_t red_, green_, blue_;
  bool flag_ = false;
  LibXR::Timer::TimerHandle timer_handle_;
};
