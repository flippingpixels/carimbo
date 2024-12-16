#include "entity.hpp"

using namespace framework;

entity::entity(entityprops &&props) noexcept
    : _props(std::move(props)) {}

entity::~entity() noexcept {
  std::cout << "entity of type '" << kind() << "' destroyed" << std::endl;
}

std::shared_ptr<entity> entity::create(entityprops &&props) {
  return std::make_shared<entity>(std::move(props));
}

uint64_t entity::id() const noexcept {
  return _props.id;
}

std::string entity::kind() const noexcept {
  return _props.kind;
}

entityprops &entity::props() noexcept {
  return _props;
}

const entityprops &entity::props() const noexcept {
  return _props;
}

int32_t entity::x() const noexcept {
  return _props.position.x();
}

int32_t entity::y() const noexcept {
  return _props.position.y();
}

void entity::move(float_t x_velocity, float_t y_velocity) noexcept {
  UNUSED(x_velocity);
  UNUSED(y_velocity);
  // cpBodySetVelocity(_props.body.get(), {x_velocity, y_velocity});
}

void entity::set_velocity(const math::vector2d &velocity) noexcept {
  _props.velocity = velocity;
}

math::vector2d entity::velocity() const noexcept {
  return _props.velocity;
}

void entity::update(float_t delta) noexcept {
  UNUSED(delta);

  if (_onupdate) {
    _onupdate(shared_from_this());
  }

  if (_props.action.empty() || !_props.visible) {
    return;
  }

  const auto now = SDL_GetTicks();
  const auto &animation = _props.animations.at(_props.action);
  const auto &frame = animation[_props.frame];

  if (frame.duration > 0 && now - _props.last_frame >= frame.duration) {
    ++_props.frame;
    _props.last_frame = now;

    if (_props.frame >= animation.size()) {
      if (std::ranges::any_of(animation, [](const auto &keyframe) { return keyframe.singleshoot; })) {
        _props.action.clear();

        if (_onanimationfinished) {
          _onanimationfinished(shared_from_this());
        }

        return;
      }

      _props.frame = 0;
    }
  }

  const auto x = static_cast<int32_t>(_props.position.x() + _props.velocity.x() * delta);
  const auto y = static_cast<int32_t>(_props.position.y() + _props.velocity.y() * delta);
  _props.position.set(x, y);
}

void entity::draw() const noexcept {
  if (_props.action.empty() || !_props.visible) {
    return;
  }

  const auto &animation = _props.animations.at(_props.action)[_props.frame];
  const auto &source = animation.frame;
  const auto &offset = animation.offset;
  geometry::rect destination{_props.position + offset, source.size()};
  destination.scale(_props.size.scale());

  _props.spritesheet->draw(
      source,
      destination,
      _props.angle,
      _props.reflection,
      _props.alpha
  );
}

void entity::set_props(entityprops props) noexcept {
  _props = std::move(props);
}

void entity::set_placement(int32_t x, int32_t y) noexcept {
  _props.position.set(x, y);
  // const auto body = _props.body.get();
  // cpBodySetVelocity(body, cpvzero);
  // cpBodySetForce(body, cpvzero);
  // cpBodySetPosition(body, {static_cast<cpFloat>(x), static_cast<cpFloat>(y)});
}

void entity::set_onupdate(const std::function<void(std::shared_ptr<entity>)> &fn) noexcept {
  _onupdate = fn;
}

void entity::set_onanimationfinished(const std::function<void(std::shared_ptr<entity>)> &fn) noexcept {
  _onanimationfinished = fn;
}

void entity::set_onmail(const std::function<void(std::shared_ptr<entity>, const std::string &)> &fn) noexcept {
  _onmail = fn;
}

void entity::set_reflection(graphics::reflection reflection) noexcept {
  _props.reflection = reflection;
}

void entity::set_action(const std::string &action) {
  if (_props.action != action) {
    _props.action.assign(action);
    _props.frame = 0;
    _props.last_frame = SDL_GetTicks();
  }
}

void entity::unset_action() {
  _props.action.clear();
  _props.frame = 0;
  _props.last_frame = SDL_GetTicks();
}

std::string entity::action() const noexcept {
  return _props.action;
}

geometry::size entity::size() const noexcept {
  return _props.size;
}

bool entity::visible() const noexcept {
  return _props.visible;
}

void entity::set_kv(const std::string &key, const std::variant<bool, std::string, int64_t, uint64_t, double_t, float_t> &value) noexcept {
  _kv[key] = value;
}

std::optional<std::variant<bool, std::string, int64_t, uint64_t, double_t, float_t>> entity::get_kv(const std::string &key) const noexcept {
  auto it = _kv.find(key);
  if (it == _kv.end()) {
    return std::nullopt;
  }

  return it->second;
}

void entity::on_email(const std::string &message) {
  if (_onmail) {
    _onmail(shared_from_this(), message);
  }
}
