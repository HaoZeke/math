#ifndef STAN_MATH_OPENCL_PRIM_FUN_INV_SQUARE_HPP
#define STAN_MATH_OPENCL_PRIM_FUN_INV_SQUARE_HPP
#ifdef STAN_OPENCL

#include <stan/math/opencl/matrix_cl.hpp>
#include <stan/math/opencl/kernel_generator.hpp>

namespace stan {
namespace math {

/**
 * Return the elementwise `1 / square(x)` of the specified kernel generator
 * expression.
 *
 * @tparam T_x type of input kernel generator expression x
 * @param x input kernel generator expression
 * @return inverse square of each value in x.
 */
template <typename T_x,
          typename = require_all_valid_kernel_expressions_and_none_scalar_t<T_x>>
inline auto inv_square(T_x&& x) {  // NOLINT
  return elewise_division(
      1.0, elewise_multiplication(std::forward<T_x>(x), std::forward<T_x>(x)));
}
}  // namespace math
}  // namespace stan

#endif
#endif
