//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Vicente J. Botet Escriba 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/fixed_point for documentation.
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file
 * @brief Defines the fixed point number facilities.
 *
 */

#ifndef BOOST_FIXED_POINT_NUMBER_HPP
#define BOOST_FIXED_POINT_NUMBER_HPP

#include <boost/mpl/logical.hpp>
#include <boost/mpl/comparison.hpp>
#include <boost/mpl/max.hpp>
#include <boost/mpl/min.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/common_type.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_signed.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/integer.hpp>
#include <boost/integer/static_log2.hpp>
#include <boost/ratio/detail/mpl/abs.hpp>
#include <limits>
#include <stdexcept>
#include <cmath>
#include <boost/integer_traits.hpp>

#include <boost/config.hpp>
//#include <boost/fixed_point/config.hpp>
//#include <boost/fixed_point/number_fwd.hpp>
//#include <boost/fixed_point/round/nearest_odd.hpp>
//#include <boost/fixed_point/overflow/exception.hpp>
//#include <boost/fixed_point/detail/helpers.hpp>

#include <limits>

namespace boost
{
  namespace fixed_point
  {
    //#include <boost/fixed_point/detail/helpers.hpp>
    namespace detail
    {

      template <typename From, typename To, bool IsPositive = (To::resolution_exp > 0)>
      struct shift_impl;
      template <typename From, typename To>
      struct shift_impl<From, To, true>
      {
        //BOOST_STATIC_ASSERT(From::digits>To::resolution_exp);
        BOOST_STATIC_CONSTEXPR
        std::size_t digits = From::digits - To::resolution_exp;
        typedef typename From::underlying_type result_type;
        static result_type apply(typename From::underlying_type v)
        {
          return v >> To::resolution_exp;
        }
      };
      template <typename From, typename To>
      struct shift_impl<From, To, false>
      {
        BOOST_STATIC_CONSTEXPR
        std::size_t digits = From::digits - To::resolution_exp;
        typedef typename ::boost::int_t<digits>::fast result_type;
        //typedef typename From::underlying_type result_type;
        static result_type apply(typename From::underlying_type v)
        {
          return result_type(v) << -To::resolution_exp;
        }
      };

      template <typename From, typename To>
      typename shift_impl<From, To>::result_type shift(typename From::underlying_type v)
      {
        return shift_impl<From, To>::apply(v);
      }

      template <int amt, typename T>
      T shift_left(T val)
      {
        if (amt > 0)
        {
          unsigned int u_amt = amt;
          return val << u_amt;
        }
        else
        {
          unsigned int u_amt = -amt;
          return val >> u_amt;
        }
      }

      template <int amt, typename T>
      T shift_right(T val)
      {
        if (amt > 0)
        {
          unsigned int u_amt = amt;
          return val >> u_amt;
        }
        else
        {
          unsigned int u_amt = -amt;
          return val << u_amt;
        }
      }

      template <bool IsSigned>
      struct max_type;
      template <>
      struct max_type<true>
      {
        typedef boost::intmax_t type;
      };
      template <>
      struct max_type<false>
      {
        typedef boost::uintmax_t type;
      };

    }

    //#include <boost/fixed_point/overflow/exceptions.hpp>
    /**
     * Exception throw when there is a positive overflow.
     */
    class positive_overflow: public std::overflow_error
    {
    public:
      positive_overflow() :
        std::overflow_error("FixedPoint: positive overflow")
      {
      }
      explicit positive_overflow(const std::string& what_arg) :
        std::overflow_error(what_arg)
      {
      }
      explicit positive_overflow(const char* what_arg) :
        std::overflow_error(what_arg)
      {
      }
    };

    /**
     * Exception throw when there is a negative overflow.
     */
    class negative_overflow: public std::underflow_error
    {
    public:
      negative_overflow() :
        std::underflow_error("FixedPoint: negative overflow")
      {
      }
      explicit negative_overflow(const std::string& what_arg) :
        std::underflow_error(what_arg)
      {
      }
      explicit negative_overflow(const char* what_arg) :
        std::underflow_error(what_arg)
      {
      }
    };

    namespace round
    {
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      /**
       * When the computation is not exact, rounding will be to one of the two nearest representable values.
       * The algorithm for choosing between these values is the rounding mode.
       * Different applications desire different modes, so programmers may specify its own rounding mode.
       * However the library provides the usual rounding policies.
       * All of them follwos the following this stereotype
       *
       */

      struct stereotype
      {
        BOOST_STATIC_CONSTEXPR
        std::float_round_style round_style;
        template <typename From, typename To>
        static typename To::underlying_type round(From const& rhs);
        template <typename To, typename From>
        static typename To::underlying_type round_divide(From const& lhs, From const& rhs);
      };
#endif
      /**
       *  Speed is more important than the choice in value.
       */
      struct fastest
      {
        BOOST_STATIC_CONSTEXPR
        std::float_round_style round_style = std::round_indeterminate;
      };

      /**
       * Rounds toward negative infinity.
       *
       * This mode is useful in interval arithmetic.
       */
      struct negative
      {
        BOOST_STATIC_CONSTEXPR
        std::float_round_style round_style = std::round_toward_neg_infinity;

        template <typename From, typename To>
        static typename To::underlying_type round_integral(From const& rhs)
        {
          BOOST_STATIC_CONSTEXPR boost::uintmax_t d = To::resolution_exp;
          typedef typename detail::max_type<is_signed<typename To::underlying_type>::value>::type tmp_type;
          BOOST_STATIC_ASSERT(d < (8 * sizeof(tmp_type)));

          tmp_type res = tmp_type(rhs) >> d;
          return res;
        }

        template <typename From, typename To>
        static typename To::underlying_type round_float_point(From const& rhs)
        {
          return To::integer_part(rhs / To::template factor<From>());
        }

        template <typename From, typename To>
        static typename To::underlying_type round(From const& rhs)
        {
          BOOST_STATIC_CONSTEXPR boost::uintmax_t d = To::resolution_exp-From::resolution_exp;
          typedef typename detail::max_type<is_signed<typename To::underlying_type>::value>::type tmp_type;
          BOOST_STATIC_ASSERT(d < (8 * sizeof(tmp_type)));
          //BOOST_MPL_ASSERT_MSG(d<(8*sizeof(tmp_type)), OVERFLOW, (mpl::int_<8*sizeof(tmp_type)>, mpl::int_<d>));

          tmp_type res = tmp_type(rhs.count()) >> d;
          BOOST_ASSERT(res <= To::max_index);
          BOOST_ASSERT(res >= To::min_index);
          return res;
        }
        template <typename To, typename From>
        static typename To::underlying_type round_divide(From const& lhs, From const& rhs)
        {
          typedef typename detail::shift_impl<From, To>::result_type result_type;
          result_type ci = detail::shift<From, To>(lhs.count()) / rhs.count();
          if (ci >= 0)
          {
            BOOST_ASSERT(ci <= To::max_index);
            BOOST_ASSERT(ci >= To::min_index);
            return ci;
          }
          else
          {
            result_type ri = detail::shift<From, To>(lhs.count()) % rhs.count();
            if (ri == 0)
            {
              BOOST_ASSERT(ci <= To::max_index);
              BOOST_ASSERT(ci >= To::min_index);
              return ci;
            }
            else
            {
              BOOST_ASSERT(ci - 1 <= To::max_index);
              BOOST_ASSERT(ci >= (To::min_index + 1));
              return ci - 1;
            }
          }
        }
      };
      /**
       * Rounds toward zero.
       *
       * This mode is useful in implementing integral arithmetic.
       */
      struct truncated
      {
        BOOST_STATIC_CONSTEXPR
        std::float_round_style round_style = std::round_toward_zero;

        template <typename From, typename To>
        static typename To::underlying_type round_integral(From const& rhs)
        {
          BOOST_STATIC_CONSTEXPR boost::uintmax_t d = To::resolution_exp;
          typedef typename detail::max_type<is_signed<typename To::underlying_type>::value>::type tmp_type;
          BOOST_STATIC_ASSERT(d < (8 * sizeof(tmp_type)));

          tmp_type m( ( (rhs > 0) ? rhs : -rhs));
          tmp_type s( ( (rhs > 0) ? +1 : -1));

          tmp_type res = s * (m >> d);
          return res;
        }

        template <typename From, typename To>
        static typename To::underlying_type round_float_point(From const& rhs)
        {
          return To::integer_part(rhs / To::template factor<From>());
        }

        template <typename From, typename To>
        static typename To::underlying_type round(From const& rhs)
        {
          BOOST_STATIC_CONSTEXPR boost::uintmax_t d = To::resolution_exp-From::resolution_exp;
          typedef typename detail::max_type<is_signed<typename To::underlying_type>::value>::type tmp_type;
          BOOST_STATIC_ASSERT(d < (8 * sizeof(tmp_type)));

          tmp_type m( ( (rhs.count() > 0) ? rhs.count() : -rhs.count()));
          tmp_type s( ( (rhs.count() > 0) ? +1 : -1));

          tmp_type res = s * (m >> d);
          BOOST_ASSERT(res <= To::max_index);
          BOOST_ASSERT(res >= To::min_index);
          return res;
        }
        template <typename To, typename From>
        static typename To::underlying_type round_divide(From const& lhs, From const& rhs)
        {
          typedef typename detail::shift_impl<From, To>::result_type result_type;
          result_type ci = detail::shift<From, To>(lhs.count()) / rhs.count();
          BOOST_ASSERT(ci <= To::max_index);
          BOOST_ASSERT(ci >= To::min_index);
          return ci;
        }
      };
      /**
       * Rounds toward positive infinity.
       *
       * This mode is useful in interval arithmetic.
       */
      struct positive
      {
        BOOST_STATIC_CONSTEXPR
        std::float_round_style round_style = std::round_toward_infinity;

        template <typename From, typename To>
        static typename To::underlying_type round_integral(From const& rhs)
        {
          BOOST_STATIC_CONSTEXPR boost::uintmax_t d = To::resolution_exp;
          typedef typename detail::max_type<is_signed<typename To::underlying_type>::value>::type tmp_type;
          BOOST_STATIC_ASSERT(d < (8 * sizeof(tmp_type)));

          BOOST_STATIC_CONSTEXPR tmp_type w = (1<<d)-1;
          tmp_type i = rhs;

          BOOST_ASSERT(i <= (integer_traits<tmp_type>::const_max - w));

          tmp_type res = (i + w) >> d;
          return res;
        }

        template <typename From, typename To>
        static typename To::underlying_type round_float_point(From const& rhs)
        {
          return To::integer_part(rhs / To::template factor<From>());
        }

        template <typename From, typename To>
        static typename To::underlying_type round(From const& rhs)
        {
          BOOST_STATIC_CONSTEXPR boost::uintmax_t d = To::resolution_exp-From::resolution_exp;
          typedef typename detail::max_type<is_signed<typename To::underlying_type>::value>::type tmp_type;
          BOOST_STATIC_ASSERT(d < (8 * sizeof(tmp_type)));

          BOOST_STATIC_CONSTEXPR tmp_type w = (1<<d)-1;
          tmp_type i = rhs.count();

          BOOST_ASSERT(i <= (integer_traits<tmp_type>::const_max - w));

          tmp_type res = (i + w) >> d;
          BOOST_ASSERT(res <= To::max_index);
          BOOST_ASSERT(res >= To::min_index);
          return res;
        }
        template <typename To, typename From>
        static typename To::underlying_type round_divide(From const& lhs, From const& rhs)
        {
          typedef typename detail::shift_impl<From, To>::result_type result_type;
          result_type ci = detail::shift<From, To>(lhs.count()) / rhs.count();
          if (ci >= 0)
          {
            result_type ri = detail::shift<From, To>(lhs.count()) % rhs.count();
            if (ri == 0)
            {
              BOOST_ASSERT(ci <= To::max_index);
              BOOST_ASSERT(ci >= To::min_index);
              return ci;
            }
            else
            {
              BOOST_ASSERT(ci <= To::max_index - 1);
              BOOST_ASSERT(ci + 1 >= To::min_index);
              return ci + 1;
            }
          }
          else
          {
            BOOST_ASSERT(ci <= To::max_index);
            BOOST_ASSERT(ci >= To::min_index);
            return ci;
          }
        }
      };
      /**
       * Round towards the nearest value, but exactly-half values are rounded towards maximum magnitude.
       *
       * This mode is the standard school algorithm.
       */
      struct nearest_half_up
      {
        BOOST_STATIC_CONSTEXPR
        std::float_round_style round_style = std::round_to_nearest;
      };
      /**
       * Rounds to nearest half down.
       */
      struct nearest_half_down
      {
        BOOST_STATIC_CONSTEXPR
        std::float_round_style round_style = std::round_to_nearest;
      };
      /**
       * Round towards the nearest value, but exactly-half values are rounded towards even values.
       * This mode has more balance than the classic mode.
       */
      struct nearest_even
      {
        BOOST_STATIC_CONSTEXPR
        std::float_round_style round_style = std::round_to_nearest;
      };
      /**
       * Round towards the nearest value, but exactly-half values are rounded towards odd values.
       * This mode has as much balance as the near_even mode, but preserves more information.
       */
      struct nearest_odd
      {
        BOOST_STATIC_CONSTEXPR
        std::float_round_style round_style = std::round_to_nearest;
      };
    }

    namespace overflow
    {

#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      /**
       * Since the range of intermediate values grow to hold all possible values, and variables have a static range and
       * resolution, construction and assignment may need to reduce the range and resolution.
       * Reducing the resolution is done with a rounding mode associated with the variable.
       * When the dynamic value exceeds the range of variable, the assignment overflows.
       *
       * When an overflow does occur, the desirable behavior depends on the application, so programmers may specify the
       * overflow mode with his own specific overflow policy. The library provides however the usual ones.
       * All of them follows the following stereotype
       */
      struct stereotype
      {
        BOOST_STATIC_CONSTEXPR
        bool is_modulo;

        template <typename T, typename U>
        static BOOST_CONSTEXPR
        typename T::underlying_type
        on_negative_overflow(U value);

        template <typename T, typename U>
        static BOOST_CONSTEXPR
        typename T::underlying_type
        on_positive_overflow(U value);
      };
#endif
      /**
       * Programmer analysis of the program has determined that overflow cannot occur.
       * Uses of this mode should be accompanied by an argument supporting the conclusion.
       *
       * An assertion is raised on debug mode.
       */
      struct impossible
      {
        BOOST_STATIC_CONSTEXPR
        bool is_modulo = false;

