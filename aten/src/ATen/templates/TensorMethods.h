#pragma once

#include "ATen/core/Tensor.h"
#include "ATen/core/Scalar.h"
#include "ATen/core/SparseTensorRef.h"
#include "ATen/core/Type.h"
#include "ATen/core/TensorOptions.h"

namespace at {

inline Tensor Tensor::toType(const Type & t, bool non_blocking) const {
  if(type() == t)
    return *this;
  return t.copy(*this, non_blocking);
}

inline Tensor Tensor::cpu() const {
  return toType(type().cpu());
}

inline Tensor Tensor::cuda() const {
  return toType(type().cuda());
}

inline Tensor & Tensor::copy_(const Tensor & src, bool non_blocking) {
  return type().copy_(*this, src, non_blocking);
}

inline Tensor Tensor::toType(ScalarType t) const {
  return toType(type().toScalarType(t));
}

inline Tensor Tensor::toBackend(Backend b) const {
  return toType(type().toBackend(b));
}

inline TensorOptions Tensor::options() const {
  return TensorOptions().dtype(dtype())
                        .device(device())
                        .layout(layout())
                        .is_variable(is_variable());
}

inline void Tensor::backward(
    at::optional<Tensor> gradient,
    bool keep_graph,
    bool create_graph) {
  type().backward(*this, std::move(gradient), keep_graph, create_graph);
}

inline void Tensor::set_data(Tensor new_data) {
  type().set_data(*this, new_data);
}

// all static inline to allow for inlining of the non-dynamic part of dispatch
${tensor_method_definitions}

#define DEFINE_CAST(T, name, _)                  \
  template <>                                    \
  inline T* Tensor::data() const {               \
    AT_CHECK(                                    \
        type().scalarType() == ScalarType::name, \
        "expected scalar type ",                 \
        #name,                                   \
        " but found ",                           \
        at::toString(type().scalarType()));      \
    return static_cast<T*>(this->data_ptr());    \
  }                                              \
  inline T* Tensor::to##name##Data() const {     \
    return data<T>();                            \
  }

AT_FORALL_SCALAR_TYPES_WITH_COMPLEX_EXCEPT_COMPLEX_HALF(DEFINE_CAST)
#undef DEFINE_CAST

#define DEFINE_TO_C_TYPE(T,name,_) \
inline T Tensor::toC##name () const { return _local_scalar().to##name (); }

AT_FORALL_SCALAR_TYPES_WITH_COMPLEX_EXCEPT_COMPLEX_HALF(DEFINE_TO_C_TYPE)
#undef DEFINE_TO_C_TYPE

} //namespace at
