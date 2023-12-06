#include "BindGuard.hpp"

#include <type_traits>

namespace GLOO {
BindGuard::BindGuard(const IBindable* binder) : binder_(binder) {
  binder_->Bind();
}

BindGuard::BindGuard(BindGuard&& other) noexcept {
  binder_ = other.binder_;
  other.binder_ = nullptr;
}

BindGuard::~BindGuard() {
  if (binder_ != nullptr) {
    binder_->Unbind();
  }
}

static_assert(std::is_move_constructible<BindGuard>(), "");

static_assert(!std::is_copy_constructible<BindGuard>(), "");
static_assert(!std::is_copy_assignable<BindGuard>(), "");
static_assert(!std::is_move_assignable<BindGuard>(), "");
}  // namespace GLOO
