#pragma once

#include "common.hpp"

namespace graphics {
enum class widgettype : uint8_t {
  label
};

class widget {
public:
  widget() = default;
  virtual ~widget() = default;

  virtual void update(float_t delta) noexcept = 0;

  virtual void draw() const noexcept = 0;
};
}
