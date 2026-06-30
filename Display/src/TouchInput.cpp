#include "TouchInput.h"

#include <SPI.h>

#include "BoardConfig.h"

static constexpr uint16_t TouchSampleIntervalMs = 25;

TouchInput::TouchInput(XPT2046_Touchscreen &touch) : touch_(touch) {}

bool TouchInput::begin() {
  SPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  const bool ok = touch_.begin();
  touch_.setRotation(1);
  return ok;
}

TouchState TouchInput::update(uint32_t nowMs) {
  TouchState next = state_;
  next.changed = false;

  if (nowMs - lastSampleMs_ < TouchSampleIntervalMs) {
    return next;
  }

  lastSampleMs_ = nowMs;
  next.pressed = false;
  next.rawX = TOUCH_IDLE_X;
  next.rawY = TOUCH_IDLE_Y;
  next.screenX = -1;
  next.screenY = -1;

  if (touch_.touched()) {
    const TS_Point point = touch_.getPoint();
    next.rawX = point.x;
    next.rawY = point.y;

    if (isValidRaw(next.rawX, next.rawY)) {
      next.pressed = true;
      next.screenX = mapAxis(next.rawX, TOUCH_CAL_MIN_X, TOUCH_CAL_MAX_X, TOUCH_SCREEN_WIDTH - 1);
      next.screenY = mapAxis(next.rawY, TOUCH_CAL_MIN_Y, TOUCH_CAL_MAX_Y, TOUCH_SCREEN_HEIGHT - 1);
    }
  }

  next.changed = next.pressed != state_.pressed ||
                 next.rawX != state_.rawX ||
                 next.rawY != state_.rawY ||
                 next.screenX != state_.screenX ||
                 next.screenY != state_.screenY;

  if (next.changed) {
    logState(next);
  }

  state_ = next;
  return state_;
}

TouchState TouchInput::current() const {
  return state_;
}

bool TouchInput::isValidRaw(uint16_t rawX, uint16_t rawY) const {
  if (rawX == TOUCH_IDLE_X && rawY == TOUCH_IDLE_Y) {
    return false;
  }

  return rawX >= TOUCH_CAL_MIN_X && rawX <= TOUCH_CAL_MAX_X &&
         rawY >= TOUCH_CAL_MIN_Y && rawY <= TOUCH_CAL_MAX_Y;
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
  Serial.print(" mapped=");

  if (state.pressed) {
    Serial.print(state.screenX);
    Serial.print(",");
    Serial.print(state.screenY);
    Serial.println(" pressed");
  } else {
    Serial.println("not pressed released");
  }
}