        template <typename T, typename U>
        static BOOST_CONSTEXPR typename T::underlying_type on_negative_overflow(U value)
        {
#if defined(BOOST_NO_CONSTEXPR)
          BOOST_ASSERT_MSG(false,"Negative overflow while trying to convert fixed point numbers");
#endif
          return value;
        }

        template <typename T, typename U>
        static BOOST_CONSTEXPR typename T::underlying_type on_positive_overflow(U value)
        {
#if defined(BOOST_NO_CONSTEXPR)
          BOOST_ASSERT_MSG(false,"Positive overflow while trying to convert fixed point numbers");
#endif
          return value;
        }
      };
      /**
       * Programmers are willing to accept undefined behavior in the event of an overflow.
       */
      struct undefined
      {
        BOOST_STATIC_CONSTEXPR
        bool is_modulo = false;

        template <typename T, typename U>
        static BOOST_CONSTEXPR typename T::underlying_type on_negative_overflow(U value)
        {
          return value;
        }

        template <typename T, typename U>
        static BOOST_CONSTEXPR typename T::underlying_type on_positive_overflow(U value)
        {
          return value;
        }
      };
#if ! defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      namespace detail
      {
        template <typename T, typename U, bool TisSigned = T::is_signed>
        struct modulus_on_negative_overflow;

        template <typename T, typename U>
        struct modulus_on_negative_overflow<T, U, false>
        {
          static BOOST_CONSTEXPR typename T::underlying_type value(U value)
          {
            return (value%(T::max_index-T::min_index+1))+(T::max_index-T::min_index+1);
          }
        };

        template <typename T, typename U>
        struct modulus_on_negative_overflow<T, U, true>
        {
          static BOOST_CONSTEXPR typename T::underlying_type value(U value)
          {
            return ((value-T::min_index)%(T::max_index-T::min_index+1))-T::min_index;
          }
        };

        template <typename T, typename U, bool TisSigned = T::is_signed>
        struct modulus_on_positive_overflow;

        template <typename T, typename U>
        struct modulus_on_positive_overflow<T, U, true>
        {
          static BOOST_CONSTEXPR typename T::underlying_type value(U value)
          {
            return ((value-T::max_index)%(T::max_index-T::min_index+1))-T::max_index;
          }
        };
        template <typename T, typename U>
        struct modulus_on_positive_overflow<T, U, false>
        {
          static BOOST_CONSTEXPR typename T::underlying_type value(U value)
          {
            return value%(T::max_index-T::min_index+1);
          }
        };
      }
#endif

      /**
       * The assigned value is the dynamic value @c mod the range of the variable.
       * This mode makes sense only with unsigned numbers. It is useful for angular measures.
       */
      struct modulus
      {
        BOOST_STATIC_CONSTEXPR
        bool is_modulo = true;

        template <typename T, typename U>
        static BOOST_CONSTEXPR typename T::underlying_type on_negative_overflow(U val)
        {
          return detail::modulus_on_negative_overflow<T,U>::value(val);
        }

        template <typename T, typename U>
        static BOOST_CONSTEXPR typename T::underlying_type modulus_on_positive_overflow(U val)
        {
          return detail::modulus_on_negative_overflow<T,U>::value(val);
        }
      };
      /**
       * If the dynamic value exceeds the range of the variable, assign the nearest representable value.
       */
      struct saturate
      {
        BOOST_STATIC_CONSTEXPR
        bool is_modulo = false;

        template <typename T, typename U>
        static BOOST_CONSTEXPR typename T::underlying_type on_negative_overflow(U )
        {
          return T::min_index;
        }

        template <typename T, typename U>
        static BOOST_CONSTEXPR typename T::underlying_type on_positive_overflow(U )
        {
          return T::max_index;
        }

      };
      /**
       * If the dynamic value exceeds the range of the variable, throw an exception of derived from std::overflow_error.
       */
      struct exception
      {
        BOOST_STATIC_CONSTEXPR
        bool is_modulo = false;
        template <typename T, typename U>
        static typename T::underlying_type on_negative_overflow(U)
        {
          throw negative_overflow();
        }
        template <typename T, typename U>
        static typename T::underlying_type on_positive_overflow(U)
        {
          throw positive_overflow();
        }

      };
    }
  } // namespace fixed_point

  template <>
  struct common_type<fixed_point::round::truncated, fixed_point::round::truncated>
  {
    typedef fixed_point::round::truncated type;
  };
  template <typename Round>
  struct common_type<Round, fixed_point::round::truncated>
  {
    typedef fixed_point::round::truncated type;
  };
  template <typename Round>
  struct common_type<fixed_point::round::truncated, Round>
  {
    typedef fixed_point::round::truncated type;
  };

  namespace fixed_point
  {

    /**
     * Namespace for storage policies.
     */
    namespace storage
    {

      /**
       * Every storage policy must define two meta-functions <c>signed_integer_type<Range, Resolution>::type</c> and
       * <c>unsigned_integer_type<Range, Resolution>::type</c>.
       */
      struct stereotype
      {
        /**
         * Gets the signed integer type with enough bits to manage with
         * the Range and Resolution.
         */
        template <int Range, int Resolution>
        struct signed_integer_type;

        /**
         * Gets the unsigned integer type with enough bits to manage with
         * the Range and Resolution
         */
        template <int Range, int Resolution>
        struct unsigned_integer_type;
      };

      /**
       * The storage is undefined.
       */
      struct undefined
      {
        /**
         * signed_integer_type: Gets the signed integer type with enough bits to manage with
         * the Range and Resolution depending on the F
         */
        template <int Range, int Resolution>
        struct signed_integer_type
        {
          typedef typename ::boost::int_t<Range - Resolution + 1>::least type;
        };

        /**
         * unsigned_integer_type: Gets the unsigned integer type with enough bits to manage with
         * the Range and Resolution depending on the F
         */
        template <int Range, int Resolution>
        struct unsigned_integer_type
        {
          typedef typename ::boost::uint_t<Range - Resolution>::least type;
        };
      };
      /**
       * The storage is chosen to be least wide possible.
       */
      struct space
      {
        template <int Range, int Resolution>
        struct signed_integer_type
        {
          typedef typename ::boost::int_t<Range - Resolution + 1>::least type;
        };
        template <int Range, int Resolution>
        struct unsigned_integer_type
        {
          typedef typename ::boost::uint_t<Range - Resolution>::least type;
        };
      };
      /**
       * The storage is chosen to be fastest possible.
       */
      struct speed
      {
        template <int Range, int Resolution>
        struct signed_integer_type
        {
          typedef typename ::boost::int_t<Range - Resolution + 1>::fast type;
        };
        template <int Range, int Resolution>
        struct unsigned_integer_type
        {
          typedef typename ::boost::uint_t<Range - Resolution>::fast type;
        };

      };
    }
  } // namespace fixed_point

  template <>
  struct common_type<fixed_point::overflow::undefined, fixed_point::overflow::modulus>
  {
    typedef fixed_point::overflow::modulus type;
  };
  template <>
  struct common_type<fixed_point::overflow::modulus, fixed_point::overflow::undefined>
  {
    typedef fixed_point::overflow::modulus type;
  };

  template <>
  struct common_type<fixed_point::overflow::saturate, fixed_point::overflow::undefined>
  {
    typedef fixed_point::overflow::saturate type;
  };
  template <>
  struct common_type<fixed_point::overflow::undefined, fixed_point::overflow::saturate>
  {
    typedef fixed_point::overflow::saturate type;
  };

  template <>
  struct common_type<fixed_point::overflow::saturate, fixed_point::overflow::modulus>
  {
    typedef fixed_point::overflow::exception type;
  };
  template <>
  struct common_type<fixed_point::overflow::modulus, fixed_point::overflow::saturate>
  {
    typedef fixed_point::overflow::exception type;
  };
  template <>
  struct common_type<fixed_point::overflow::exception, fixed_point::overflow::exception>
  {
    typedef fixed_point::overflow::exception type;
  };
  template <typename Overflow>
  struct common_type<fixed_point::overflow::exception, Overflow>
  {
    typedef fixed_point::overflow::exception type;
  };
  template <typename Overflow>
  struct common_type<Overflow, fixed_point::overflow::exception>
  {
    typedef fixed_point::overflow::exception type;
  };

  template <>
  struct common_type<fixed_point::overflow::impossible, fixed_point::overflow::impossible>
  {
    typedef fixed_point::overflow::impossible type;
  };
  template <typename Overflow>
  struct common_type<fixed_point::overflow::impossible, Overflow>
  {
    typedef Overflow type;
  };
  template <typename Overflow>
  struct common_type<Overflow, fixed_point::overflow::impossible>
  {
    typedef Overflow type;
  };

  namespace fixed_point
  {
    /**
     * Since fixed points have different range and resolution the user needs to convert from one type to another.
     *
     * When the target type has a larger range and a more precise resolution than the source type, the conversion is implicit.
     * Otherwise, as the conversion could loss information, the conversion should be explicit to be safe.
     * Anyway some domains could consider that working with fixed-points should mimic the builtin and expect this conversion to be implicit.
     * If for this reason that the library manage with both cases via the conversion policy.
     *
     * The conversion from builtins arithmetic types suffer from the same loss of information issue but
     * some users could find an implicit conversion more natural.
     *
     * Conversions to builtins arithmetic types is a different concern.
     * There is no know way to enable conversions operator subject to conditions on the type.
     * The library has taken a conservative approach and only explicit conversions are provided.
     * The user could always wrap the type and provide implicit conversion.
     *
     * Note that there is no common_type between explicitly and implicitly.
     *
     */
    namespace conversion
    {
      /**
       * Used to state that a conversion needs to be explicit.
       */
      struct explicitly
      {
      };
      /**
       * Used to state that a conversion needs to be implicit.
       */
      struct implicitly
      {
      };
    }
  } // namespace fixed_point

  namespace fixed_point
  {
    /**
     * Namespace for arithmetic operations policies.
     *
     * The common_type is open if one of the is closed, open otherwise.
     */
    namespace arithmetic
    {
      /*
       * The range and resolution of the result of basic operations are deduced to try to hold the mathematical results.
       * This deduction depends on the bound policy.
       *
       * - unbounded: The range and resolution of the result of basic operations are large enough to hold the mathematical results.
       *
       * Overflow in template argument computation is undefined behavior.
       * In practice, overflow is unlikely to be a significant problem because even small machines can represent
       * numbers with thousands of bits and because compiler can diagnose overflow in template arguments.
       *
       * The special case in the operations is division, where the mathematical result may require an infinite
       * number of bits. The actual value must be rounded to a representable value.
       * The above resolution is sufficient to ensure that if the mathematical result is not zero, the fixed-point
       * result is not zero.
       * Furthermore, assuming values have an error of one-half ULP, the defined resolution is close to the error
       * bound in the computation.
       *
       * - bounded: As far as the result type is large enough to hold mathematical results it behaves as the unbounded one.
       * When the bounding type is not enough large the operation is undefined.
       * The user need to use functions that have the expected result type as parameter.
       *
       * Overflow while computing the arithmetic operations can be detected in this bounded cases.
       *
       */
      struct open
      {
      };
      /*
       * The range and resolution of the result is the one of the argument operations.
       * In order to mix different fixed points, the user could be forced to convert explicitly the arguments to the expected type.
       */
      struct closed
      {
      };
    }
  } // namespace fixed_point

  template <>
  struct common_type<fixed_point::arithmetic::open, fixed_point::arithmetic::closed>
  {
    typedef fixed_point::arithmetic::open type;
  };
  template <>
  struct common_type<fixed_point::arithmetic::closed, fixed_point::arithmetic::open>
  {
    typedef fixed_point::arithmetic::open type;
  };

  namespace fixed_point
  {
    /**
     * Namespace for bounding policies.
     *
     * The common_type is unbounded if one of them is unbounded, bounded otherwise.
     *
     */
    namespace bound
    {
      /**
       * Bounded fixed points types are closed, that is that the result of an arithmetic operations will be
       * closed only if both arguments are closed.
       * The range and resolution are bounded by the larger integral type provided by the compiler.
       *
       */
      struct bounded
      {
      };

      /**
       * The range and resolution of the result of basic operations are large enough to hold the mathematical results.
       */
      struct unbounded
      {
      };
    }
  } // namespace fixed_point

  template <>
  struct common_type<fixed_point::bound::unbounded, fixed_point::bound::bounded>
  {
    typedef fixed_point::bound::unbounded type;
  };
  template <>
  struct common_type<fixed_point::bound::bounded, fixed_point::bound::unbounded>
  {
    typedef fixed_point::bound::unbounded type;
  };

  namespace fixed_point
  {
    /**
     * Namespace for bounding policies.
     */

    template <typename Storage = storage::space, typename ConversionFp = conversion::explicitly,
        typename ConversionBuilt = conversion::explicitly, typename Arithmetic = arithmetic::open,
        typename Bound = bound::bounded>
    struct family
    {
      typedef Storage storage_type;
      typedef ConversionFp conversion_from_fixed_point_type;
      typedef ConversionBuilt conversion_from_builtin_type;
      typedef Arithmetic arithmetic_type;
      typedef Bound bound_type;

    };
  } // namespace fixed_point

  //  template <int R1, int P1, typename RP1, typename OP1, typename F1,
  //  int R2, int P2, typename RP2, typename OP2, typename F2>
  //  struct common_type<
  //  fixed_point::ureal_t<R1,P1,RP1,OP1,F1>,
  //  fixed_point::ureal_t<R2,P2,RP2,OP2,F2> >
  //  {
  //    typedef fixed_point::ureal_t<
  //    mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value,
  //    mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
  //    typename common_type<RP1,RP2>::type,
  //    typename common_type<OP1,OP2>::type,
  //    typename common_type<F1,F2>::type
  //    > type;
  //  };

