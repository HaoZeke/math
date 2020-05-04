#ifndef STAN_MATH_OPENCL_KERNEL_GENERATOR_BROADCAST_HPP
#define STAN_MATH_OPENCL_KERNEL_GENERATOR_BROADCAST_HPP
#ifdef STAN_OPENCL

#include <stan/math/opencl/matrix_cl_view.hpp>
#include <stan/math/prim/meta.hpp>
#include <stan/math/opencl/kernel_generator/type_str.hpp>
#include <stan/math/opencl/kernel_generator/name_generator.hpp>
#include <stan/math/opencl/kernel_generator/operation_cl.hpp>
#include <stan/math/opencl/kernel_generator/as_operation_cl.hpp>
#include <stan/math/opencl/kernel_generator/is_valid_kernel_expression.hpp>
#include <limits>
#include <string>
#include <type_traits>
#include <set>
#include <utility>

namespace stan {
namespace math {

/** \addtogroup opencl_kernel_generator
 *  @{
 */

/**
 * Represents a broadcasting operation in kernel generator expressions.
 * @tparam T type of arguments
 * @tparam Colwise whether to broadcast colwise
 * @tparam Rowwise whether to broadcast rowwise
 */
template <typename T, bool Colwise, bool Rowwise>
class broadcast_
    : public operation_cl<broadcast_<T, Colwise, Rowwise>,
                          typename std::remove_reference_t<T>::Scalar, T> {
 public:
  using Scalar = typename std::remove_reference_t<T>::Scalar;
  using base = operation_cl<broadcast_<T, Colwise, Rowwise>, Scalar, T>;
  using base::var_name;

  /**
   * Constructor
   * @param a expression
   */
  explicit broadcast_(T&& a) : base(std::forward<T>(a)) {
    const char* function = "broadcast";
    if (Colwise) {
      check_size_match(function, "Rows of ", "a", a.rows(), "", "", 1);
    }
    if (Rowwise) {
      check_size_match(function, "Columns of ", "a", a.cols(), "", "", 1);
    }
  }

  /**
   * Creates a deep copy of this expression.
   * @return copy of \c *this
   */
  inline auto deep_copy() const {
    auto&& arg_copy = this->template get_arg<0>().deep_copy();
    return broadcast_<std::remove_reference_t<decltype(arg_copy)>, Colwise,
                      Rowwise>{std::move(arg_copy)};
  }

  /**
   * Generates kernel code for this and nested expressions.
   * @param var_name_arg name of the variable in kernel that holds argument to
   * this expression
   * @param i row index variable name
   * @param j column index variable name
   * @param view_handled whether whether caller already handled matrix view
   * @return part of kernel with code for this and nested expressions
   */
  inline kernel_parts generate(const std::string& i, const std::string& j,
                               const bool view_handled,
                               const std::string& var_name_arg) const {
    var_name = this->template get_arg<0>().var_name;
    return {};
  }

  /**
   * Sets index/indices along broadcasted dimmension(s) to 0.
   * @param[in, out] i row index
   * @param[in, out] j column index
   */
  inline void modify_argument_indices(std::string& i, std::string& j) const {
    if (Colwise) {
      i = "0";
    }
    if (Rowwise) {
      j = "0";
    }
  }

  /**
   * Number of rows of a matrix that would be the result of evaluating this
   * expression.
   * @return number of rows
   */
  inline int rows() const {
    return Colwise ? base::dynamic : this->template get_arg<0>().rows();
  }

  /**
   * Number of columns of a matrix that would be the result of evaluating this
   * expression.
   * @return number of columns
   */
  inline int cols() const {
    return Rowwise ? base::dynamic : this->template get_arg<0>().cols();
  }

  /**
   * Determine indices of extreme sub- and superdiagonals written.
   * @return pair of indices - bottom and top diagonal
   */
  inline std::pair<int, int> extreme_diagonals() const {
    int bottom, top;
    std::pair<int, int> arg_diags
        = this->template get_arg<0>().extreme_diagonals();
    return {Colwise ? std::numeric_limits<int>::min() : arg_diags.first,
            Rowwise ? std::numeric_limits<int>::max() : arg_diags.second};
  }
};

/**
 * Broadcast an expression in specified dimension(s). If broadcasting rowwise,
 * the argument must have a single column. If broadcasting colwise, the argument
 * must have a single row. Further expressions can use this expression as if it
 * had any size in broadcasted dimension, repeating the values.
 *
 * Broadcasting evaluates the argument expression multiple times. For
 * performance reasons don't broadcast slow operations. Instead evaluate them in
 * a separate kernel.
 * @tparam Colwise whether to broadcast Colwise
 * @tparam Rowwise whether to broadcast Rowwise
 * @tparam T type of input expression
 * @param a input expression
 * @return broadcast expression
 */
template <bool Colwise, bool Rowwise, typename T,
          typename = require_all_valid_expressions_and_none_scalar_t<T>>
inline broadcast_<as_operation_cl_t<T>, Colwise, Rowwise> broadcast(T&& a) {
  auto&& a_operation = as_operation_cl(std::forward<T>(a)).deep_copy();
  return broadcast_<as_operation_cl_t<T>, Colwise, Rowwise>(
      std::move(a_operation));
}

/**
 * Broadcast an expression in rowwise dimmension. The argument must have single
 * column. Further expressions can use this expression as if it had any number
 * of columns, repeating the values.
 *
 * Broadcasting evaluates argument expression multiple times. For performance
 * reasons don't broadcast slow operations. Instead evaluate them in a separate
 * kernel.
 * @tparam T type of input expression
 * @param a input expression
 * @return broadcast expression
 */
template <typename T,
          typename = require_all_valid_expressions_and_none_scalar_t<T>>
inline auto rowwise_broadcast(T&& a) {
  return broadcast<false, true>(std::forward<T>(a));
}

/**
 * Broadcast an expression in colwise dimmension. The argument must have single
 * row. Further expressions can use this expression as if it had any number of
 * rows, repeating the values.
 *
 * Broadcasting evaluates argument expression multiple times. For performance
 * reasons don't broadcast slow operations. Instead evaluate them in a separate
 * kernel.
 * @tparam T type of input expression
 * @param a input expression
 * @return broadcast expression
 */
template <typename T,
          typename = require_all_valid_expressions_and_none_scalar_t<T>>
inline auto colwise_broadcast(T&& a) {
  return broadcast<true, false>(std::forward<T>(a));
}
/** @}*/
}  // namespace math
}  // namespace stan
#endif
#endif
