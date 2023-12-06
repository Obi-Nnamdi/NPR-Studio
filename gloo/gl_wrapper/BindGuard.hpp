#ifndef GLOO_BIND_GUARD_H_
#define GLOO_BIND_GUARD_H_

#include "IBindable.hpp"

namespace GLOO {
// https://riptutorial.com/opengl/example/24911/in-cplusplus11-and-later
class BindGuard {
 public:
  BindGuard(const IBindable* binder);

  BindGuard(const BindGuard&) = delete;
  BindGuard& operator=(const BindGuard&) = delete;

  // Move-constructible.
  BindGuard(BindGuard&& other) noexcept;
  // Not move-assignable.
  BindGuard& operator=(BindGuard&&) = delete;

  ~BindGuard();

 private:
  const IBindable* binder_;
};
}  // namespace GLOO

#endif