  namespace fixed_point
  {
    template <typename F>
    struct allows_explicit_conversion_from_fp: public is_same<typename F::conversion_from_fixed_point_type,
        conversion::explicitly>
    {
    };
    template <typename F>
    struct allows_explicit_conversion_from_builtin: public is_same<typename F::conversion_from_builtin_type,
        conversion::explicitly>
    {
    };
    template <typename F>
    struct allows_implicit_conversion_from_fp: public is_same<typename F::conversion_from_fixed_point_type,
        conversion::implicitly>
    {
    };
    template <typename F>
    struct allows_implicit_conversion_from_builtin: public is_same<typename F::conversion_from_builtin_type,
        conversion::implicitly>
    {
    };
    template <typename F>
    struct is_bounded: public is_same<typename F::bound_type, bound::bounded>
    {
    };
    template <typename F>
    struct is_open: public is_same<typename F::arithmetic_type, arithmetic::open>
    {
    };
    template <typename F>
    struct is_closed: public is_same<typename F::arithmetic_type, arithmetic::closed>
    {
    };

    template <int Range, int Resolution, typename Rounding = round::negative, typename Overflow = overflow::exception,
        typename Family = family<> >
    class ureal_t;

    template <int Range, int Resolution, typename Rounding = round::negative, typename Overflow = overflow::exception,
        typename Family = family<> >
    class real_t;

    template <typename Res, int R1, int P1, typename RP1, typename OP1, typename F1, int R2, int P2, typename RP2,
        typename OP2, typename F2>
    inline Res
    divide(real_t<R1, P1, RP1, OP1, F1> const& lhs, real_t<R2, P2, RP2, OP2, F2> const& rhs);
    template <typename Res, int R1, int P1, typename RP1, typename OP1, typename F1, int R2, int P2, typename RP2,
        typename OP2, typename F2>
    inline Res
    divide(real_t<R1, P1, RP1, OP1, F1> const& lhs, real_t<R2, P2, RP2, OP2, F2> const& rhs);

    template <typename Res, int R1, int P1, typename RP1, typename OP1, typename F1, int R2, int P2, typename RP2,
        typename OP2, typename F2>
    inline Res
    divide(real_t<R1, P1, RP1, OP1, F1> const& lhs, ureal_t<R2, P2, RP2, OP2, F2> const& rhs);

    template <typename Res, int R1, int P1, typename RP1, typename OP1, typename F1, int R2, int P2, typename RP2,
        typename OP2, typename F2>
    inline Res
    divide(ureal_t<R1, P1, RP1, OP1, F1> const& lhs, real_t<R2, P2, RP2, OP2, F2> const& rhs);

  }
}

#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
#define BOOST_TYPEOF_SILENT
#include <boost/typeof/typeof.hpp>

#include BOOST_TYPEOF_INCREMENT_REGISTRATION_GROUP()

BOOST_TYPEOF_REGISTER_TEMPLATE(boost::fixed_point::real_t, (int)(int)(typename)(typename)(typename))
BOOST_TYPEOF_REGISTER_TEMPLATE(boost::fixed_point::ureal_t, (int)(int)(typename)(typename)(typename))

#endif

///////////////////////////////////////

namespace boost
{
  namespace fixed_point
  {

    /**
     *  named parameter like class, allowing to make a specific overload when the integer must be taken by the index.
     */
    template <typename T>
    struct index_tag
    {
      typedef T type;
      T value;
      BOOST_CONSTEXPR index_tag(T v) : value(v)
      {}
      BOOST_CONSTEXPR T get()
      { return value;}

    };

    /**
     *  Helper function to make easier the use of @c index_tag.
     *  @returns @c index_tag<T>(v).
     */
    template <typename T>
    BOOST_CONSTEXPR index_tag<T> index(T v)
    { return index_tag<T>(v);}

    /**
     *  named parameter like class, allowing to make a implicit conversions.
     */
    template <typename T>
    struct convert_tag
    {
      typedef T type;
      T value;
      BOOST_CONSTEXPR convert_tag(T &v) : value(v)
      {}
      BOOST_CONSTEXPR T& get()
      { return value;}

    };

    /**
     *  Helper function to make easier the use of @c convert_tag.
     *  @returns @c convert_tag<T>(v).
     */
    template <typename T>
    BOOST_CONSTEXPR convert_tag<T> convert(T v)
    { return convert_tag<T>(v);}

    /**
     * Explicit conversion between fixed_point numbers.
     *
     * @TParams
     * @Param{From,the source type of the conversion}
     * @Param{To,the target type of the conversion}
     *
     * @Params
     * @Param{from,the number from which the conversion is done}
     *
     * @Requires @c From is a fixed_point number or a builtin type and @c To is a fixed_point number.
     * @Returns the conversion with possible reduced range and loss of resolution.
     *
     */
    template <class From, class To>
    To number_cast(From const& from);

    //#include <boost/fixed_point/detail/helpers.hpp>
    namespace detail
    {
      template <typename T, int Range, int Resolution >
      struct signed_integer_traits
      {
        BOOST_STATIC_CONSTEXPR std::size_t digits = (Range-Resolution)+1;
        BOOST_STATIC_ASSERT_MSG((sizeof(T)*8)>=digits, "LLLL");
        //BOOST_MPL_ASSERT_MSG((sizeof(T)*8)>=digits, LLLL, (mpl::int_<sizeof(T)*8>, mpl::int_<digits>));
        BOOST_STATIC_CONSTEXPR T const_max = (1LL<<(digits-1)) - 1;
        BOOST_STATIC_CONSTEXPR T const_min = -const_max;

      };
      template <typename T, int Range, int Resolution >
      struct unsigned_integer_traits
      {
        BOOST_STATIC_CONSTEXPR std::size_t digits = (Range-Resolution);
        BOOST_STATIC_CONSTEXPR T const_max = (1LL<<(digits)) - 1;
        BOOST_STATIC_CONSTEXPR T const_min = 0;

      };

      //      template <int Range, int Resolution, typename Family=family::space>
      //      class signed_uniform_quantizer
      //      {
      //        BOOST_MPL_ASSERT_MSG(Range>=Resolution, RANGE_MUST_BE_GREATER_EQUAL_THAN_RESOLUTION, (mpl::int_<Range>,mpl::int_<Resolution>));
      //      public:
      //
      //        //! The underlying integer type
      //        typedef typename Family::template  signed_integer_type<Range,Resolution>::type underlying_type;
      //
      //        // name the template parameters
      //        BOOST_STATIC_CONSTEXPR int range_exp = Range;
      //        BOOST_STATIC_CONSTEXPR int resolution_exp = Resolution;
      //        BOOST_STATIC_CONSTEXPR int digits = range_exp-resolution_exp+1;
      //
      //        typedef Family family_type;
      //
      //        BOOST_STATIC_CONSTEXPR underlying_type min_index = detail::signed_integer_traits<underlying_type,Range,Resolution>::const_min;
      //        BOOST_STATIC_CONSTEXPR underlying_type max_index = detail::signed_integer_traits<underlying_type,Range,Resolution>::const_max;
      //
      //        //! conversion factor.
      //        template <typename FP>
      //        static FP factor()
      //        {
      //          if (Resolution>=0) return FP(1 << Resolution);
      //          else return FP(1)/(1 << -Resolution);
      //        }
      //        template <typename FP>
      //        static underlying_type integer_part(FP x)
      //        {
      //          return underlying_type(std::floor(x));
      //        }
      //      };
      //
      //
      //      template <typename Final, int Range, int Resolution, typename Rounding=round::negative, typename Overflow=overflow::exception,
      //          typename Family=family::space
      //          >
      //      class signed_quantizer : public signed_uniform_quantizer<Range,Resolution,Family>
      //      {
      //        typedef signed_uniform_quantizer<Range,Resolution,Family> base_type;
      //      public:
      //        typedef typename base_type::underlying_type underlying_type;
      //        BOOST_STATIC_CONSTEXPR underlying_type min_index = base_type::const_min;
      //        BOOST_STATIC_CONSTEXPR underlying_type max_index = base_type::const_max;
      //
      //        template <typename FP>
      //        static FP reconstruct(underlying_type k)
      //        {
      //          BOOST_ASSERT(min_index <= k && k <= max_index);
      //
      //          return Rounding::reconstruct(k, base_type::template factor<FP>());
      //        }
      //        template <typename FP>
      //        static underlying_type classify(FP x)
      //        {
      //          if (x<Final::min().template as<FP>()) {
      //            return Overflow::on_negative_overflow(min_index,x);
      //          }
      //          if (x>Final::max().template as<FP>()) {
      //            return Overflow::on_positive_overflow(max_index,x);
      //          }
      //          return Rounding::classify(x, base_type::template factor<FP>());
      //        }
      //        template <typename FP>
      //        static Final cast(FP x)
      //        {
      //          fixed_point::number_cast<Final>(x);
      //        }
      //      };

      template <
      typename From,
      typename To,
      bool LE_Range= From::range_exp <= To::range_exp,
      bool GE_Resolution= From::resolution_exp >= To::resolution_exp
      > struct fxp_number_cast;

      // LE_Range=true GE_Resolution=true
      ///////////////////////////////////
      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      real_t<R1,P1,RP1,OP1,F1>,
      real_t<R2,P2,RP2,OP2,F2>,
      true, true >
      {
        typedef real_t<R1,P1,RP1,OP1,F1> From;
        typedef real_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // No overflow and no round needed
          return To(index(underlying_type(rhs.count()) << (P1-P2)));
        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      ureal_t<R1,P1,RP1,OP1,F1>,
      real_t<R2,P2,RP2,OP2,F2>,
      true, true >
      {
        typedef ureal_t<R1,P1,RP1,OP1,F1> From;
        typedef real_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // No overflow and no round needed
          return To(index(underlying_type(rhs.count()) << (P1-P2)));
        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      ureal_t<R1,P1,RP1,OP1,F1>,
      ureal_t<R2,P2,RP2,OP2,F2>,
      true, true >
      {
        typedef ureal_t<R1,P1,RP1,OP1,F1> From;
        typedef ureal_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // No overflow and no round needed
          return To(index(underlying_type(rhs.count()) << (P1-P2)));
        }
      };

      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      real_t<R1,P1,RP1,OP1,F1>,
      ureal_t<R2,P2,RP2,OP2,F2>,
      true, true >
      {
        typedef real_t<R1,P1,RP1,OP1,F1> From;
        typedef ureal_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          //          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
          //          // Overflow
          //          if (indx < To::min_index)
          //            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          //          else // No round needed
          //            return To(index(underlying_type(rhs.count()) << (P1-P2)));

          return
          (
              (((underlying_type(rhs.count()) << (P1-P2))) < To::min_index)
              ? To(index(
                      OP2::template on_negative_overflow<To,underlying_type>(
                          ((underlying_type(rhs.count()) << (P1-P2)))
                      )
                  ))
              : To(index(underlying_type(rhs.count()) << (P1-P2)))
          );

        }
      };

