#include "TouchInput.h"

#include <SPI.h>

#include "BoardConfig.h"

static constexpr uint16_t TouchSampleIntervalMs = 25;
static constexpr uint16_t TouchDebounceMs = 40;
static constexpr uint16_t TouchLongPressMs = 700;
static constexpr uint16_t TouchMinPressure = 1;

TouchInput::TouchInput(XPT2046_Touchscreen &touch) : touch_(touch) {}

bool TouchInput::begin() {
  pinMode(TOUCH_IRQ, INPUT);
  const bool ok = touch_.begin();
  SPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  touch_.setRotation(1);
  return ok;
}

TouchState TouchInput::update(uint32_t nowMs) {
  TouchState sample = state_;
  sample.changed = false;

  if (nowMs - lastSampleMs_ < TouchSampleIntervalMs) {
    return sample;
  }

  lastSampleMs_ = nowMs;
  sample = mapRaw(readRaw());

  if (sample.pressed != candidatePressed_) {
    candidatePressed_ = sample.pressed;
    candidateSinceMs_ = nowMs;
  }

  const bool debouncedPressed = (sample.pressed == state_.pressed) ||
                                (nowMs - candidateSinceMs_ >= TouchDebounceMs)
                                    ? sample.pressed
                                    : state_.pressed;

  if (debouncedPressed && !state_.pressed) {
    pressedSinceMs_ = nowMs;
  }

  sample.pressed = debouncedPressed;
  sample.longPressed = sample.pressed && (nowMs - pressedSinceMs_ >= TouchLongPressMs);
  if (!sample.pressed) {
    sample.screenX = -1;
    sample.screenY = -1;
    sample.longPressed = false;
  }

  sample.changed = sample.pressed != state_.pressed ||
                   sample.longPressed != state_.longPressed ||
                   sample.rawX != state_.rawX ||
                   sample.rawY != state_.rawY ||
                   sample.rawZ != state_.rawZ ||
                   sample.screenX != state_.screenX ||
                   sample.screenY != state_.screenY ||
                   sample.irqActive != state_.irqActive;

  if (sample.changed && sample.pressed) {
    logState(sample);
  }

  state_ = sample;
  return state_;
}

TouchState TouchInput::current() const {
  return state_;
}

const char *TouchInput::controllerName() const {
  return "XPT2046";
}

bool TouchInput::isValidRaw(uint16_t rawX, uint16_t rawY, uint16_t rawZ) const {
  if (rawX == TOUCH_IDLE_X && rawY == TOUCH_IDLE_Y) {
    return false;
  }

  if (rawX == 0 && rawY == 0 && rawZ < TouchMinPressure) {
    return false;
  }

  return rawZ >= TouchMinPressure &&
         rawX >= TOUCH_MIN_X && rawX <= TOUCH_MAX_X &&
         rawY >= TOUCH_MIN_Y && rawY <= TOUCH_MAX_Y;
}

TouchState TouchInput::readRaw() const {
  TouchState raw;
  const TS_Point point = touch_.getPoint();
  raw.rawX = point.x;
  raw.rawY = point.y;
  raw.rawZ = point.z;
  raw.irqActive = digitalRead(TOUCH_IRQ) == LOW;
  raw.pressed = isValidRaw(raw.rawX, raw.rawY, raw.rawZ);
  return raw;
}

TouchState TouchInput::mapRaw(const TouchState &raw) const {
  TouchState mapped = raw;
  if (!mapped.pressed) {
    mapped.screenX = -1;
    mapped.screenY = -1;
    return mapped;
  }

  uint16_t mapX = mapped.rawX;
  uint16_t mapY = mapped.rawY;
  if (TOUCH_SWAP_XY) {
    const uint16_t swap = mapX;
    mapX = mapY;
    mapY = swap;
  }

  mapped.screenX = mapAxis(mapX, TOUCH_MIN_X, TOUCH_MAX_X, TOUCH_SCREEN_WIDTH - 1);
  mapped.screenY = mapAxis(mapY, TOUCH_MIN_Y, TOUCH_MAX_Y, TOUCH_SCREEN_HEIGHT - 1);

  if (TOUCH_INVERT_X) {
    mapped.screenX = (TOUCH_SCREEN_WIDTH - 1) - mapped.screenX;
  }
  if (TOUCH_INVERT_Y) {
    mapped.screenY = (TOUCH_SCREEN_HEIGHT - 1) - mapped.screenY;
  }

  return mapped;
}

int16_t TouchInput::mapAxis(uint16_t raw, uint16_t rawMin, uint16_t rawMax, int16_t screenMax) const {
  const long mapped = map(raw, rawMin, rawMax, 0, screenMax);
  return static_cast<int16_t>(constrain(mapped, 0L, static_cast<long>(screenMax)));
}

void TouchInput::logState(const TouchState &state) const {
  Serial.print("Touch raw=");
  Serial.print(state.rawX);
  Serial.print(",");
  Serial.print(state.rawY);
  Serial.print(" z=");
  Serial.print(state.rawZ);
  Serial.print(" mapped=");
  Serial.print(state.screenX);
  Serial.print(",");
  Serial.print(state.screenY);
  Serial.print(state.longPressed ? " long-pressed" : " pressed");
  Serial.print(" irq=");
  Serial.println(state.irqActive ? "active" : "inactive");
}
