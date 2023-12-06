#ifndef GLOO_I_BINDABLE_H_
#define GLOO_I_BINDABLE_H_

namespace GLOO {
class IBindable {
 public:
  virtual ~IBindable() {
  }
  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;
};
}  // namespace GLOO

#endif
