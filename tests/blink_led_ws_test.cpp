#include <cassert>

#include "BlinkLED_WS.hpp"

int main() {
  // Distinct channels verify GRB order and MSB-first serialization.
  const auto frame = BlinkLED_WS::Encode(0x55, 0x80, 0x01);
  for (size_t i = 0; i < frame.size(); ++i) {
    if (i < 240 || i >= 264) {
      assert(frame[i] == 0);
    } else {
      const uint8_t grb[] = {0x80, 0x55, 0x01};
      const size_t bit = i - 240;
      assert(frame[i] == ((grb[bit / 8] & (0x80 >> (bit % 8))) ? 0xF0 : 0xC0));
    }
  }
  const auto off = BlinkLED_WS::Encode(0, 0, 0);
  for (size_t i = 240; i < 264; ++i) assert(off[i] == 0xC0);
  const auto white = BlinkLED_WS::Encode(255, 255, 255);
  for (size_t i = 240; i < 264; ++i) assert(white[i] == 0xF0);
}