      // LE_Range=false GE_Resolution=true
      ////////////////////////////////////
      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      real_t<R1,P1,RP1,OP1,F1>,
      real_t<R2,P2,RP2,OP2,F2>,
      false, true >
      {
        typedef real_t<R1,P1,RP1,OP1,F1> From;
        typedef real_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          //          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
          //          // Overflow impossible
          //          if (indx > To::max_index)
          //            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          //          else if (indx < To::min_index)
          //            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          //          else // No round needed
          //            return To(index(indx));

          return
          (
              (((underlying_type(rhs.count()) << (P1-P2))) > To::max_index)
              ? To(index(
                      OP2::template on_positive_overflow<To,underlying_type>(
                          ((underlying_type(rhs.count()) << (P1-P2)))
                      )
                  ))
              : (
                  (((underlying_type(rhs.count()) << (P1-P2))) < To::min_index)
                  ? To(index(
                          OP2::template on_negative_overflow<To,underlying_type>(
                              ((underlying_type(rhs.count()) << (P1-P2)))
                          )
                      ))
                  : To(index(((underlying_type(rhs.count()) << (P1-P2)))))
              )
          );
        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      real_t<R1,P1,RP1,OP1,F1>,
      ureal_t<R2,P2,RP2,OP2,F2>,
      false, true >
      {
        typedef real_t<R1,P1,RP1,OP1,F1> From;
        typedef ureal_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {

          //          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
          //          // Overflow impossible
          //          if (indx > To::max_index)
          //            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          //          else if (indx < To::min_index)
          //            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          //          else // No round needed
          //            return To(index(indx));

          return (
              (((underlying_type(rhs.count()) << (P1-P2))) > To::max_index)
              ? To(index(
                      OP2::template on_positive_overflow<To,underlying_type>(
                          ((underlying_type(rhs.count()) << (P1-P2)))
                      )
                  ))
              : (
                  (((underlying_type(rhs.count()) << (P1-P2))) < To::min_index)
                  ? To(index(
                          OP2::template on_negative_overflow<To,underlying_type>(
                              ((underlying_type(rhs.count()) << (P1-P2)))
                          )
                      ))
                  : To(index(((underlying_type(rhs.count()) << (P1-P2)))))
              )
          );
        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      ureal_t<R1,P1,RP1,OP1,F1>,
      ureal_t<R2,P2,RP2,OP2,F2>,
      false, true >
      {
        typedef ureal_t<R1,P1,RP1,OP1,F1> From;
        typedef ureal_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {

          //          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
          //          // Overflow
          //          if (indx > To::max_index)
          //            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          //          else // No round needed
          //            return To(index(indx));
          //
          return
          (
              (((underlying_type(rhs.count()) << (P1-P2))) > To::max_index)
              ? To(index(
                      OP2::template on_positive_overflow<To,underlying_type>(
                          ((underlying_type(rhs.count()) << (P1-P2)))
                      )
                  ))
              : To(index(((underlying_type(rhs.count()) << (P1-P2)))))
          );
        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      ureal_t<R1,P1,RP1,OP1,F1>,
      real_t<R2,P2,RP2,OP2,F2>,
      false, true >
      {
        typedef ureal_t<R1,P1,RP1,OP1,F1> From;
        typedef real_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {

          //          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
          //          // Overflow
          //          if (indx > To::max_index)
          //            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          //          else // No round needed
          //            return To(index(indx));

          return
          (
              (((underlying_type(rhs.count()) << (P1-P2))) > To::max_index)
              ? To(index(
                      OP2::template on_positive_overflow<To,underlying_type>(((underlying_type(rhs.count()) << (P1-P2))))
                  ))
              : To(index(((underlying_type(rhs.count()) << (P1-P2)))))
          );

        }
      };

      // LE_Range=true GE_Resolution=false
      ////////////////////////////////////
      template <int R, int P1, typename RP1, typename OP1, typename F1,
      int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      real_t<R,P1,RP1,OP1,F1>,
      real_t<R,P2,RP2,OP2,F2>,
      true, false >
      {
        typedef real_t<R,P1,RP1,OP1,F1> From;
        typedef real_t<R,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          //          // Overflow could be possible because more resolution implies a bigger range when the range exponents are the same.
          //          underlying_type indx(((rhs.count()) >> (P2-P1)));
          //          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          //            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          //          else if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
          //            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          //          else // Round
          //          To res((index(RP2::template round<From,To>(rhs))));
          //          return res;

          return
          (
              (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
              ? To(index(OP2::template on_positive_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
              : (
                  (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
                  ? To(index(OP2::template on_negative_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
                  : To((index(RP2::template round<From,To>(rhs))))
              )
          );
        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      real_t<R1,P1,RP1,OP1,F1>,
      real_t<R2,P2,RP2,OP2,F2>,
      true, false >
      {
        typedef real_t<R1,P1,RP1,OP1,F1> From;
        typedef real_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // No overflow check needed as the case for the same range exponent is explicit above
          // Round
          return To(index(RP2::template round<From,To>(rhs)));
        }
      };

      ////

      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      real_t<R1,P1,RP1,OP1,F1>,
      ureal_t<R2,P2,RP2,OP2,F2>,
      true, false >
      {
        typedef real_t<R1,P1,RP1,OP1,F1> From;
        typedef ureal_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          //          // Overflow
          //          underlying_type indx(((rhs.count()) >> (P2-P1)));
          //          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          //            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          //          else if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
          //            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          //          else // Round
          //          return To((index(RP2::template round<From,To>(rhs))));

          return
          (
              (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
              ? To(index(
                      OP2::template on_positive_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))
                  ))
              : (
                  (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
                  ? To(index(
                          OP2::template on_negative_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))
                      ))
                  : To((index(RP2::template round<From,To>(rhs))))
              )
          );
        }
      };

      ////
      template <int R, int P1, typename RP1, typename OP1, typename F1,
      int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      ureal_t<R,P1,RP1,OP1,F1>,
      real_t<R,P2,RP2,OP2,F2>,
      true, false >
      {
        typedef ureal_t<R,P1,RP1,OP1,F1> From;
        typedef real_t<R,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          //          // Overflow could be possible because more resolution implies a bigger range when the range exponents are the same.
          //          underlying_type indx(((rhs.count()) >> (P2-P1)));
          //          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          //            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          //          else // Round
          //          return To((index(RP2::template round<From,To>(rhs))));

          return
          (
              (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
              ? To(index(OP2::template on_positive_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
              : To((index(RP2::template round<From,To>(rhs))))
          );

        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      ureal_t<R1,P1,RP1,OP1,F1>,
      real_t<R2,P2,RP2,OP2,F2>,
      true, false >
      {
        typedef ureal_t<R1,P1,RP1,OP1,F1> From;
        typedef real_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // No overflow check needed as the case for the same range exponent is explicit above

          // Round
          return To(index(RP2::template round<From,To>(rhs)));
        }
      };

      ////

      template <int R, int P1, typename RP1, typename OP1, typename F1,
      int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      ureal_t<R,P1,RP1,OP1,F1>,
      ureal_t<R,P2,RP2,OP2,F2>,
      true, false >
      {
        typedef ureal_t<R,P1,RP1,OP1,F1> From;
        typedef ureal_t<R,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          //          // Overflow could be possible because more resolution implies a bigger range when the range exponents are the same.
          //          underlying_type indx(((rhs.count()) >> (P2-P1)));
          //          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          //            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          //          else // Round
          //            return To((index(RP2::template round<From,To>(rhs))));

          return
          (
              (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
              ? To(index(OP2::template on_positive_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
              : To((index(RP2::template round<From,To>(rhs))))
          );

        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      ureal_t<R1,P1,RP1,OP1,F1>,
      ureal_t<R2,P2,RP2,OP2,F2>,
      true, false >
      {
        typedef ureal_t<R1,P1,RP1,OP1,F1> From;
        typedef ureal_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // No overflow check needed as the case for the same range exponent is explicit above
          // Round
          return To(index(RP2::template round<From,To>(rhs)));
        }
      };

      // LE_Range=false GE_Resolution=false
      /////////////////////////////////////

      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      real_t<R1,P1,RP1,OP1,F1>,
      real_t<R2,P2,RP2,OP2,F2>,
      false, false >
      {
        typedef real_t<R1,P1,RP1,OP1,F1> From;
        typedef real_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          //          // Overflow
          //          underlying_type indx(((rhs.count()) >> (P2-P1)));
          //          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          //            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          //          else if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
          //            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          //          else // Round
          //            return To(index(RP2::template round<From,To>(rhs)));

          return
          (
              (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
              ? To(index(OP2::template on_positive_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
              : (
                  (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
                  ? To(index(OP2::template on_negative_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
                  : To(index(RP2::template round<From,To>(rhs)))
              )
          );

        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      ureal_t<R1,P1,RP1,OP1,F1>,
      real_t<R2,P2,RP2,OP2,F2>,
      false, false >
      {
        typedef ureal_t<R1,P1,RP1,OP1,F1> From;
        typedef real_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          //          // Overflow
          //          underlying_type indx(((rhs.count()) >> (P2-P1)));
          //          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          //            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          //          else // Round
          //            return To(index(RP2::template round<From,To>(rhs)));

          return
          (
              (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
              ? To(index(OP2::template on_positive_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
              : To(index(RP2::template round<From,To>(rhs)))
          );

        }
      };

      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      real_t<R1,P1,RP1,OP1,F1>,
      ureal_t<R2,P2,RP2,OP2,F2>,
      false, false >
      {
        typedef real_t<R1,P1,RP1,OP1,F1> From;
        typedef ureal_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          //          // Overflow
          //          underlying_type indx(((rhs.count()) >> (P2-P1)));
          //          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          //            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          //          else if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
          //            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          //          else // Round
          //            return To(index(RP2::template round<From,To>(rhs)));


          return
          (
              (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
              ? To(index(OP2::template on_positive_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
              : (
                  (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
                  ? To(index(OP2::template on_negative_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
                  : To(index(RP2::template round<From,To>(rhs)))
              )
          );

        }
      };

      template <int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2>
      struct fxp_number_cast<
      ureal_t<R1,P1,RP1,OP1,F1>,
      ureal_t<R2,P2,RP2,OP2,F2>,
      false, false >
      {
        typedef ureal_t<R1,P1,RP1,OP1,F1> From;
        typedef ureal_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          //          // Overflow
          //          underlying_type indx(((rhs.count()) >> (P2-P1)));
          //          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          //            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          //          else if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
          //            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          //          else  // Round
          //            return To(index(RP2::template round<From,To>(rhs)));

          return
          (
              (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
              ? To(index(OP2::template on_positive_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
              : (
                  (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
                  ? To(index(OP2::template on_negative_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
                  : To(index(RP2::template round<From,To>(rhs)))
              )
          );

        }
      };

      // arthm_number_cast
      /////////////////////////////////////

      template <
      typename From,
      typename To,
      bool IsIntegral = is_integral<From>::value
      >
      struct arthm_number_cast;

      template <
      typename From,
      int R2, int P2, typename RP2, typename OP2, typename F2
      >
      struct arthm_number_cast<From,real_t<R2,P2,RP2,OP2,F2>,true>
      {
        typedef real_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          return
          To(index(detail::shift_left<-To::resolution_exp>(rhs)));
        }
      };
      template <
      typename From,
      int R2, int P2, typename RP2, typename OP2, typename F2
      >
      struct arthm_number_cast<From,ureal_t<R2,P2,RP2,OP2,F2>,true>
      {
        typedef ureal_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          return
          To(index(detail::shift_left<-To::resolution_exp>(rhs)));
        }
      };
      template <
      typename From,
      int R2, int P2, typename RP2, typename OP2, typename F2
      >
      struct arthm_number_cast<From,real_t<R2,P2,RP2,OP2,F2>,false>
      {

        typedef real_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          return
          To(index(To::classify(rhs)));
        }

      };
      template <
      typename From,
      int R2, int P2, typename RP2, typename OP2, typename F2
      >
      struct arthm_number_cast<From,ureal_t<R2,P2,RP2,OP2,F2>,false>
      {

        typedef ureal_t<R2,P2,RP2,OP2,F2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          return
          To(index(To::classify(rhs)));
        }

      };

      template <
      typename From,
      typename To,
      bool IsArithmetic= is_arithmetic<From>::value
      > struct number_cast;

      template <
      typename From,
      typename To
      > struct number_cast<From,To,true>: public arthm_number_cast<From,To>
      {};

      template <
      typename From,
      typename To
      > struct number_cast<From,To,false>: public fxp_number_cast<From,To>
      {};

    } // namespace detail
  } // namespace fixed_point

  /**
   * common_type specialization for ureal_t and ureal_t.
   */
  template <int R1, int P1, typename RP1, typename OP1, typename F1,
  int R2, int P2, typename RP2, typename OP2, typename F2>
  struct common_type<
  fixed_point::ureal_t<R1,P1,RP1,OP1,F1>,
  fixed_point::ureal_t<R2,P2,RP2,OP2,F2> >
  {
    typedef fixed_point::ureal_t<
    mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value,
    mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
    typename common_type<RP1,RP2>::type,
    typename common_type<OP1,OP2>::type,
    typename common_type<F1,F2>::type
    > type;
  };

  /**
   * common_type specialization for real_t and real_t.
   */
  template <int R1, int P1, typename RP1, typename OP1, typename F1,
  int R2, int P2, typename RP2, typename OP2, typename F2>
  struct common_type<
  fixed_point::real_t<R1,P1,RP1,OP1,F1>,
  fixed_point::real_t<R2,P2,RP2,OP2,F2> >
  {
    typedef fixed_point::real_t<
    mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value,
    mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
    typename common_type<RP1,RP2>::type,
    typename common_type<OP1,OP2>::type,
    typename common_type<F1,F2>::type
    > type;
  };
  /**
   * common_type specialization for real_t and ureal_t.
   */
  template <int R1, int P1, typename RP1, typename OP1, typename F1,
  int R2, int P2, typename RP2, typename OP2, typename F2>
  struct common_type<
  fixed_point::real_t<R1,P1,RP1,OP1,F1>,
  fixed_point::ureal_t<R2,P2,RP2,OP2,F2> >
  {
    typedef fixed_point::real_t<
    mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value,
    mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
    typename common_type<RP1,RP2>::type,
    typename common_type<OP1,OP2>::type,
    typename common_type<F1,F2>::type
    > type;
  };

  /**
   * common_type specialization for ureal_t and real_t.
   */
  template <int R1, int P1, typename RP1, typename OP1, typename F1,
  int R2, int P2, typename RP2, typename OP2, typename F2>
  struct common_type<
  fixed_point::ureal_t<R1,P1,RP1,OP1,F1>,
  fixed_point::real_t<R2,P2,RP2,OP2,F2> >
  {
    typedef fixed_point::real_t<
    mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value,
    mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
    typename common_type<RP1,RP2>::type,
    typename common_type<OP1,OP2>::type,
    typename common_type<F1,F2>::type
    > type;
  };

  namespace fixed_point
  {
    namespace detail
    {
      template <typename T, typename U>
      struct is_more_precisse;

      template <
      int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2
      >
      struct is_more_precisse<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> > :
      mpl::and_ <
      mpl::less_equal < mpl::int_<R2>, mpl::int_<R1> >,
      mpl::greater_equal < mpl::int_<P2>, mpl::int_<P1> >
      >
      {};

      template <
      int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2
      >
      struct is_more_precisse<real_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> > :
      mpl::and_ <
      mpl::less_equal < mpl::int_<R2>, mpl::int_<R1> >,
      mpl::greater_equal < mpl::int_<P2>, mpl::int_<P1> >
      >
      {};

      template <
      int R1, int P1, typename RP1, typename OP1, typename F1,
      int R2, int P2, typename RP2, typename OP2, typename F2
      >
      struct is_more_precisse<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> > :
      mpl::and_ <
      mpl::less_equal < mpl::int_<R2>, mpl::int_<R1> >,
      mpl::greater_equal < mpl::int_<P2>, mpl::int_<P1> >
      >
      {};
    }

    template <typename S, typename T>
    struct is_convertible : detail::is_more_precisse<T,S>
    {};

    template <typename S, typename T, bool IsArithmetic=is_arithmetic<S>::value >
    struct is_explicitly_convertible;
    template <typename S, typename T >
    struct is_explicitly_convertible<S,T,false> :
    mpl::and_<
    mpl::not_<allows_implicit_conversion_from_fp<T> >,
    mpl::not_<detail::is_more_precisse<T,S> >
    >
    {};

    template <typename S, typename T>
    struct is_explicitly_convertible<S,T,true> :
    allows_explicit_conversion_from_builtin<T>
    {};

    template <typename S, typename T, bool IsArithmetic=is_arithmetic<S>::value >
    struct is_implicitly_convertible;
    template <typename S, typename T >
    struct is_implicitly_convertible<S,T,false> :
    mpl::and_<
    allows_implicit_conversion_from_fp<T>,
    mpl::not_<detail::is_more_precisse<T,S> >
    >
    {};

    template <typename S, typename T>
    struct is_implicitly_convertible<S,T,true> :
    allows_implicit_conversion_from_builtin<T>
    {};

    /**
     * @brief Signed fixed point number.
     *
     * @TParams
     * @Param{Range,Range specified by an integer. The range of a signed number x is 2^Range < x < 2^Range. Note that the range interval is open for signed numbers}
     * @Param{Resolution,resolution specified by an integer. The resolution of a fractional number x is 2^Resolution}
     * @Param{Rounding,The rounding policy}
     * @Param{Overflow,The overflow policy}
     * @Param{Family,The family traits}
     *
     * @Example For example, real_t<8,-4> has values n such that -256 < n < 256 in increments of 2^(-4) = 1/16.
     */
    template <int Range, int Resolution, typename Rounding, typename Overflow, typename Family>
    class real_t
    {
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      BOOST_MPL_ASSERT_MSG(Range>=Resolution,
          RANGE_MUST_BE_GREATER_EQUAL_THAN_RESOLUTION, (mpl::int_<Range>,mpl::int_<Resolution>));
#endif

    public:
      //! this type
      typedef real_t self_type;
      // name the template parameters
      //! the Range parameter.
      BOOST_STATIC_CONSTEXPR int range_exp = Range;
      //! the Resolution parameter.
      BOOST_STATIC_CONSTEXPR int resolution_exp = Resolution;
      //! the Rounding parameter.
      typedef Rounding rounding_type;
      //! the Overflow parameter.
      typedef Overflow overflow_type;
      //! the Family parameter.
      typedef Family family_type;

      //! the storage policy
      typedef typename family_type::storage_type storage_type;
      //! the conversion policy
      typedef typename family_type::conversion_from_fixed_point_type conversion_from_fixed_point_type;
      typedef typename family_type::conversion_from_builtin_type conversion_from_builtin_type;

      //! the arithmetic policy
      typedef typename family_type::arithmetic_type arithmetic_type;
      //! the arithmetic policy
      typedef typename family_type::bound_type bound_type;

      //! The underlying integer type
      typedef typename storage_type::template signed_integer_type<Range,Resolution>::type underlying_type;
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      BOOST_MPL_ASSERT_MSG((sizeof(underlying_type)*8) >= (Range-Resolution+1),
          UNDERLYING_TYPE_MUST_BE_LARGE_ENOUGH, (underlying_type));
      BOOST_MPL_ASSERT_MSG(boost::is_signed<underlying_type>::value,
          UNDERLYING_TYPE_MUST_BE_SIGNED, (underlying_type));
#endif

      //! whether the tyoe is signed (always true).
      BOOST_STATIC_CONSTEXPR bool is_signed = true;
      //! The standard std::number_traits<>::digits.
      BOOST_STATIC_CONSTEXPR std::size_t digits =
      detail::signed_integer_traits<underlying_type,Range,Resolution>::digits;
      //! The standard std::number_traits<>::min_index
      BOOST_STATIC_CONSTEXPR underlying_type min_index =
      detail::signed_integer_traits<underlying_type,Range,Resolution>::const_min;
      //! The standard std::number_traits<>::max_index
      BOOST_STATIC_CONSTEXPR underlying_type max_index =
      detail::signed_integer_traits<underlying_type,Range,Resolution>::const_max;

      // construct/copy/destroy:

      /**
       * Default constructor.
       */
      BOOST_CONSTEXPR real_t()
      {} // = default;

      /**
       * Copy constructor.
       */
      BOOST_CONSTEXPR real_t(real_t const& rhs) : value_(rhs.value_)
      {} // = default;

      /**
       * Implicit constructor from a real_t no larger range and no better resolution.
       *
       * @Params
       * @Param{rhs,a real_t with no larger range and no better resolution}
       *
       * @Effects Adapt the resolution of @c rhs to this resolution.
       * @Throws Nothing.
       * @Remark This overload participates in overload resolution only if the source @c is_convertible to the target.
       */
      template <int R, int P, typename RP, typename OP, typename F>
      real_t(real_t<R,P,RP,OP,F> const& rhs
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
          , typename boost::enable_if<is_convertible<real_t<R,P,RP,OP,F>, real_t > >::type* = 0
#endif
      )
      : value_(fixed_point::detail::fxp_number_cast<real_t<R,P,RP,OP,F>, real_t>()(rhs).count())
      {
      }
      /**
       * Implicit constructor from a ureal_t with no larger range and no better resolution.
       *
       * @Params
       * @Param{rhs,a ureal_t with no larger range and no better resolution}
       *
       * @Effects Adapt the resolution of @c rhs to this resolution.
       * @Throws Nothing.
       * @Remark This overload participates in overload resolution only if the source @c is_convertible to the target.
       */
      template <int R, int P, typename RP, typename OP, typename F>
      real_t(ureal_t<R,P,RP,OP,F> const& rhs
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
          , typename boost::enable_if<is_convertible<ureal_t<R,P,RP,OP,F>, real_t > >::type* = 0
#endif
      )
      : value_(fixed_point::detail::fxp_number_cast<ureal_t<R,P,RP,OP,F>, real_t>()(rhs).count())
      {
      }

      /**
       * Explicit constructor from a real_t with larger range or better resolution.
       *
       * @Params
       * @Param{rhs,a real_t with larger range and better resolution}
       *
       * @Effects Rounds and check overflow if needed.
       * @Throws Whatever the target overflow policy can throw.
       * @Remark This overload participates in overload resolution only if the source @c is_explicitly_convertible to the target.
       */
      template <int R, int P, typename RP, typename OP, typename F>
      explicit real_t(real_t<R,P,RP,OP,F> const& rhs
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
          , typename boost::enable_if <is_explicitly_convertible<real_t<R,P,RP,OP,F>,real_t> >::type* = 0
#endif
      )
      : value_(fixed_point::detail::fxp_number_cast<real_t<R,P,RP,OP,F>, real_t,
          mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >::value,
          mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >::value
          >()(rhs).count())
      {
      }

      /**
       * Constructor from a real_t with larger range or better resolution.
       *
       * @Params
       * @Param{rhs,a real_t with larger range and better resolution}
       *
       * @Effects Rounds and check overflow if needed.
       * @Throws Whatever the target overflow policy can throw.
       * @Remark This overload participates in overload resolution only if the source @c is_implicitly_convertible to the target.
       */
      template <int R, int P, typename RP, typename OP, typename F>
      real_t(real_t<R,P,RP,OP,F> const& rhs
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
          , typename boost::enable_if <is_implicitly_convertible<real_t<R,P,RP,OP,F>,real_t> >::type* = 0
#endif
      )
      : value_(fixed_point::detail::fxp_number_cast<real_t<R,P,RP,OP,F>, real_t,
          mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >::value,
          mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >::value
          >()(rhs).count())
      {
      }

      /**
       * Explicit constructor from a ureal_t with larger range or better resolution.
       *
       * @Params
       * @Param{rhs,a ureal_t with larger range and better resolution}
       *
       * @Throws Whatever the target overflow policy can throw.
       * @Effects Rounds and check overflow if needed.
       * @Remark This overload participates in overload resolution only if the source @c is_explicitly_convertible to the target.
       */
      template <int R, int P, typename RP, typename OP, typename F>
      explicit real_t(ureal_t<R,P,RP,OP,F> const& rhs
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
          , typename boost::enable_if <is_explicitly_convertible<ureal_t<R,P,RP,OP,F>,real_t> >::type* = 0
#endif
      )
      : value_(fixed_point::detail::fxp_number_cast<ureal_t<R,P,RP,OP,F>, real_t,
          mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >::value,
          mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >::value
          >()(rhs).count())
      {
      }

      /**
       * Constructor from a ureal_t with larger range or better resolution.
       *
       * @Params
       * @Param{rhs,a ureal_t with larger range and better resolution}
       *
       * @Effects Rounds and check overflow if needed.
       * @Throws Whatever the target overflow policy can throw.
       * @Remark This overload participates in overload resolution only if the source @c is_implicitly_convertible to the target.
       */
      template <int R, int P, typename RP, typename OP, typename F>
      real_t(ureal_t<R,P,RP,OP,F> const& rhs
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
          , typename boost::enable_if <is_implicitly_convertible<ureal_t<R,P,RP,OP,F>,real_t> >::type* = 0
#endif
      )
      : value_(fixed_point::detail::fxp_number_cast<ureal_t<R,P,RP,OP,F>, real_t,
          mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >::value,
          mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >::value
          >()(rhs).count())
      {
      }

      /**
       * Implicit constructor from a real_t wrapped by a convert_tag.
       *
       * @Params
       * @Param{rhs,a real_t wrapped by @c convert_tag }
       *
       * @Effects Rounds and check overflow if needed.
       * @Throws Whatever the target overflow policy can throw.
       * @Remark This overload participates in overload resolution only if the source @c is_implicitly_convertible to the target.
       */

      template <int R, int P, typename RP, typename OP, typename F>
      real_t(convert_tag<real_t<R,P,RP,OP,F> > rhs)
      : value_(fixed_point::detail::fxp_number_cast<real_t<R,P,RP,OP,F>, real_t,
          mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >::value,
          mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >::value
          >()(rhs.get()).count())
      {
      }
      /**
       * Implicit constructor from a ureal_t wrapped by a convert_tag.
       *
       * @Params
       * @Param{rhs,a ureal_t wrapped by @c convert_tag }
       *
       * @Effects Rounds and check overflow if needed.
       * @Throws Whatever the target overflow policy can throw.
       * @Remark This overload participates in overload resolution only if the source @c is_implicitly_convertible to the target.
       */
      template <int R, int P, typename RP, typename OP, typename F>
      real_t(convert_tag<ureal_t<R,P,RP,OP,F> > rhs)
      : value_(fixed_point::detail::fxp_number_cast<ureal_t<R,P,RP,OP,F>, real_t,
          mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >::value,
          mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >::value
          >()(rhs.get()).count())
      {
      }

      //! destructor
      //~real_t() {} //= default;


      /**
       * Explicit construction from an index.
       *
       * @Params
       * @Param{i,the index}
       *
       * @Requires <c>min_index<=i<=max_index</c>.
       */
      template <typename UT>
      BOOST_CONSTEXPR explicit real_t(index_tag<UT> i)
      : value_(i.get())
      {
#if defined(BOOST_NO_CONSTEXPR)
        BOOST_ASSERT(i.get()>=min_index);
        BOOST_ASSERT(i.get()<=max_index);
#endif
      }

      //observers

      /**
       * Underlying integer type observer.
       *
       * @Returns the underlying representation.
       */
      BOOST_CONSTEXPR underlying_type count() const
      { return value_;}

      /**
       * @Returns the absolute zero.
       */
      static BOOST_CONSTEXPR real_t zero()
      {
        return real_t(index(0));
      }
      /**
       * @Returns the minimal value that can be represented.
       */
      static BOOST_CONSTEXPR real_t min BOOST_PREVENT_MACRO_SUBSTITUTION ()
      {
        return real_t(index(min_index));
      }
      /**
       * @Returns the maximal value that can be represented.
       */
      static BOOST_CONSTEXPR real_t max BOOST_PREVENT_MACRO_SUBSTITUTION ()
      {
        return real_t(index(max_index));
      }

      /**
       * @Returns the integral part of the fixed point number.
       */
      underlying_type integral_part() const
      {
        return count() >> resolution_exp;
      }

      //! @Returns the conversion factor.
      template <typename FP>
      static FP factor()
      {
        if (Resolution>=0) return FP(detail::shift_left<Resolution>(1));
        else return FP(1)/(detail::shift_left<-Resolution>(1));

      }

      /**
       * Reconstructs a floating point type from the underlying type.
       */
      template <typename FP>
      static FP reconstruct(underlying_type k)
      {
        BOOST_ASSERT(min_index <= k && k <= max_index);

        return k*factor<FP>();
      }

      //! explicit conversion to FP.
      //! @Returns the @c FP represented by @c *this.
      template <typename FP>
      FP as() const
      {
        return reconstruct<FP>(this->value_);
      }
      //! explicit conversion to int.
      int as_int() const
      {
        return detail::shift_right<-Resolution>(value_);
      }
      //! explicit conversion to float.
      float as_float() const
      {
        return as<float>();
      }

      //! explicit conversion to double.
      double as_double() const
      {
        return as<double>();
      }

      //! explicit conversion to long double.
      long double as_long_double() const
      {
        return as<long double>();
      }

#ifndef BOOST_NO_EXPLICIT_CONVERSION_OPERATORS
      //! explicit conversion to float.
      explicit operator int() const
      {
        return as_int();
      }
      //! explicit conversion to float.
      explicit operator float() const
      {
        return as<float>();
      }
      //! explicit conversion to double.
      explicit operator double() const
      {
        return as<double>();
      }
      //! explicit conversion to long double.
      explicit operator long double() const
      {
        return as<long double>();
      }
#endif

      template <typename FP>
      static underlying_type integer_part(FP x)
      {
        return underlying_type(std::floor(x));
      }
      template <typename I>
      static underlying_type classify(I i
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
          , typename boost::enable_if<is_integral<I> >::type* = 0
#endif
      )
      {
        // Round
        underlying_type indx = rounding_type::template round_integral<I, self_type>(i);
        // Overflow
        if (indx > max_index)
        {
          return overflow_type::template on_positive_overflow<self_type,underlying_type>(indx);
        }
        if (indx < min_index)
        {
          return overflow_type::template on_negative_overflow<self_type,underlying_type>(indx);
        }

        return indx;
      }

      template <typename FP>
      static underlying_type classify(FP x
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
          , typename boost::enable_if<is_floating_point<FP> >::type* = 0
#endif
      )
      {

        // Round
        underlying_type indx = rounding_type::template round_float_point<FP,self_type>(x);
        // Overflow
        if (indx > max_index)
        {
          return overflow_type::template on_positive_overflow<self_type,underlying_type>(indx);
        }
        if (indx < min_index)
        {
          return overflow_type::template on_negative_overflow<self_type,underlying_type>(indx);
        }

        return indx;
      }

      /**
       * Implicit conversion from arithmetic @c T.
       */
      template <typename T>
      real_t(T x
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
          , typename boost::enable_if<is_implicitly_convertible<T, real_t> >::type* = 0
#endif
      ) : value_(fixed_point::detail::arthm_number_cast<T, real_t>()(x).count())
      {}
      //! explicit conversion from arithmetic @c T.
      template <typename T>
      explicit real_t(T x
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
          , typename boost::disable_if<is_implicitly_convertible<T ,real_t> >::type* = 0
#endif
      ) : value_(fixed_point::detail::arthm_number_cast<T, real_t>()(x).count())
      {}

      //! implicit conversion from int
      real_t(convert_tag<int> x) : value_(detail::shift_left<-Resolution>(x.get()))
      {}
      //! implicit conversion from float
      real_t(convert_tag<float> x) : value_(classify(x.get()))
      {}
      //! implicit conversion from double
      real_t(convert_tag<double> x) : value_(classify(x.get()))
      {}
      //! implicit conversion from long double
      real_t(convert_tag<long double> x) : value_(classify(x.get()))
      {}

      template <typename T>
      real_t& operator=(convert_tag<T> v)
      {
        *this=real_t(v);
        return *this;
      }

      // arithmetic

      /**
       * @Returns this instance.
       */
      real_t operator+() const
      {
        return *this;
      }
      /**
       * @Returns a new instance with the representation negated.
       */
      real_t operator-() const
      {
        // As the range is symmetric the type is preserved
        return real_t(index(-value_));
      }

      /**
       * @Effects Pre-increase this instance as if <c>*this+=1</c>
       * @Returns this instance.
       */
      real_t& operator++()
      {
        *this+=convert(1);
        return *this;
      }

      /**
       * @Effects Post-increase this instance as if <c>*this+=1</c>
       * @Returns a copy of this instance before increasing it.
       */
      real_t operator++(int)
      {
        real_t tmp=*this;
        *this+=convert(1);
        return tmp;
      }

      /**
       * @Effects Pre-decrease this instance as if <c>*this-=1</c>
       * @Returns this instance.
       */
      real_t& operator--()
      {
        *this-=convert(1);
        return *this;
      }

      /**
       * @Effects Post-decrease this instance as if <c>*this-=1</c>
       * @Returns a copy of this instance before decreasing it.
       */
      real_t operator--(int)
      {
        real_t tmp=*this;
        *this-=convert(1);
        return tmp;
      }

      /**
       * @Effects As if <c>number_cast<real_t>(*this+rhs)</c>
       * @Returns this instance.
       * @Throws Any exception the Overflow policy can throw.
       */
      real_t& operator += (real_t const& rhs)
      {
        real_t tmp = number_cast<real_t>(*this+rhs);
        value_ = tmp.count();
        return *this;
      }

      /**
       * @Effects As if <c>number_cast<real_t>(*this-rhs)</c>
       * @Returns this instance.
       * @Throws Any exception the Overflow policy can throw.
       */
      real_t& operator-=(const real_t& rhs)
      {
        real_t tmp = number_cast<real_t>(*this-rhs);
        value_ = tmp.count();
        return *this;
      }

      /**
       * @Effects As if <c>number_cast<real_t>(*this*rhs)</c>
       * @Returns this instance.
       * @Throws Any exception the Overflow policy can throw.
       */
      real_t& operator*=(const real_t& rhs)
      {
        real_t tmp = number_cast<real_t>((*this) * rhs);
        value_ = tmp.count();
        return *this;
      }

      /**
       * @Effects As if <c>divide<real_t>(*this,rhs)</c>
       * @Returns this instance.
       * @Throws Any exception the Overflow policy can throw.
       */
      real_t& operator/=(const real_t& rhs)
      {
        real_t tmp = divide<real_t>(*this , rhs);
        value_ = tmp.count();
        return *this;
      }

      /**
       * Virtual scaling.
       *
       * @Returns a new instance with the same data representation and with the range and resolution increased by @c N.
       */
      template <unsigned N>
      real_t<Range+N, Resolution+N, Rounding, Overflow, Family>
      virtual_scale() const
      {
        return real_t<Range+N, Resolution+N, Rounding, Overflow, Family>(index(count()));
      }

      /**
       * Scales up N bits.
       *
       * @Effects Scales up this instance as if <c>(*this)=(*this)*(2^N)</c>
       */
      template <unsigned N>
      void scale_up()
      {
        value_ <<= N;
      }

      /**
       * Scales up/down depending on the sign of @c N.
       *
       * @Effects Scales up this instance as if <c>(*this)*(2^N)</c>
       */
      template <int N, typename RP>
      void scale()
      {
        if (N>=0)
        {
          //value_ <<= N;
          value_ = detail::shift_left<N>(value_);
        }
        else
        {
          real_t tmp=
          divide<real_t<Range, Resolution, RP, Overflow, Family> >(*this,
              real_t<-N+1, -N, Rounding, Overflow, Family>(index(1)));
          value_ = tmp.count();
        }
      }

    protected:
      //! The representation.
      underlying_type value_;
    };

    /**
     * @brief Unsigned fixed point number.
     *
     * @TParams
     * @Param{Range,Range specified by an integer. The range of a signed number x is 0<=x<2^Range},
     * @Param{Resolution,resolution specified by an integer. The resolution of a fractional number x is 2^Resolution}
     * @Param{Rounding,The rounding policy}
     * @Param{Overflow,The overflow policy}
     * @Param{Family,The family policy}
     */
    template <int Range, int Resolution, typename Rounding, typename Overflow, typename Family>
    class ureal_t
    {
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      BOOST_MPL_ASSERT_MSG(Range>=Resolution, RANGE_MUST_BE_GREATER_EQUAL_THAN_RESOLUTION, (mpl::int_<Range>,mpl::int_<Resolution>));
#endif

    public:
      //! this type
      typedef ureal_t self_type;

      // name the template parameters
      //! the Range parameter.
      BOOST_STATIC_CONSTEXPR int range_exp = Range;
      //! the Resolution parameter.
      BOOST_STATIC_CONSTEXPR int resolution_exp = Resolution;
      //! the Rounding parameter.
      typedef Rounding rounding_type;
      //! the Overflow parameter.
      typedef Overflow overflow_type;
      //! the Family parameter.
      typedef Family family_type;

      //! the storage policy
      typedef typename family_type::storage_type storage_type;
      //! the conversion policy
      typedef typename family_type::conversion_from_fixed_point_type conversion_from_fixed_point_type;
      typedef typename family_type::conversion_from_builtin_type conversion_from_builtin_type;
      //! the arithmetic policy
      typedef typename family_type::arithmetic_type arithmetic_type;
      //! the arithmetic policy
      typedef typename family_type::bound_type bound_type;

      //! The underlying integer type
      typedef typename storage_type::template unsigned_integer_type<Range,Resolution>::type underlying_type;

#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      BOOST_MPL_ASSERT_MSG((sizeof(underlying_type)*8) >= (Range-Resolution),
          UNDERLYING_TYPE_MUST_BE_LARGE_ENOUGH, (underlying_type));
      BOOST_MPL_ASSERT_MSG(!boost::is_signed<underlying_type>::value,
          UNDERLYING_TYPE_MUST_BE_UNSIGNED, (underlying_type));
#endif

      //! whether the tyoe is signed (always @c false).
      BOOST_STATIC_CONSTEXPR bool is_signed = false;
      //! The standard std::number_traits<>::digits.
      BOOST_STATIC_CONSTEXPR std::size_t digits =
      detail::unsigned_integer_traits<underlying_type,Range,Resolution>::digits;
      //! The standard std::number_traits<>::min_index
      BOOST_STATIC_CONSTEXPR underlying_type min_index =
      detail::unsigned_integer_traits<underlying_type,Range,Resolution>::const_min;
      //! The standard std::number_traits<>::max_index
      BOOST_STATIC_CONSTEXPR underlying_type max_index =
      detail::unsigned_integer_traits<underlying_type,Range,Resolution>::const_max;

      // construct/copy/destroy:
      /**
       * Default constructor.
       */
      BOOST_CONSTEXPR ureal_t()
      {} // = default;
      /**
       * Copy constructor.
       */
      BOOST_CONSTEXPR ureal_t(ureal_t const& rhs) : value_(rhs.value_)
      {} // = default;


      //! implicit constructor from a ureal_t with no larger range and no better resolution
      template <int R, int P, typename RP, typename OP, typename F>
      ureal_t(ureal_t<R,P,RP,OP,F> const& rhs
          , typename boost::enable_if<is_convertible<ureal_t<R,P,RP,OP,F>, ureal_t > >::type* = 0
      )
      : value_(fixed_point::detail::fxp_number_cast<ureal_t<R,P,RP,OP,F>, ureal_t>()(rhs).count())
      {
      }

      //! explicit constructor from a ureal_t with larger range or better resolution
      template <int R, int P, typename RP, typename OP, typename F>
      explicit ureal_t(ureal_t<R,P,RP,OP,F> const& rhs
          , typename boost::enable_if<is_explicitly_convertible<ureal_t<R,P,RP,OP,F>, ureal_t > >::type* = 0
      )
      : value_(fixed_point::detail::fxp_number_cast<ureal_t<R,P,RP,OP,F>, ureal_t,
          mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >::value,
          mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >::value
          >()(rhs).count())
      {
      }

      //! constructor from a ureal_t with larger range or better resolution
      template <int R, int P, typename RP, typename OP, typename F>
      ureal_t(ureal_t<R,P,RP,OP,F> const& rhs
          , typename boost::enable_if<is_implicitly_convertible<ureal_t<R,P,RP,OP,F>, ureal_t > >::type* = 0
      )
      : value_(fixed_point::detail::fxp_number_cast<ureal_t<R,P,RP,OP,F>, ureal_t,
          mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >::value,
          mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >::value
          >()(rhs).count())
      {
      }

      //! implicit constructor from a ureal_t with larger range or better resolution
      template <int R, int P, typename RP, typename OP, typename F>
      ureal_t(convert_tag<ureal_t<R,P,RP,OP,F> > rhs)
      : value_(fixed_point::detail::fxp_number_cast<ureal_t<R,P,RP,OP,F>, ureal_t,
          mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >::value,
          mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >::value
          >()(rhs.get()).count())
      {
      }

      //! destructor
      //~ureal_t() {} //= default;


      /**
       * Explicit construction from an index.
       *
       * @Params
       * @Param{i,the index}
       *
       * @Requires <c>0<=i<=max_index</c>.
       */
      template <typename UT>
      explicit ureal_t(index_tag<UT> i) : value_(i.get())
      {
        //BOOST_ASSERT(i.get()>=min_index);
        BOOST_ASSERT(i.get()<=max_index);
      }

      // observers

      /**
       * Underlying integer type observer.
       *
       * @Returns the underlying representation.
       */
      BOOST_CONSTEXPR underlying_type count() const
      { return value_;}

      /**
       * @Returns the absolute zero.
       */
      static BOOST_CONSTEXPR ureal_t zero()
      {
        return ureal_t(index(0));
      }

      /**
       * @Returns the minimal value that can be represented.
       */
      static BOOST_CONSTEXPR ureal_t min BOOST_PREVENT_MACRO_SUBSTITUTION ()
      {
        return ureal_t(index(min_index));
      }

      /**
       * @Returns the maximal value that can be represented.
       */
      static BOOST_CONSTEXPR ureal_t max BOOST_PREVENT_MACRO_SUBSTITUTION ()
      {
        return ureal_t(index(max_index));
      }

      /**
       * @Returns the integral part of the fixed point number.
       */
      underlying_type integral_part() const
      {
        return count() >> resolution_exp;
      }

      //! conversion factor.
      template <typename FP>
      static FP factor()
      {
        if (Resolution>=0) return FP(detail::shift_left<Resolution>(1));
        else return FP(1)/(detail::shift_left<-Resolution>(1));
      }
      template <typename FP>
      static underlying_type integer_part(FP x)
      {
        return underlying_type(std::floor(x));
      }
      template <typename FP>
      static FP reconstruct(underlying_type k)
      {
        BOOST_ASSERT(min_index <= k && k <= max_index);

        return k*factor<FP>();
      }

      template <typename I>
      static underlying_type classify(I i
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
          , typename boost::enable_if<is_integral<I> >::type* = 0
#endif
      )
      {
        if (i<0)
        return overflow_type::template on_negative_overflow<self_type,underlying_type>(0);
        // Round
        underlying_type indx = rounding_type::template round_integral<I, self_type>(i);
        // Overflow
        if (indx > max_index)
        {
          return overflow_type::template on_positive_overflow<self_type,underlying_type>(indx);
        }

        return indx;
      }

      template <typename FP>
      static underlying_type classify(FP x
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
          , typename boost::enable_if<is_floating_point<FP> >::type* = 0
#endif
      )
      {
        if (x<0)
        return overflow_type::template on_negative_overflow<self_type,underlying_type>(0);

        // Round
        underlying_type indx = rounding_type::template round_float_point<FP,self_type>(x);
        // Overflow
        if (indx > max_index)
        {
          return overflow_type::template on_positive_overflow<self_type,underlying_type>(indx);
        }

        return indx;
      }

      //! implicit conversion from int
      template <typename T>
      ureal_t(T x,
          typename boost::enable_if<is_implicitly_convertible<T, ureal_t> >::type* = 0
      ) : value_(fixed_point::detail::arthm_number_cast<T, ureal_t>()(x).count())
      {}
      //! explicit conversion from int
      template <typename T>
      explicit ureal_t(T x
          , typename boost::disable_if<is_implicitly_convertible<T, ureal_t> >::type* = 0
      ) : value_(fixed_point::detail::arthm_number_cast<T, ureal_t>()(x).count())
      {}

      //! implicit conversion from int
      ureal_t(convert_tag<unsigned int> x) : value_(detail::shift_left<-Resolution>(x.get()))
      {}

      //! implicit conversion from float
      ureal_t(convert_tag<float> x) : value_(classify(x.get()))
      {}
      //! implicit conversion from double
      ureal_t(convert_tag<double> x) : value_(classify(x.get()))
      {}
      //! implicit conversion from long double
      ureal_t(convert_tag<long double> x) : value_(classify(x.get()))
      {}

      template <typename T>
      ureal_t& operator=(convert_tag<T> v)
      {
        *this=ureal_t(v);
        return *this;
      }

      //! explicit conversion to FP.
      template <typename FP>
      FP as() const
      {
        return reconstruct<FP>(this->value_);
      }
      //! explicit conversion to int.
      int as_unsigned_int() const
      {
        return detail::shift_right<-Resolution>(value_);
      }
      //! explicit conversion to float.
      float as_float() const
      {
        return as<float>();
      }

      //! explicit conversion to double.
      double as_double() const
      {
        return as<double>();
      }

      //! explicit conversion to long double.
      long double as_long_double() const
      {
        return as<long double>();
      }

#ifndef BOOST_NO_EXPLICIT_CONVERSION_OPERATORS      //! explicit conversion to float.
      explicit operator unsigned int() const
      {
        return as_unsigned_int());
      }
      //! explicit conversion to float.
      explicit operator float() const
      {
        return as<float>();
      }
      //! explicit conversion to double.
      explicit operator double() const
      {
        return as<double>();
      }
      //! explicit conversion to long double.
      explicit operator long double() const
      {
        return as<long double>();
      }
#endif

      // arithmetic

      /**
       * @Returns this instance.
       */
      ureal_t operator+() const
      {
        return *this;
      }
      /**
       * @Returns an instance of a signed fixed point number with
       * the representation the negation of the representation of this.
       */
      real_t<Range,Resolution,Rounding,Overflow,Family>
      operator-() const
      {
        return real_t<Range,Resolution,Rounding,Overflow,Family>(index(-value_));
      }

      /**
       * @Effects Pre-increase this instance as if <c>*this+=1</c>
       * @Returns <c>*this</c>.
       */
      ureal_t& operator++()
      {
        *this+=convert(1u);
        return *this;
      }
      /**
       * @Effects Post-increase this instance as if <c>*this+=1</c>
       * @Returns a copy of this instance before increasing it.
       */
      ureal_t operator++(int)
      {
        ureal_t tmp=*this;
        *this+=convert(1u);
        return tmp;
      }
      /**
       * @Effects Pre-decrease this instance as if <c>*this-=1</c>
       * @Returns <c>*this</c>.
       */
      ureal_t& operator--()
      {
        *this-=convert(1u);
        return *this;
      }
      /**
       * @Effects Post-decrease this instance as if <c>*this-=1</c>
       * @Returns a copy of this instance before decreasing it.
       */
      ureal_t operator--(int)
      {
        ureal_t tmp=*this;
        *this-=convert(1u);
        return tmp;
      }

      /**
       * @Effects As if <c>number_cast<ureal_t>(*this+rhs)</c>
       * @Returns <c>*this</c>.
       * @Throws Any exception the Overflow policy can throw.
       */
      ureal_t& operator += (ureal_t const& rhs)
      {
        ureal_t tmp = number_cast<ureal_t>((*this) + rhs);
        value_ = tmp.count();
        return *this;
      }

      /**
       * @Effects As if <c>number_cast<ureal_t>(*this-rhs)</c>
       * @Returns <c>*this</c>.
       * @Throws Any exception the Overflow policy can throw.
       */
      ureal_t& operator-=(ureal_t const& rhs)
      {
        ureal_t tmp = number_cast<ureal_t>((*this) - rhs);
        value_ = tmp.count();
        return *this;
      }
      /**
       * @Effects As if <c>number_cast<ureal_t>(*this*rhs)</c>
       * @Returns <c>*this</c>.
       * @Throws Any exception the Overflow policy can throw.
       */
      ureal_t& operator*=(ureal_t const& rhs)
      {
        ureal_t tmp = number_cast<ureal_t>((*this) * rhs);
        value_ = tmp.count();
        return *this;
      }

      /**
       * @Effects As if <c>divide<ureal_t>(*this,rhs)</c>
       * @Returns <c>*this</c>.
       * @Throws Any exception the Overflow policy can throw.
       */
      ureal_t& operator/=(ureal_t const& rhs)
      {
        ureal_t tmp = divide<ureal_t>(*this, rhs);
        value_ += tmp.count();
        return *this;
      }

      // Scaling

      /**
       * Virtual scaling.
       *
       * @Returns a new instance with the same data representation and with the range and resolution increased by @c N.
       */
      template <std::size_t N>
      ureal_t<Range+N, Resolution+N, Rounding, Overflow, Family>
      virtual_scale() const
      {
        return ureal_t<Range+N, Resolution+N, Rounding, Overflow, Family>(index(count()));
      }

      /**
       * Scales up N bits.
       *
       * @Effects Scales up this instance as if <c>(*this)*(2^N)</c>
       */
      template <std::size_t N>
      void scale_up()
      {
        value_ <<= N;
      }

      /**
       * Scales up/down depending on the sign of @c N.
       *
       * @Effects Scales up this instance as if <c>(*this)*(2^N)</c>
       */
      template <int N, typename RP>
      void scale()
      {
        if (N >= 0)
        {
          //value_ <<= N;
          value_ = detail::shift_left<N>(value_);
        }
        else
        {
          ureal_t tmp=
          divide<ureal_t<Range, Resolution, RP, Overflow, Family> >(*this,
          ureal_t<-N+1, -N, Rounding, Overflow, Family>(index(1)));
          value_ = tmp.count();
        }
      }

    protected:
      //! The representation.
      underlying_type value_;
    };

    /**
     * ureal_t compile time factory.
     * @TParams
     * @Param{Times,an @c int representing the number of times}
     * @Param{Resolution,the binary resolution}
     *
     * @Returns an @c ureal_t enough large to represent <c>Times*(2^Resolution)</c>.
     */
    template <int Times, int Resolution>
    BOOST_CONSTEXPR
    inline
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    ureal_t< LOG2(ABS(Times+1)), Resolution>
#else
    ureal_t< static_log2<mpl::abs<mpl::int_<Times+1> >::type::value>::value+Resolution, Resolution>
#endif
    to_ureal_t()
    {
      return ureal_t<
      static_log2<mpl::abs<mpl::int_<Times+1> >::type::value>::value+Resolution, Resolution
      >(index(Times));
    }

    /**
     * real_t compile time factory.
     * @TParams
     * @Param{Times,an @c int representing the number of times}
     * @Param{Resolution,the binary resolution}
     *
     * @Returns a @c real_t enough large to represent <c>Times*(2^Resolution)</c>.
     */
    template <int Times, int Resolution>
    BOOST_CONSTEXPR
    inline
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    real_t< LOG2(ABS(Times+1)), Resolution>
#else
    real_t< static_log2<mpl::abs<mpl::int_<Times+1> >::type::value>::value+Resolution, Resolution>
#endif
    to_real_t()
    {
      return real_t<
      static_log2<mpl::abs<mpl::int_<Times+1> >::type::value>::value+Resolution, Resolution
      > (index(Times));
    }

    //    /*
    //     * ureal_t compile time factory from integer and fractional parts
    //     *
    //     * @Returns an @c ureal_t enough large to represent <c>Integral.Fractional</c>.
    //     *
    //     * @Example ratio_to_fp<ratio<314,100>,-32>
    //     */
    //    template <typename Ratio, int Resolution>
    //    BOOST_CONSTEXPR
    //    inline
    //#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    //    ureal_t< LOG2(ABS(Integral+1)), LOG2(ABS(Fractional+1))>
    //#else
    //    ureal_t<
    //      static_log2<mpl::abs<mpl::int_<Integral+1> >::type::value>::value,
    //      static_log2<mpl::abs<mpl::int_<Fractional+1> >::type::value>::value>
    //#endif
    //    ratio_to_fp()
    //    {
    //      BOOST_CONSTEXPR intmax_t Resolution=static_log2<mpl::abs<mpl::int_<Fractional+1> >::type::value>::value;
    //      return ureal_t<
    //            static_log2<mpl::abs<mpl::int_<Integral+1> >::type::value>::value,
    //            Resolution
    //            >(index(Times));
    //    }


    // real_t non-member arithmetic

    // mixed fixed point arithmetic


    /**
     * Add type metafunction.
     *
     * The result type depends on whether the types are open/closed.
     *
     * - Both are closed: The nested typedef type is only defined if @c is_same<T1,T2> and is @c T1.
     * - One of them is Open:
     *     - if one of them is signed : real_t<MAX(R1,R2)+1, MIN(P1,P2), DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>.
     *     - if both are unsigned : ureal_t<MAX(R1,R2)+1, MIN(P1,P2), DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>.
     */
    template <typename T1, typename T2=T1, bool B1=is_open<T1>::value, bool B2=is_open<T2>::value >
    struct add_result
    {
    };
#if ! defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    /**
     * When of of them is close, both must be the same and the result is itself.
     */
    template <typename T>
    struct add_result<T, T, false, false >
    {
      typedef T type;
    };

    /**
     * When one of them is open, the result is open
     * real_t<MAX(R1,R2)+1, MIN(P1,P2), DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2, bool B1, bool B2>
    struct add_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2>, B1, B2 >
    {
      typedef real_t<
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<F1,F2>::type
      >
      type;
    };
    /**
     * When one of them is open, the result is open
     * real_t<MAX(R1,R2)+1, MIN(P1,P2), DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2, bool B1, bool B2>
    struct add_result<ureal_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2>, B1, B2 >
    : add_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >
    {
    };
    /**
     * When one of them is open, the result is open
     * real_t<MAX(R1,R2)+1, MIN(P1,P2), DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2, bool B1, bool B2>
    struct add_result<real_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2>, B1, B2 >
    : add_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >
    {
    };
    /**
     * When one of them is open, the result is open
     * ureal_t<MAX(R1,R2)+1, MIN(P1,P2), DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2, bool B1, bool B2>
    struct add_result<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2>, B1, B2 >
    {
      typedef ureal_t<
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<F1,F2>::type
      >
      type;
    };
#endif

    /**
     * @Params
     * @Param{lhs,a @c ureal_t}
     * @Param{rhs,an @c int}
     *
     * @Returns <c>lhs+AT(rhs)</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1>
    inline BOOST_CONSTEXPR
    typename add_result<real_t<R1,P1,RP1,OP1,F1> >::type
    operator+(real_t<R1,P1,RP1,OP1,F1> const& lhs, int rhs)
    {
      typedef real_t<R1,P1,RP1,OP1,F1> arg_type;

      return lhs + arg_type(rhs);
    }
    /**
     * @Params
     * @Param{lhs,a @c ureal_t}
     * @Param{rhs,an <c>unsigned int</c>}
     *
     * @Returns <c>lhs+AT(rhs)</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1>
    inline BOOST_CONSTEXPR
    typename add_result<ureal_t<R1,P1,RP1,OP1,F1> >::type
    operator+(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, unsigned int rhs)
    {
      typedef ureal_t<R1,P1,RP1,OP1,F1> arg_type;

      return lhs + arg_type(rhs);
    }

#if 0
    /**
     * signed + signed -> signed.
     * @Returns <c>RT(index(RT(lhs).count()+RT(rhs).count())</c>.
     */
    template <typename T, int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline
    T
    add(real_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef T result_type;

      return result_type(index(result_type(lhs).count()+result_type(rhs).count()));
    }
#endif

    /**
     * @Params
     * @Param{lhs,a @c real_t}
     * @Param{rhs,a @c real_t}
     *
     * @Returns <c>RT(index(RT(lhs).count() + RT(rhs).count())</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    typename add_result<real_t<R1,P1,RP1,OP1,F1> , real_t<R2,P2,RP2,OP2,F2> >::type
    operator+(real_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename add_result<real_t<R1,P1,RP1,OP1,F1> , real_t<R2,P2,RP2,OP2,F2> >::type result_type;

      return result_type(index(result_type(lhs).count()+result_type(rhs).count()));
    }

    /**
     * @Params
     * @Param{lhs,a @c ureal_t}
     * @Param{rhs,a @c real_t}
     *
     * @Returns <c>RT(index(RT(lhs).count() + RT(rhs).count())</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    typename add_result<ureal_t<R1,P1,RP1,OP1,F1> , real_t<R2,P2,RP2,OP2,F2> >::type
    operator+(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename add_result<ureal_t<R1,P1,RP1,OP1,F1> , real_t<R2,P2,RP2,OP2,F2> >::type result_type;

      return result_type(index(result_type(lhs).count()+result_type(rhs).count()));
    }
    /**
     * @Params
     * @Param{lhs,a @c real_t}
     * @Param{rhs,a @c ureal_t}
     *
     * @Returns <c>rhs + lhs</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    typename add_result<real_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type
    operator+(real_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      return rhs+lhs;
    }
    /**
     * @Params
     * @Param{lhs,a @c ureal_t}
     * @Param{rhs,a @c ureal_t}
     *
     * @Returns <c>RT(index(RT(lhs).count() + RT(rhs).count())</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    typename add_result<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type
    operator+(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename add_result<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type result_type;

      return result_type(index(result_type(lhs).count()+result_type(rhs).count()));
    }

    /**
     * @Params
     * @Param{lhs,a @c real_t}
     * @Param{rhs,a @c real_t}
     *
     * @Returns <c>RT(index(RT(lhs).count() - RT(rhs).count())</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    typename add_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type
    operator-(real_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename add_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type result_type;

      return result_type(index(result_type(lhs).count()-result_type(rhs).count()));
    }

    /**
     * @Params
     * @Param{lhs,a @c ureal_t}
     * @Param{rhs,a @c real_t}
     *
     * @Returns <c>RT(index(RT(lhs).count() - RT(rhs).count())</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    typename add_result<ureal_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type
    operator-(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename add_result<ureal_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type result_type;

      return result_type(index(result_type(lhs).count()-result_type(rhs).count()));
    }

    /**
     * @Params
     * @Param{lhs,a @c real_t}
     * @Param{rhs,a @c ureal_t}
     *
     * @Returns <c>RT(index(RT(lhs).count() - RT(rhs).count())</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    typename add_result<real_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type
    operator-(real_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename add_result<real_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type result_type;

      return result_type(index(result_type(lhs).count()-result_type(rhs).count()));
    }

    /**
     * @Params
     * @Param{lhs,a @c ureal_t}
     * @Param{rhs,a @c ureal_t}
     *
     * @Returns <c>RT(index(RT(lhs).count() - RT(rhs).count())</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    typename add_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type
    operator-(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename add_result<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type result_type;

      return result_type(index(result_type(lhs).count()-result_type(rhs).count()));
    }

    /**
     * Multiply type metafunction.
     *
     * The result type depends on whether the types are open/closed:
     * - Both are closed: The nested typedef type is only defined if @c is_same<T1,T2> and is @c T1.
     * - One of them is Open:
     *   - if one of them is signed : real_t<R1+R2, P1+P2, DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     *   - if both are unsigned : ureal_t<R1+R2, P1+P2, DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     */
    template <typename T1, typename T2=T1, bool B1=is_open<T1>::value, bool B2=is_open<T2>::value >
    struct multiply_result
    {
    };
#if ! defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    /**
     * When of of them is close, both must be the same and the result is itself.
     */
    template <typename T>
    struct multiply_result<T, T, false, false >
    {
      typedef T type;
    };

    /**
     * When one of them is open, the result is open
     * real_t<R1+R2, P1+P2, DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2, bool B1, bool B2>
    struct multiply_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2>, B1, B2 >
    {
      typedef real_t< R1+R2, P1+P2,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<F1,F2>::type
      >
      type;
    };
    /**
     * When one of them is open, the result is open
     * real_t<R1+R2, P1+P2, DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2, bool B1, bool B2>
    struct multiply_result<real_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2>, B1, B2 >
    : multiply_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >
    {
    };
    /**
     * When one of them is open, the result is open
     * real_t<R1+R2, P1+P2, DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2, bool B1, bool B2>
    struct multiply_result<ureal_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2>, B1, B2 >
    : multiply_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >
    {
    };
    /**
     * When one of them is open, the result is open
     * real_t<R1+R2, P1+P2, DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2, bool B1, bool B2>
    struct multiply_result<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2>, B1, B2 >
    {
      typedef ureal_t< R1+R2, P1+P2,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<F1,F2>::type
      >
      type;
    };
#endif

    /**
     * @Params
     * @Param{lhs,a @c real_t}
     * @Param{rhs,a @c real_t}
     *
     * @Returns <c>RT(index(RT(lhs).count() * RT(rhs).count())</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    typename multiply_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type
    operator*(real_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename multiply_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type result_type;
      typedef typename result_type::underlying_type underlying_type;

      return result_type(index(underlying_type(lhs.count()) * rhs.count()));
    }

    /**
     * @Params
     * @Param{lhs,a @c real_t}
     * @Param{rhs,a @c ureal_t}
     *
     * @Returns <c>RT(index(RT(lhs).count() * RT(rhs).count())</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    typename multiply_result<real_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type
    operator*(real_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename multiply_result<real_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type result_type;
      typedef typename result_type::underlying_type underlying_type;

      return result_type(index(underlying_type(lhs.count()) * rhs.count()));
    }

    /**
     * @Params
     * @Param{lhs,a @c ureal_t}
     * @Param{rhs,a @c real_t}
     *
     * @Returns <c>RT(index(RT(lhs).count() * RT(rhs).count())</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    typename multiply_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type
    operator*(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename multiply_result<ureal_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type result_type;
      typedef typename result_type::underlying_type underlying_type;

      return result_type(index(underlying_type(lhs.count()) * rhs.count()));
    }

    /**
     * @Params
     * @Param{lhs,a @c ureal_t}
     * @Param{rhs,a @c ureal_t}
     *
     * @Returns <c>RT(index(RT(lhs).count() * RT(rhs).count())</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    typename multiply_result<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type
    operator*(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename multiply_result<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type result_type;
      typedef typename result_type::underlying_type underlying_type;

      return result_type(index(underlying_type(lhs.count()) * rhs.count()));
    }

    /**
     * Fixed point division giving the expected result type.
     * @Returns <c>DT(lhs) / DT(rhs)</c> taking in account the rounding policy of the result type @c Res.
     */
    template <
    typename Res,
    int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline
    Res
    divide(real_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef Res result_type;
      typedef typename result_type::underlying_type underlying_type;

      typedef typename common_type<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type DT;
      //BOOST_STATIC_CONSTEXPR int P = Res::resolution_exp;

      //BOOST_STATIC_ASSERT((Res::digits>=(DT::digits-P)));
      BOOST_STATIC_ASSERT((Res::is_signed==DT::is_signed));
      BOOST_ASSERT_MSG(DT(rhs).count()!=0, "Division by 0");

      typedef typename result_type::rounding_type rounding_type;
      return result_type(index(rounding_type::template round_divide<Res>(DT(lhs), DT(rhs))));
    }

    /**
     * Fixed point division giving the expected result type.
     * @Returns <c>DT(lhs) / DT(rhs)</c> taking in account the rounding policy of the result type @c Res.
     */
    template <
    typename Res,
    int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline
    Res
    divide(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef Res result_type;
      typedef typename result_type::underlying_type underlying_type;
      typedef typename common_type<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type DT;
      //BOOST_STATIC_CONSTEXPR int P = Res::resolution_exp;

      //BOOST_STATIC_ASSERT((Res::digits>=(DT::digits-P)));
      BOOST_STATIC_ASSERT((Res::is_signed==DT::is_signed));
      BOOST_ASSERT_MSG(DT(rhs).count()!=0, "Division by 0");

      typedef typename result_type::rounding_type rounding_type;
      return result_type(index(rounding_type::template round_divide<Res>(DT(lhs), DT(rhs))));
    }

    /**
     * fixed point division giving the expected result type.
     * @Returns <c>DT(lhs) / DT(rhs)</c> taking in account the rounding policy of the result type @c Res.
     */
    template <
    typename Res,
    int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline
    Res
    divide(real_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef Res result_type;
      typedef typename result_type::underlying_type underlying_type;
      typedef typename common_type<real_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type DT;
      //BOOST_STATIC_CONSTEXPR int P = Res::resolution_exp;

      //BOOST_STATIC_ASSERT((Res::digits>=(DT::digits-P)));
      BOOST_STATIC_ASSERT((Res::is_signed==DT::is_signed));
      BOOST_ASSERT_MSG(DT(rhs).count()!=0, "Division by 0");

      typedef typename result_type::rounding_type rounding_type;
      return result_type(index(rounding_type::template round_divide<Res>(DT(lhs), DT(rhs))));
    }

    /**
     * fixed point division giving the expected result type.
     * @Returns <c>DT(lhs) / DT(rhs)</c> taking in account the rounding policy of the result type @c Res.
     */
    template <
    typename Res,
    int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline
    Res
    divide(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef Res result_type;
      typedef typename result_type::underlying_type underlying_type;
      typedef typename common_type<ureal_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type DT;
      //BOOST_STATIC_CONSTEXPR int P = Res::resolution_exp;

      //BOOST_STATIC_ASSERT((Res::digits>=(DT::digits-P)));
      BOOST_STATIC_ASSERT((Res::is_signed==DT::is_signed));
      BOOST_ASSERT_MSG(DT(rhs).count()!=0, "Division by 0");

      typedef typename result_type::rounding_type rounding_type;
      return result_type(index(rounding_type::template round_divide<Res>(DT(lhs), DT(rhs))));
    }

    /**
     * Divide type metafunction.
     *
     * The result type depends on whether the types are open/closed.
     * - Both are closed: The nested typedef type is only defined if @c is_same<T1,T2> and is @c T1.
     * - One of them is open:
     *   - if one of them is signed : <c>real_t<R1-P2, P1-R2, DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)></c>
     *   - if both are unsigned : <c>ureal_t<R1-P2, P1-R2, DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)></c>
     */
    template <typename T1, typename T2=T1, bool B1=is_open<T1>::value, bool B2=is_open<T2>::value >
    struct divide_result
    {
    };
#if ! defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    /**
     * When of of them is close, both must be the same and the result is itself.
     */
    template <typename T>
    struct divide_result<T, T, false, false >
    {
      typedef T type;
    };

    /**
     * When one of them is open, the result is open
     * real_t<R1-P2, P1-R2, DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2, bool B1, bool B2>
    struct divide_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2>, B1, B2 >
    {
      typedef real_t< R1-P2, P1-R2,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<F1,F2>::type
      >
      type;
    };
    /**
     * When one of them is open, the result is open
     * real_t<R1-P2, P1-R2, DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2, bool B1, bool B2>
    struct divide_result<ureal_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2>, B1, B2 >
    : divide_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >
    {
    };
    /**
     * When one of them is open, the result is open
     * real_t<R1-P2, P1-R2, DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2, bool B1, bool B2>
    struct divide_result<real_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2>, B1, B2 >
    : divide_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >
    {
    };
    /**
     * When one of them is open, the result is open
     * real_t<R1-P2, P1-R2, DT(RP1,RP2), DT(OP1,OP2), DT(F1,F2)>
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2, bool B1, bool B2>
    struct divide_result<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2>, B1, B2 >
    {
      typedef ureal_t< R1-P2, P1-R2,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<F1,F2>::type
      >
      type;
    };
#endif

    /**
     * fixed point division  deducing the result type as <R1-P2, P1,R2>.
     * @Returns <c>divide<RT>(lhs,rhs)</c> taking in account the rounding policy of the result type.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline
    typename divide_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type
    operator/(real_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename divide_result<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type result_type;

      return divide<result_type>(lhs,rhs);
    }

    /**
     * fixed point division  deducing the result type as <R1-P2, P1,R2>.
     * @Returns <c>divide<RT>(lhs,rhs)</c> taking in account the rounding policy of the result type.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    typename divide_result<real_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type
    operator/(real_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename divide_result<real_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type result_type;

      return divide<result_type>(lhs,rhs);
    }

    /**
     * fixed point division deducing the result type as <R1-P2, P1,R2>.
     * @Returns <c>divide<RT>(lhs,rhs)</c> taking in account the rounding policy of the result type.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    typename divide_result<ureal_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type
    operator/(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename divide_result<ureal_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type result_type;

      return divide<result_type>(lhs,rhs);
    }

    /**
     * fixed point division deducing the result type as <R1-P2, P1,R2>.
     * @Returns <c>divide<RT>(lhs,rhs)</c> taking in account the rounding policy of the result type.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline
    typename divide_result<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type
    operator/(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename divide_result<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type result_type;

      return divide<result_type>(lhs,rhs);
    }

    // comparisons

    /**
     * @Returns As if <c>DT(lhs).count() == DT(rhs).count()</c> where DT is the common_type of the parameters.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    bool
    operator==(real_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename common_type<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type DT;
      return DT(lhs).count() == DT(rhs).count();
    }

    /**
     * @Returns As if <c>DT(lhs).count() == DT(rhs).count()</c> where DT is the common_type of the parameters..
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    bool
    operator==(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename common_type<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type DT;
      return DT(lhs).count() == DT(rhs).count();
    }

    /**
     * @Returns <c>DT(lhs).count() != DT(rhs).count()</c> where DT is the common_type of the parameters..
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    bool
    operator!=(real_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      return !(lhs == rhs);
    }

    /**
     * @Returns <c>DT(lhs).count() != DT(rhs).count()</c> where DT is the common_type of the parameters..
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    bool
    operator!=(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      return !(lhs == rhs);
    }

    /**
     * @Returns <c>DT(lhs).count() < DT(rhs).count()</c> where DT is the common_type of the parameters..
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    bool
    operator<(real_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename common_type<real_t<R1,P1,RP1,OP1,F1>, real_t<R2,P2,RP2,OP2,F2> >::type DT;
      return DT(lhs).count() < DT(rhs).count();
    }

    /**
     * @Returns <c>DT(lhs).count() < DT(rhs).count()</c> where DT is the common_type of the parameters..
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    bool
    operator<(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      typedef typename common_type<ureal_t<R1,P1,RP1,OP1,F1>, ureal_t<R2,P2,RP2,OP2,F2> >::type DT;
      return DT(lhs).count() < DT(rhs).count();
    }

    /**
     * @Returns <c>rhs < lhs</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    bool
    operator>(real_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      return rhs < lhs;
    }
    /**
     * @Returns <c>rhs < lhs</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    bool
    operator>(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      return rhs < lhs;
    }

    /**
     * @Returns <c>!(rhs < lhs)</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    bool
    operator<=(real_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      return !(rhs < lhs);
    }
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    /**
     * @Returns <c>!(rhs < lhs)</c>.
     */
    inline BOOST_CONSTEXPR
    bool
    operator<=(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      return !(rhs < lhs);
    }

    /**
     * @Returns <c>!(lhs < rhs)</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    inline BOOST_CONSTEXPR
    bool
    operator>=(real_t<R1,P1,RP1,OP1,F1> const& lhs, real_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      return !(lhs < rhs);
    }
    template <int R1, int P1, typename RP1, typename OP1, typename F1,
    int R2, int P2, typename RP2, typename OP2, typename F2>
    /**
     * @Returns <c>!(lhs < rhs)</c>.
     */
    inline BOOST_CONSTEXPR
    bool
    operator>=(ureal_t<R1,P1,RP1,OP1,F1> const& lhs, ureal_t<R2,P2,RP2,OP2,F2> const& rhs)
    {
      return !(lhs < rhs);
    }

    template <typename To, typename From>
    BOOST_CONSTEXPR
    To number_cast(From const& f)
    {
      return fixed_point::detail::number_cast<From, To>()(f);
    }
  }
}

//! specializations
namespace std
{
  //! numeric limits trait specializations
  template <int R, int P, typename RP, typename OP, typename F>
  struct numeric_limits<boost::fixed_point::real_t<R, P, RP, OP, F> >
  {
    typedef boost::fixed_point::real_t<R, P, RP, OP, F> rep;
  public:
    BOOST_STATIC_CONSTEXPR
    bool is_specialized = true;
    inline static rep min()
    {
      return rep::min();
    }
    inline static rep max()
    {
      return rep::max();
    }
    inline static rep lowest()
    {
      return rep::lowest();
    }
    BOOST_STATIC_CONSTEXPR
    int digits = rep::digits;
    //BOOST_STATIC_CONSTEXPR int digits10 = rep::digits10;
    //BOOST_STATIC_CONSTEXPR int max_digits10 = rep::max_digits10;
    BOOST_STATIC_CONSTEXPR
    bool is_signed = true;BOOST_STATIC_CONSTEXPR
    bool is_integer = false;BOOST_STATIC_CONSTEXPR
    bool is_exact = true;BOOST_STATIC_CONSTEXPR
    int radix = 2;
    //inline static rep epsilon() { return rep::epsilon(); }
    //inline static rep round_error() { return rep::round_error(); }
    //BOOST_STATIC_CONSTEXPR int min_exponent = rep::min_exponent;
    //BOOST_STATIC_CONSTEXPR int min_exponent10  = rep::min_exponent10;
    //BOOST_STATIC_CONSTEXPR int max_exponent = rep::max_exponent;
    //BOOST_STATIC_CONSTEXPR int max_exponent10 = rep::max_exponent10;
    BOOST_STATIC_CONSTEXPR
    bool has_infinity = false;BOOST_STATIC_CONSTEXPR
    bool has_quiet_NaN = false;BOOST_STATIC_CONSTEXPR
    bool has_signaling_NaN = false;
    //BOOST_STATIC_CONSTEXPR float_denorm_style has_denorm = denorm_absent;
    //BOOST_STATIC_CONSTEXPR bool has_denorm_loss = false;
    //inline static rep infinity() { return rep::infinity(); }
    //inline static rep quiet_NaN() { return rep::quiet_NaN(); }
    //inline static rep signaling_NaN() { return rep::signaling_NaN(); }
    //inline static rep denorm_min() { return rep::denorm_min<Q>(); }
    //BOOST_STATIC_CONSTEXPR bool is_iec559 = false;
    BOOST_STATIC_CONSTEXPR
    bool is_bounded = true;BOOST_STATIC_CONSTEXPR
    bool is_modulo = RP::is_modulo;BOOST_STATIC_CONSTEXPR
    bool traps = true;
    //BOOST_STATIC_CONSTEXPR bool tinyness_before = rep::tinyness_before;
    BOOST_STATIC_CONSTEXPR
    float_round_style round_style = RP::round_style;
  };
}

#endif // header
