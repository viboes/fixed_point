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
#include <boost/utility/enable_if.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/integer.hpp>
#include <boost/integer/static_log2.hpp>
#include <boost/ratio/detail/mpl/abs.hpp>
#include <limits>
#include <boost/integer_traits.hpp>

#include <boost/config.hpp>
//#include <boost/fixed_point/config.hpp>
//#include <boost/fixed_point/round/nearest_odd.hpp>
//#include <boost/fixed_point/overflow/exception.hpp>

#include <limits>

namespace boost
{
  namespace fixed_point
  {
    namespace detail {

      template <typename From, typename To, bool IsPositive=(To::resolution_exp>0)>
      struct shift_impl;
      template <typename From, typename To>
      struct shift_impl<From, To, true> {
        BOOST_STATIC_ASSERT(From::digits>To::resolution_exp);
        BOOST_STATIC_CONSTEXPR std::size_t digits = From::digits-To::resolution_exp;
        typedef typename From::underlying_type result_type;
        static  result_type apply(typename From::underlying_type v)
        {
          return v >> To::resolution_exp;
        }
      };
      template <typename From, typename To>
      struct shift_impl<From,To,false> {
        BOOST_STATIC_CONSTEXPR std::size_t digits = From::digits-To::resolution_exp;
        typedef typename ::boost::int_t<digits>::fast result_type;
        static result_type apply(typename From::underlying_type v)
        {
          return result_type(v) << -To::resolution_exp;
        }
      };

      template <typename From, typename To>
      typename shift_impl<From,To>::result_type shift(typename From::underlying_type v) {
        return shift_impl<From,To>::apply(v);
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

    /**
     * Exception throw when there is a positive overflow.
     */
    struct positive_overflow {};
    /**
     * Exception throw when there is a negative overflow.
     */
    struct negative_overflow {};

    /**
     * Namespace for rounding policies.
     */
    namespace round
    {
      struct fastest {
        BOOST_STATIC_CONSTEXPR std::float_round_style  round_style = std::round_indeterminate;
      };
      struct negative {
        BOOST_STATIC_CONSTEXPR std::float_round_style  round_style = std::round_toward_infinity;
        template <typename From, typename To>
        static typename To::underlying_type round(From const& rhs)
        {
          BOOST_STATIC_CONSTEXPR boost::uintmax_t d = To::resolution_exp-From::resolution_exp;
          typedef typename detail::max_type<is_signed<typename To::underlying_type>::value>::type tmp_type;
          BOOST_STATIC_ASSERT(d<(8*sizeof(tmp_type)));
          //BOOST_MPL_ASSERT_MSG(d<(8*sizeof(tmp_type)), OVERFLOW, (mpl::int_<8*sizeof(tmp_type)>, mpl::int_<d>));

          tmp_type res = tmp_type(rhs.count()) >> d;
          BOOST_ASSERT(res<=To::max_index);
          BOOST_ASSERT(res>=To::min_index);
          return res;
        }
        template <typename To, typename From>
        static typename To::underlying_type round_divide(From const& lhs, From const& rhs)
        {
          typedef typename detail::shift_impl<From, To>::result_type result_type;
          result_type ci = detail::shift<From, To>(lhs.count()) / rhs.count();
          if (ci>=0 )
          {
            BOOST_ASSERT(ci<=To::max_index);
            BOOST_ASSERT(ci>=To::min_index);
            return ci;
          } else {
            result_type ri = detail::shift<From, To>(lhs.count()) % rhs.count();
            if (ri==0) {
              BOOST_ASSERT(ci<=To::max_index);
              BOOST_ASSERT(ci>=To::min_index);
              return ci;
            }
            else
            {
              BOOST_ASSERT(ci-1<=To::max_index);
              BOOST_ASSERT(ci>=(To::min_index+1));
              return ci-1;
            }
          }
        }
      };
      struct truncated {
        BOOST_STATIC_CONSTEXPR std::float_round_style  round_style = std::round_toward_zero;
        template <typename From, typename To>
        static typename To::underlying_type round(From const& rhs)
        {
          BOOST_STATIC_CONSTEXPR boost::uintmax_t d = To::resolution_exp-From::resolution_exp;
          typedef typename detail::max_type<is_signed<typename To::underlying_type>::value>::type tmp_type;
          BOOST_STATIC_ASSERT(d<(8*sizeof(tmp_type)));

          tmp_type m(((rhs.count()>0)?rhs.count():-rhs.count()));
          tmp_type s(((rhs.count()>0)?+1:-1));

          tmp_type res = s * (m >> d);
          BOOST_ASSERT(res<=To::max_index);
          BOOST_ASSERT(res>=To::min_index);
          return res;
          ;
        }
        template <typename To, typename From>
        static typename To::underlying_type round_divide(From const& lhs, From const& rhs)
        {
          typedef typename detail::shift_impl<From, To>::result_type result_type;
          result_type ci = detail::shift<From, To>(lhs.count()) / rhs.count();
          BOOST_ASSERT(ci<=To::max_index);
          BOOST_ASSERT(ci>=To::min_index);
          return ci;
        }
      };
      struct positive {
        BOOST_STATIC_CONSTEXPR std::float_round_style  round_style = std::round_toward_neg_infinity;
        template <typename From, typename To>
        static typename To::underlying_type round(From const& rhs)
        {
          BOOST_STATIC_CONSTEXPR boost::uintmax_t d = To::resolution_exp-From::resolution_exp;
          typedef typename detail::max_type<is_signed<typename To::underlying_type>::value>::type tmp_type;
          BOOST_STATIC_ASSERT(d<(8*sizeof(tmp_type)));

          BOOST_STATIC_CONSTEXPR tmp_type w = (1<<d)-1;
          tmp_type i = rhs.count();

          BOOST_ASSERT(i<=(integer_traits<tmp_type>::const_max-w));

          tmp_type res =  (i+w) >> d;
          BOOST_ASSERT(res<=To::max_index);
          BOOST_ASSERT(res>=To::min_index);
          return res;
        }
        template <typename To, typename From>
        static typename To::underlying_type round_divide(From const& lhs, From const& rhs)
        {
          typedef typename detail::shift_impl<From, To>::result_type result_type;
          result_type ci = detail::shift<From, To>(lhs.count()) / rhs.count();
          if (ci>=0 )
          {
            result_type ri = detail::shift<From, To>(lhs.count()) % rhs.count();
            if (ri==0) {
              BOOST_ASSERT(ci<=To::max_index);
              BOOST_ASSERT(ci>=To::min_index);
              return ci;
            } else {
              BOOST_ASSERT(ci<=To::max_index-1);
              BOOST_ASSERT(ci+1>=To::min_index);
              return ci+1;
            }
          } else {
            BOOST_ASSERT(ci<=To::max_index);
            BOOST_ASSERT(ci>=To::min_index);
            return ci;
          }
        }
      };
      struct nearest_half_up {
        BOOST_STATIC_CONSTEXPR std::float_round_style  round_style =  std::round_to_nearest;
      };
      struct nearest_half_down {
        BOOST_STATIC_CONSTEXPR std::float_round_style  round_style =  std::round_to_nearest;
      };
      struct nearest_even {
        BOOST_STATIC_CONSTEXPR std::float_round_style  round_style =  std::round_to_nearest;
      };
      struct nearest_odd {
        BOOST_STATIC_CONSTEXPR std::float_round_style  round_style =  std::round_to_nearest;
      };
    }

    /**
     * Namespace for overflow policies.
     */
    namespace overflow
    {
      struct impossible {
        BOOST_STATIC_CONSTEXPR bool is_modulo = false;
        template <typename T, typename U>
        static typename T::underlying_type on_negative_overflow(U value)
        {
          BOOST_ASSERT_MSG(false,"Negative overflow while trying to convert fixed point numbers");
          return value;
        }
        template <typename T, typename U>
        static typename T::underlying_type on_positive_overflow(U value)
        {
          BOOST_ASSERT_MSG(false,"Positive overflow while trying to convert fixed point numbers");
          return value;
        }
      };
      struct undefined {
        BOOST_STATIC_CONSTEXPR bool is_modulo = false;
        template <typename T, typename U>
        static typename T::underlying_type on_negative_overflow(U value)
        {
          return value;
        }
        template <typename T, typename U>
        static typename T::underlying_type on_positive_overflow(U value)
        {
          return value;
        }
      };
      namespace detail {
        template <typename T, typename U, bool TisSigned=T::is_signed>
        struct modulus_on_negative_overflow;


        template <typename T, typename U>
        struct modulus_on_negative_overflow<T,U, false>
        {
          static typename T::underlying_type value(U value)
          {
            return (value%(T::max_index-T::min_index+1))+(T::max_index-T::min_index+1);
          }
        };

        template <typename T, typename U>
        struct modulus_on_negative_overflow<T,U, true>
        {
          static typename T::underlying_type value(U value)
          {
            return ((value-T::min_index)%(T::max_index-T::min_index+1))-T::min_index;
          }
        };


        template <typename T, typename U, bool TisSigned=T::is_signed>
        struct modulus_on_positive_overflow;

        template <typename T, typename U>
        struct modulus_on_positive_overflow<T,U, true>
        {
          static typename T::underlying_type value(U value)
          {
            return ((value-T::max_index)%(T::max_index-T::min_index+1))-T::max_index;
          }
        };
        template <typename T, typename U>
        struct modulus_on_positive_overflow<T,U, false>
        {
          static typename T::underlying_type value(U value)
          {
            return value%(T::max_index-T::min_index+1);
          }
        };
      }
      struct modulus {
        BOOST_STATIC_CONSTEXPR bool is_modulo = true;
        template <typename T, typename U>
        static typename T::underlying_type on_negative_overflow(U val)
        {
          return detail::modulus_on_negative_overflow<T,U>::value(val);
        }
        template <typename T, typename U>
        static typename T::underlying_type modulus_on_positive_overflow(U val)
        {
          return detail::modulus_on_negative_overflow<T,U>::value(val);
        }
      };
      struct saturate {
        BOOST_STATIC_CONSTEXPR bool is_modulo = false;
        template <typename T, typename U>
        static typename T::underlying_type on_negative_overflow(U value)
        {
          return T::min_index;
        }
        template <typename T, typename U>
        static typename T::underlying_type on_positive_overflow(U value)
        {
          return T::max_index;
        }

      };
      struct exception {
        BOOST_STATIC_CONSTEXPR bool is_modulo = false;
        template <typename T, typename U>
        static typename T::underlying_type on_negative_overflow(U value)
        {
          throw negative_overflow();
        }
        template <typename T, typename U>
        static typename T::underlying_type on_positive_overflow(U value)
        {
          throw positive_overflow();
        }

      };
    }

    /**
     * Namespace for optimization policies.
     */
    namespace optimization
    {

      struct undefined {
        /**
         * signed_integer_type: Gets the signed integer type with enough bits to manage with the Range and Resolution depending on the Opt
         */
        template <int Range, int Resolution>
        struct signed_integer_type
        {
          typedef typename ::boost::int_t<Range-Resolution+1>::least type;
        };

        /**
         * unsigned_integer_type: Gets the unsigned integer type with enough bits to manage with the Range and Resolution depending on the Opt
         */
        template <int Range, int Resolution>
        struct unsigned_integer_type
        {
          typedef typename ::boost::uint_t<Range-Resolution>::least type;
        };
      };
      struct space {
        template <int Range, int Resolution>
        struct signed_integer_type
        {
          typedef typename ::boost::int_t<Range-Resolution+1>::least type;
        };
        template <int Range, int Resolution>
        struct unsigned_integer_type
        {
          typedef typename ::boost::uint_t<Range-Resolution>::least type;
        };
      };
      struct speed {
        template <int Range, int Resolution>
        struct signed_integer_type
        {
          typedef typename ::boost::int_t<Range-Resolution+1>::fast type;
        };
        template <int Range, int Resolution>
        struct unsigned_integer_type
        {
          typedef typename ::boost::uint_t<Range-Resolution>::fast type;
        };

      };
    }

    template <
      int Range,
      int Resolution,
      typename Rounding=round::negative,
      typename Overflow=overflow::exception,
      typename Optimization=optimization::space>
    class unsigned_number;

    template <
      int Range,
      int Resolution,
      typename Rounding=round::negative,
      typename Overflow=overflow::exception,
      typename Optimization=optimization::space>
    class signed_number;

    template <
              typename Res,
              int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    Res
    divide(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs);
    template <
              typename Res,
              int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    Res
    divide(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs);

    template <
              typename Res,
              int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    Res
    divide(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs);

    template <
              typename Res,
              int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    Res
    divide(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs);

  }
}

#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
#define BOOST_TYPEOF_SILENT
#include <boost/typeof/typeof.hpp>

#include BOOST_TYPEOF_INCREMENT_REGISTRATION_GROUP()

BOOST_TYPEOF_REGISTER_TEMPLATE(boost::fixed_point::signed_number, (int)(int)(typename)(typename)(typename))
BOOST_TYPEOF_REGISTER_TEMPLATE(boost::fixed_point::unsigned_number, (int)(int)(typename)(typename)(typename))

#endif

///////////////////////////////////////

namespace boost {
  namespace fixed_point {


    /**
     *  named parameter like class, allowing to make a specific overload when the integer must be taken by the index.
     */
    template <typename T>
    struct index_tag
    {
      typedef T type;
      T value;
      BOOST_CONSTEXPR index_tag(T v) : value(v) {}
      BOOST_CONSTEXPR T get() { return value; }

    };

    /**
     *  helper function to make easier the use of index_tag.
     */
    template <typename T>
    struct index_tag<T> index(T v) { return index_tag<T>(v); }

    /**
     *  explicit conversion between fixed_point numbers.
     */
    template <class From, class To>
    To number_cast(From const&);

    namespace detail
    {
      template <typename T, int Range, int Resolution >
      struct signed_integer_traits {
        BOOST_STATIC_CONSTEXPR std::size_t digits = (Range-Resolution)+1;
        BOOST_STATIC_ASSERT_MSG((sizeof(T)*8)>=digits, "LLLL");
        //BOOST_MPL_ASSERT_MSG((sizeof(T)*8)>=digits, LLLL, (mpl::int_<sizeof(T)*8>, mpl::int_<digits>));
        BOOST_STATIC_CONSTEXPR T const_max = (1<<(digits-1)) - 1;
        BOOST_STATIC_CONSTEXPR T const_min = -const_max;

      };
      template <typename T, int Range, int Resolution >
      struct unsigned_integer_traits {
        BOOST_STATIC_CONSTEXPR std::size_t digits = (Range-Resolution);
        BOOST_STATIC_CONSTEXPR T const_max = (1<<(digits)) - 1;
        BOOST_STATIC_CONSTEXPR T const_min = 0;

      };

      template <int Range, int Resolution, typename Optimization=optimization::space>
      class signed_uniform_quantizer
      {
        BOOST_MPL_ASSERT_MSG(Range>=Resolution, RANGE_MUST_BE_GREATER_EQUAL_THAN_RESOLUTION, (mpl::int_<Range>,mpl::int_<Resolution>));
      public:

        //! The underlying integer type
        typedef typename Optimization::template  signed_integer_type<Range,Resolution>::type underlying_type;

        // name the template parameters
        BOOST_STATIC_CONSTEXPR int range_exp = Range;
        BOOST_STATIC_CONSTEXPR int resolution_exp = Resolution;
        BOOST_STATIC_CONSTEXPR int digits = range_exp-resolution_exp+1;

        typedef Optimization optimization_type;

        BOOST_STATIC_CONSTEXPR underlying_type min_index = detail::signed_integer_traits<underlying_type,Range,Resolution>::const_min;
        BOOST_STATIC_CONSTEXPR underlying_type max_index = detail::signed_integer_traits<underlying_type,Range,Resolution>::const_max;

        //! conversion factor.
        template <typename FP>
        static FP factor()
        {
          if (Resolution>=0) return FP(1 << Resolution);
          else return FP(1)/(1 << -Resolution);
        }
        template <typename FP>
        static underlying_type integer_part(FP x)
        {
          return underlying_type(floor(x));
        }
      };


      template <typename Final, int Range, int Resolution, typename Rounding=round::negative, typename Overflow=overflow::exception,
          typename Optimization=optimization::space
          >
      class signed_quantizer : public signed_uniform_quantizer<Range,Resolution,Optimization>
      {
        typedef signed_uniform_quantizer<Range,Resolution,Optimization> base_type;
      public:
        typedef typename base_type::underlying_type underlying_type;
        BOOST_STATIC_CONSTEXPR underlying_type min_index = base_type::const_min;
        BOOST_STATIC_CONSTEXPR underlying_type max_index = base_type::const_max;

        template <typename FP>
        static FP reconstruct(underlying_type k)
        {
          BOOST_ASSERT(min_index <= k && k <= max_index);

          return Rounding::reconstruct(k, base_type::template factor<FP>());
        }
        template <typename FP>
        static underlying_type classify(FP x)
        {
          if (x<Final::min().template as<FP>()) {
            return Overflow::on_negative_overflow(min_index,x);
          }
          if (x>Final::max().template as<FP>()) {
            return Overflow::on_positive_overflow(max_index,x);
          }
          return Rounding::classify(x, base_type::template factor<FP>());
        }
        template <typename FP>
        static Final cast(FP x)
        {
          fixed_point::number_cast<Final>(x);
        }
      };

      template <
        typename From,
        typename To,
        bool LE_Range=      From::range_exp      <= To::range_exp,
        bool GE_Resolution= From::resolution_exp >= To::resolution_exp
      > struct number_cast;

      // LE_Range=true GE_Resolution=true
      ///////////////////////////////////
      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        signed_number<R1,P1,RP1,OP1,Opt1>,
        signed_number<R2,P2,RP2,OP2,Opt2>,
        true, true >
      {
        typedef signed_number<R1,P1,RP1,OP1,Opt1> From;
        typedef signed_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // No overflow and no round needed
          return To(index(underlying_type(rhs.count()) << (P1-P2)));
        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        unsigned_number<R1,P1,RP1,OP1,Opt1>,
        signed_number<R2,P2,RP2,OP2,Opt2>,
        true, true >
      {
        typedef unsigned_number<R1,P1,RP1,OP1,Opt1> From;
        typedef signed_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // No overflow and no round needed
          return To(index(underlying_type(rhs.count()) << (P1-P2)));
        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        unsigned_number<R1,P1,RP1,OP1,Opt1>,
        unsigned_number<R2,P2,RP2,OP2,Opt2>,
        true, true >
      {
        typedef unsigned_number<R1,P1,RP1,OP1,Opt1> From;
        typedef unsigned_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // No overflow and no round needed
          return To(index(underlying_type(rhs.count()) << (P1-P2)));
        }
      };

      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        signed_number<R1,P1,RP1,OP1,Opt1>,
        unsigned_number<R2,P2,RP2,OP2,Opt2>,
        true, true >
      {
        typedef unsigned_number<R1,P1,RP1,OP1,Opt1> From;
        typedef unsigned_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
//          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
//          // Overflow
//          if (indx < To::min_index)
//          {
//            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
//          }
//          // No round needed
//          return To(index(underlying_type(rhs.count()) << (P1-P2)));

          return
          (
            (((underlying_type(rhs.count()) << (P1-P2))) < To::min_index)
          ? To(index(OP2::template on_negative_overflow<To,underlying_type>(((underlying_type(rhs.count()) << (P1-P2))))))
          : To(index(underlying_type(rhs.count()) << (P1-P2)))
          );

        }
      };

      // LE_Range=false GE_Resolution=true
      ////////////////////////////////////
      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        signed_number<R1,P1,RP1,OP1,Opt1>,
        signed_number<R2,P2,RP2,OP2,Opt2>,
        false, true >
      {
        typedef signed_number<R1,P1,RP1,OP1,Opt1> From;
        typedef signed_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {

//          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
//          // Overflow impossible
//          if (indx > To::max_index)
//          {
//            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
//          }
//          if (indx < To::min_index)
//          {
//            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
//          }
//
//          // No round needed
//          return To(index(indx));

          // Overflow impossible
          // No round needed
          return (
              (((underlying_type(rhs.count()) << (P1-P2))) > To::max_index)
            ? To(index(OP2::template on_positive_overflow<To,underlying_type>(((underlying_type(rhs.count()) << (P1-P2))))))
            : (
                (((underlying_type(rhs.count()) << (P1-P2))) < To::min_index)
              ? To(index(OP2::template on_negative_overflow<To,underlying_type>(((underlying_type(rhs.count()) << (P1-P2))))))
              : To(index(((underlying_type(rhs.count()) << (P1-P2)))))
              )
            );
        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        signed_number<R1,P1,RP1,OP1,Opt1>,
        unsigned_number<R2,P2,RP2,OP2,Opt2>,
        false, true >
      {
        typedef signed_number<R1,P1,RP1,OP1,Opt1> From;
        typedef unsigned_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {

//          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
//          // Overflow impossible
//          if (indx > To::max_index)
//          {
//            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
//          }
//          if (indx < To::min_index)
//          {
//            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
//          }
//
//          // No round needed
//          return To(index(indx));

          // Overflow impossible
          // No round needed
          return (
                (((underlying_type(rhs.count()) << (P1-P2))) > To::max_index)
              ? To(index(OP2::template on_positive_overflow<To,underlying_type>(((underlying_type(rhs.count()) << (P1-P2))))))
              : (
                  (((underlying_type(rhs.count()) << (P1-P2))) < To::min_index)
                  ? To(index(OP2::template on_negative_overflow<To,underlying_type>(((underlying_type(rhs.count()) << (P1-P2))))))
                  : To(index(((underlying_type(rhs.count()) << (P1-P2)))))
                )
              );
        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        unsigned_number<R1,P1,RP1,OP1,Opt1>,
        unsigned_number<R2,P2,RP2,OP2,Opt2>,
        false, true >
      {
        typedef unsigned_number<R1,P1,RP1,OP1,Opt1> From;
        typedef unsigned_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {

//          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
//          // Overflow
//          if (indx > To::max_index)
//          {
//            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
//          }
//
//          // No round needed
//          return To(index(indx));
//
          return
          (
            (((underlying_type(rhs.count()) << (P1-P2))) > To::max_index)
          ? To(index(OP2::template on_positive_overflow<To,underlying_type>(((underlying_type(rhs.count()) << (P1-P2))))))
          : To(index(((underlying_type(rhs.count()) << (P1-P2)))))
          );
        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        unsigned_number<R1,P1,RP1,OP1,Opt1>,
        signed_number<R2,P2,RP2,OP2,Opt2>,
        false, true >
      {
        typedef unsigned_number<R1,P1,RP1,OP1,Opt1> From;
        typedef signed_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {

//          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
//          // Overflow
//          if (indx > To::max_index)
//          {
//            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
//          }
//
//          // No round needed
//          return To(index(indx));

          return
          (
            (((underlying_type(rhs.count()) << (P1-P2))) > To::max_index)
          ? To(index(OP2::template on_positive_overflow<To,underlying_type>(((underlying_type(rhs.count()) << (P1-P2))))))
          : To(index(((underlying_type(rhs.count()) << (P1-P2)))))
          );

        }
      };

      // LE_Range=true GE_Resolution=false
      ////////////////////////////////////
      template <int R, int P1, typename RP1, typename OP1, typename Opt1,
                       int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        signed_number<R,P1,RP1,OP1,Opt1>,
        signed_number<R,P2,RP2,OP2,Opt2>,
        true, false >
      {
        typedef signed_number<R,P1,RP1,OP1,Opt1> From;
        typedef signed_number<R,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
//          // Overflow could be possible because more resolution implies a bigger range when the range exponents are the same.
//          underlying_type indx(((rhs.count()) >> (P2-P1)));
//          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
//          {
//            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
//          }
//          if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
//          {
//            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
//          }
//
//          // Round
//          To res((index(RP2::template round<From,To>(rhs))));
//          return res;

          // Overflow could be possible because more resolution implies a bigger range when the range exponents are the same.
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
      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        signed_number<R1,P1,RP1,OP1,Opt1>,
        signed_number<R2,P2,RP2,OP2,Opt2>,
        true, false >
      {
        typedef signed_number<R1,P1,RP1,OP1,Opt1> From;
        typedef signed_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // No overflow check needed as the case for the same range exponent is explicit above

          // Round
          return To(index(RP2::template round<From,To>(rhs)));
        }
      };

      ////

      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        signed_number<R1,P1,RP1,OP1,Opt1>,
        unsigned_number<R2,P2,RP2,OP2,Opt2>,
        true, false >
      {
        typedef signed_number<R1,P1,RP1,OP1,Opt1> From;
        typedef unsigned_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // Overflow
//          underlying_type indx(((rhs.count()) >> (P2-P1)));
//          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
//          {
//            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
//          }
//          if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
//          {
//            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
//          }
//
//          // Round
//          return To((index(RP2::template round<From,To>(rhs))));

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

      ////
      template <int R, int P1, typename RP1, typename OP1, typename Opt1,
                       int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        unsigned_number<R,P1,RP1,OP1,Opt1>,
        signed_number<R,P2,RP2,OP2,Opt2>,
        true, false >
      {
        typedef unsigned_number<R,P1,RP1,OP1,Opt1> From;
        typedef signed_number<R,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // Overflow could be possible because more resolution implies a bigger range when the range exponents are the same.
//          underlying_type indx(((rhs.count()) >> (P2-P1)));
//          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
//          {
//            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
//          }
//
//          // Round
//          return To((index(RP2::template round<From,To>(rhs))));

          return
          (
            (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          ? To(index(OP2::template on_positive_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
          : To((index(RP2::template round<From,To>(rhs))))
          );

        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        unsigned_number<R1,P1,RP1,OP1,Opt1>,
        signed_number<R2,P2,RP2,OP2,Opt2>,
        true, false >
      {
        typedef unsigned_number<R1,P1,RP1,OP1,Opt1> From;
        typedef signed_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // No overflow check needed as the case for the same range exponent is explicit above

          // Round
          return To(index(RP2::template round<From,To>(rhs)));
        }
      };

      ////

      template <int R, int P1, typename RP1, typename OP1, typename Opt1,
                       int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        unsigned_number<R,P1,RP1,OP1,Opt1>,
        unsigned_number<R,P2,RP2,OP2,Opt2>,
        true, false >
      {
        typedef unsigned_number<R,P1,RP1,OP1,Opt1> From;
        typedef unsigned_number<R,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
//          // Overflow could be possible because more resolution implies a bigger range when the range exponents are the same.
//          underlying_type indx(((rhs.count()) >> (P2-P1)));
//          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
//          {
//            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
//          }
//
//          // Round
//          return To((index(RP2::template round<From,To>(rhs))));

          return
          (
              (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          ? To(index(OP2::template on_positive_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
          : To((index(RP2::template round<From,To>(rhs))))
          );

        }
      };
      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        unsigned_number<R1,P1,RP1,OP1,Opt1>,
        unsigned_number<R2,P2,RP2,OP2,Opt2>,
        true, false >
      {
        typedef unsigned_number<R1,P1,RP1,OP1,Opt1> From;
        typedef unsigned_number<R2,P2,RP2,OP2,Opt2> To;
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

      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        signed_number<R1,P1,RP1,OP1,Opt1>,
        signed_number<R2,P2,RP2,OP2,Opt2>,
        false, false >
      {
        typedef signed_number<R1,P1,RP1,OP1,Opt1> From;
        typedef signed_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
//          // Overflow
//          underlying_type indx(((rhs.count()) >> (P2-P1)));
//          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
//          {
//            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
//          }
//          if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
//          {
//            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
//          }
//
//          // Round
//          return To(index(RP2::template round<From,To>(rhs)));

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
      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        unsigned_number<R1,P1,RP1,OP1,Opt1>,
        signed_number<R2,P2,RP2,OP2,Opt2>,
        false, false >
      {
        typedef unsigned_number<R1,P1,RP1,OP1,Opt1> From;
        typedef signed_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // Overflow
//          underlying_type indx(((rhs.count()) >> (P2-P1)));
//          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
//          {
//            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
//          }
//
//          // Round
//          return To(index(RP2::template round<From,To>(rhs)));

          return
          (
            (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          ? To(index(OP2::template on_positive_overflow<To,underlying_type>((((rhs.count()) >> (P2-P1))))))
          : To(index(RP2::template round<From,To>(rhs)))
          );

        }
      };

      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        signed_number<R1,P1,RP1,OP1,Opt1>,
        unsigned_number<R2,P2,RP2,OP2,Opt2>,
        false, false >
      {
        typedef signed_number<R1,P1,RP1,OP1,Opt1> From;
        typedef unsigned_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // Overflow
//          underlying_type indx(((rhs.count()) >> (P2-P1)));
//          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
//          {
//            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
//          }
//          if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
//          {
//            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
//          }
//
//          // Round
//          return To(index(RP2::template round<From,To>(rhs)));


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

      template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
                int R2, int P2, typename RP2, typename OP2, typename Opt2>
      struct number_cast<
        unsigned_number<R1,P1,RP1,OP1,Opt1>,
        unsigned_number<R2,P2,RP2,OP2,Opt2>,
        false, false >
      {
        typedef unsigned_number<R1,P1,RP1,OP1,Opt1> From;
        typedef unsigned_number<R2,P2,RP2,OP2,Opt2> To;
        typedef typename To::underlying_type underlying_type;

        BOOST_CONSTEXPR To operator()(const From& rhs) const
        {
          // Overflow
//          underlying_type indx(((rhs.count()) >> (P2-P1)));
//          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
//          {
//            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
//          }
//          if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
//          {
//            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
//          }
//
//          // Round
//          return To(index(RP2::template round<From,To>(rhs)));

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

    } // namespace detail
  } // namespace fixed_point

#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
  // common_type trait specializations

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
  struct common_type<Overflow,fixed_point::overflow::exception>
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
  struct common_type<Overflow,fixed_point::overflow::impossible>
  {
    typedef Overflow type;
  };

  template <>
  struct common_type<fixed_point::round::truncated,fixed_point::round::truncated>
  {
    typedef fixed_point::round::truncated type;
  };
  template <typename Round>
  struct common_type<Round,fixed_point::round::truncated>
  {
    typedef fixed_point::round::truncated type;
  };
  template <typename Round>
  struct common_type<fixed_point::round::truncated,Round>
  {
    typedef fixed_point::round::truncated type;
  };




  template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
            int R2, int P2, typename RP2, typename OP2, typename Opt2>
  struct common_type<
    fixed_point::unsigned_number<R1,P1,RP1,OP1,Opt1>,
    fixed_point::unsigned_number<R2,P2,RP2,OP2,Opt2> >
  {
    typedef fixed_point::unsigned_number<
        mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value,
        mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
        typename common_type<RP1,RP2>::type,
        typename common_type<OP1,OP2>::type,
        typename common_type<Opt1,Opt2>::type
    > type;
  };

  template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
            int R2, int P2, typename RP2, typename OP2, typename Opt2>
  struct common_type<
    fixed_point::signed_number<R1,P1,RP1,OP1,Opt1>,
    fixed_point::signed_number<R2,P2,RP2,OP2,Opt2> >
  {
    typedef fixed_point::signed_number<
        mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value,
        mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
        typename common_type<RP1,RP2>::type,
        typename common_type<OP1,OP2>::type,
        typename common_type<Opt1,Opt2>::type
    > type;
  };
  template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
            int R2, int P2, typename RP2, typename OP2, typename Opt2>
  struct common_type<
    fixed_point::signed_number<R1,P1,RP1,OP1,Opt1>,
    fixed_point::unsigned_number<R2,P2,RP2,OP2,Opt2> >
  {
    typedef fixed_point::signed_number<
        mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value,
        mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
        typename common_type<RP1,RP2>::type,
        typename common_type<OP1,OP2>::type,
        typename common_type<Opt1,Opt2>::type
    > type;
  };
  template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
            int R2, int P2, typename RP2, typename OP2, typename Opt2>
  struct common_type<
    fixed_point::unsigned_number<R1,P1,RP1,OP1,Opt1>,
    fixed_point::signed_number<R2,P2,RP2,OP2,Opt2> >
  {
    typedef fixed_point::signed_number<
        mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value,
        mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
        typename common_type<RP1,RP2>::type,
        typename common_type<OP1,OP2>::type,
        typename common_type<Opt1,Opt2>::type
    > type;
  };
#endif

  namespace fixed_point {


    /**
     * @brief Signed fixed point number.
     *
     * @TParams
     * @Param{Range,Range specified by an integer. The range of a signed number x is 2^Range < x < 2^Range. Note that the range interval is open for signed numbers.}
     * @Param{Resolution,resolution specified by an integer. The resolution of a fractional number x is 2^Resolution.}
     * @Param{Rounding,The rounding policy.}
     * @Param{Overflow,The overflow policy.}
     * @Param{Optimization,The optimization policy.}
     *
     * @Example For example, signed_number<8,-4> has values n such that -256 < n < 256 in increments of 2^(-4) = 1/16.
     */
    template <int Range, int Resolution, typename Rounding, typename Overflow, typename Optimization>
    class signed_number
    {
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      BOOST_MPL_ASSERT_MSG(Range>=Resolution, RANGE_MUST_BE_GREATER_EQUAL_THAN_RESOLUTION, (mpl::int_<Range>,mpl::int_<Resolution>));
#endif
    public:

      //! The underlying integer type
      typedef typename Optimization::template signed_integer_type<Range,Resolution>::type underlying_type;
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      BOOST_MPL_ASSERT_MSG((sizeof(underlying_type)*8) >= (Range-Resolution+1), UNDERLYING_TYPE_MUST_BE_LARGE_ENOUGH, (underlying_type));
      BOOST_MPL_ASSERT_MSG(boost::is_signed<underlying_type>::value, UNDERLYING_TYPE_MUST_BE_SIGNED, (underlying_type));
#endif

      // name the template parameters
      //! the Range parameter.
      BOOST_STATIC_CONSTEXPR int range_exp = Range;
      //! the Resolution parameter.
      BOOST_STATIC_CONSTEXPR int resolution_exp = Resolution;
      //! the Rounding parameter.
      typedef Rounding rounding_type;
      //! the Overflow parameter.
      typedef Overflow overflow_type;
      //! the Optimization parameter.
      typedef Optimization optimization_type;

      //! whether the tyoe is signed (always true).
      BOOST_STATIC_CONSTEXPR bool is_signed = true;
      //! The standard std::number_traits<>::digits.
      BOOST_STATIC_CONSTEXPR std::size_t digits = detail::signed_integer_traits<underlying_type,Range,Resolution>::digits;
      //! The standard std::number_traits<>::min_index
      BOOST_STATIC_CONSTEXPR underlying_type min_index = detail::signed_integer_traits<underlying_type,Range,Resolution>::const_min;
      //! The standard std::number_traits<>::max_index
      BOOST_STATIC_CONSTEXPR underlying_type max_index = detail::signed_integer_traits<underlying_type,Range,Resolution>::const_max;

      // construct/copy/destroy:

      /**
       * Default constructor.
       */
      BOOST_CONSTEXPR signed_number() {} // = default;
      /**
       * Copy constructor.
       */
      BOOST_CONSTEXPR signed_number(signed_number const& rhs) : value_(rhs.value_) {} // = default;

      //! Implicit constructor from a signed_number with no larger range and no better resolution
      template <int R, int P, typename RP, typename OP, typename Opt>
      signed_number(signed_number<R,P,RP,OP,Opt> const& rhs
          , typename boost::enable_if <
              mpl::and_ <
                mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >,
                mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >
              >
          >::type* = 0
        )
        : value_(fixed_point::detail::number_cast<signed_number<R,P,RP,OP,Opt>, signed_number, true, true>()(rhs).count())
      {
      }
      //! Implicit constructor from a unsigned_number with no larger range and no better resolution
      template <int R, int P, typename RP, typename OP, typename Opt>
      signed_number(unsigned_number<R,P,RP,OP,Opt> const& rhs
          , typename boost::enable_if <
              mpl::and_ <
                mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >,
                mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >
              >
          >::type* = 0
        )
        : value_(fixed_point::detail::number_cast<unsigned_number<R,P,RP,OP,Opt>, signed_number, true, true>()(rhs).count())
      {
      }

      //! explicit constructor from a signed_number with larger range or better resolution
      template <int R, int P, typename RP, typename OP, typename Opt>
      explicit signed_number(signed_number<R,P,RP,OP,Opt> const& rhs
          , typename boost::disable_if <
              mpl::and_ <
                mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >,
                mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >
              >
          >::type* = 0
        )
        : value_(fixed_point::detail::number_cast<signed_number<R,P,RP,OP,Opt>, signed_number,
            mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >::value,
            mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >::value
            >()(rhs).count())
      {
      }
      //! explicit constructor from a signed_number with larger range or better resolution
      template <int R, int P, typename RP, typename OP, typename Opt>
      explicit signed_number(unsigned_number<R,P,RP,OP,Opt> const& rhs
          , typename boost::disable_if <
              mpl::and_ <
                mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >,
                mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >
              >
          >::type* = 0
        )
        : value_(fixed_point::detail::number_cast<unsigned_number<R,P,RP,OP,Opt>, signed_number,
            mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >::value,
            mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >::value
            >()(rhs).count())
      {
      }

      //! destructor
      //~signed_number() {} //= default;


      /**
       * Explicit construction from an index.
       *
       * @Params
       * @Param{i,the index.}
       *
       * @Requires <c>min_index<=i<=max_index</c>.
       */
      template <typename UT>
      BOOST_CONSTEXPR explicit signed_number(index_tag<UT> i) : value_(i.get())
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
      BOOST_CONSTEXPR underlying_type count() const {return value_;}

      /**
       * @Returns the absolute zero.
       */
      static BOOST_CONSTEXPR signed_number zero()
      {
          return signed_number(index(0));
      }
      /**
       * @Returns the minimal value that can be represented.
       */
      static BOOST_CONSTEXPR signed_number min BOOST_PREVENT_MACRO_SUBSTITUTION ()
      {
        return signed_number(index(min_index));
      }
      /**
       * @Returns the maximal value that can be represented.
       */
      static BOOST_CONSTEXPR signed_number max BOOST_PREVENT_MACRO_SUBSTITUTION ()
      {
        return signed_number(index(max_index));
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
        if (Resolution>=0) return FP(1 << Resolution);
        else return FP(1)/(1 << -Resolution);
      }
      template <typename FP>
      static FP reconstruct(underlying_type k)
      {
        BOOST_ASSERT(min_index <= k && k <= max_index);

        return k*factor<FP>();
      }

      //! explicit conversion to FP.
      template <typename FP>
      FP as() const
      {
        return reconstruct<FP>(this->value_);
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

#if 0

      template <typename FP>
      static underlying_type integer_part(FP x)
      {
        return underlying_type(floor(x));
      }
      template <typename FP>
      static underlying_type classify(FP x) const
      {

        underlying_type indx =
        // Overflow
        if (x>max().as<FP>())
        {
          return overflow_type::template on_positive_overflow<To,underlying_type>(indx)));
        }
        if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
        {
          return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
        }

        // Round
        return To(index(RP2::template round<From,To>(rhs)));

        if (x<min().as<FP>())
          return min_index;
        if (x>max().as<FP>())
          return max_index;
        return integer_part(x/factor());
      }

      //! implicit conversion from float
      signed_number(float x) : value_(classify(x))
      {}
      //! implicit conversion from double
      signed_number(double x) : value_(classify(x))
      {}
      //! implicit conversion from long double
      signed_number(long double x) : value_(classify(x))
      {}


#endif

      // arithmetic

      /**
       * @Returns this instance.
       */
      signed_number  operator+() const
      {
        return *this;
      }
      /**
       * @Returns a new instance with the representation negated.
       */
      signed_number  operator-() const
      {
        // As the range is symmetric the type is preserved
        return signed_number(index(-value_));
      }
#if 0
      signed_number& operator++(
          typename boost::enable_if <
            is_equal<mpl::int_<Resolution>, mpl::int_<0> >
          >::type* = 0
          )
      {
        ++value_;
        return *this;
      }
      signed_number  operator++(int
          , typename boost::enable_if <
            is_equal<mpl::int_<Resolution>, mpl::int_<0> >
          >::type* = 0
          )
      {
        signed_number tmp=*this;
        ++value_;
        return tmp;
      }
      signed_number& operator--(
          typename boost::enable_if <
            is_equal<mpl::int_<Resolution>, mpl::int_<0> >
          >::type* = 0
          )
      {
        --value_;
        return *this;
      }
      signed_number  operator--(int
          , typename boost::enable_if <
          is_equal<mpl::int_<Resolution>, mpl::int_<0> >
      >::type* = 0
          )
      {
        signed_number tmp=*this;
        --value_;
        return tmp;
      }
#endif

      /**
       * @Effects As if <c>number_cast<signed_number>(*this+rhs)</c>
       * @Returns this instance.
       * @Throws Any exception the Overflow policy can throw.
       */
      signed_number& operator += (signed_number const& rhs)
      {
        signed_number tmp = number_cast<signed_number>(*this+rhs);
        value_ = tmp.count();
        return *this;
      }

      /**
      * @Effects As if <c>number_cast<signed_number>(*this-rhs)</c>
      * @Returns this instance.
      * @Throws Any exception the Overflow policy can throw.
      */
      signed_number& operator-=(const signed_number& rhs)
      {
        signed_number tmp = number_cast<signed_number>(*this-rhs);
        value_ = tmp.count();
        return *this;
      }

      /**
      * @Effects As if <c>number_cast<signed_number>(*this*rhs)</c>
      * @Returns this instance.
      * @Throws Any exception the Overflow policy can throw.
      */
      signed_number& operator*=(const signed_number& rhs)
      {
        signed_number tmp = number_cast<signed_number>((*this) * rhs);
        value_ = tmp.count();
        return *this;
      }

      /**
      * @Effects As if <c>divide<signed_number>(*this,rhs)</c>
      * @Returns this instance.
      * @Throws Any exception the Overflow policy can throw.
      */
      signed_number& operator/=(const signed_number& rhs)
      {
        signed_number tmp = divide<signed_number>(*this , rhs);
        value_ = tmp.count();
        return *this;
      }

      /**
       * Virtual scaling.
       *
       * @Returns a new instance with the same data representation and with the range and resolution increased by @c N.
       */
      template <std::size_t N>
      signed_number<Range+N, Resolution+N, Rounding, Overflow, Optimization>
      virtual_scale() const
      {
        return signed_number<Range+N, Resolution+N, Rounding, Overflow, Optimization>(index(count()));
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
        if (N>=0)
        {
          value_ <<= N;
        }
        else
        {
          signed_number tmp=
              divide<signed_number<Range, Resolution, RP, Overflow, Optimization> >(*this,
              signed_number<-N+1, -N, Rounding, Overflow, Optimization>(index(1)));
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
     * @Param{Resolution,resolution specified by an integer. The resolution of a fractional number x is 2^Resolution.}
     * @Param{Rounding,The rounding policy.}
     * @Param{Overflow,The overflow policy.}
     * @Param{Optimization,The optimization policy.}
     */
    template <int Range, int Resolution, typename Rounding, typename Overflow, typename Optimization>
    class unsigned_number
    {
#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      BOOST_MPL_ASSERT_MSG(Range>=Resolution, RANGE_MUST_BE_GREATER_EQUAL_THAN_RESOLUTION, (mpl::int_<Range>,mpl::int_<Resolution>));
#endif


    public:

      //! The underlying integer type
      typedef typename Optimization::template unsigned_integer_type<Range,Resolution>::type underlying_type;

#if !defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      BOOST_MPL_ASSERT_MSG((sizeof(underlying_type)*8) >= (Range-Resolution), UNDERLYING_TYPE_MUST_BE_LARGE_ENOUGH, (underlying_type));
      BOOST_MPL_ASSERT_MSG(!boost::is_signed<underlying_type>::value, UNDERLYING_TYPE_MUST_BE_UNSIGNED, (underlying_type));
#endif

      // name the template parameters
      //! the Range parameter.
      BOOST_STATIC_CONSTEXPR int range_exp = Range;
      //! the Resolution parameter.
      BOOST_STATIC_CONSTEXPR int resolution_exp = Resolution;
      //! the Rounding parameter.
      typedef Rounding rounding_type;
      //! the Overflow parameter.
      typedef Overflow overflow_type;
      //! the Optimization parameter.
      typedef Optimization optimization_type;

      //! whether the tyoe is signed (always @c false).
      BOOST_STATIC_CONSTEXPR bool is_signed = false;
      //! The standard std::number_traits<>::digits.
      BOOST_STATIC_CONSTEXPR std::size_t digits = detail::unsigned_integer_traits<underlying_type,Range,Resolution>::digits;
      //! The standard std::number_traits<>::min_index
      BOOST_STATIC_CONSTEXPR underlying_type min_index = detail::unsigned_integer_traits<underlying_type,Range,Resolution>::const_min;
      //! The standard std::number_traits<>::max_index
      BOOST_STATIC_CONSTEXPR underlying_type max_index = detail::unsigned_integer_traits<underlying_type,Range,Resolution>::const_max;

      // construct/copy/destroy:
      /**
       * Default constructor.
       */
      BOOST_CONSTEXPR unsigned_number() {} // = default;
      /**
       * Copy constructor.
       */
      BOOST_CONSTEXPR unsigned_number(unsigned_number const& rhs) : value_(rhs.value_) {} // = default;


      //! implicit constructor from a unsigned_number with no larger range and no better resolution
      template <int R, int P, typename RP, typename OP, typename Opt>
      unsigned_number(unsigned_number<R,P,RP,OP,Opt> const& rhs
          , typename boost::enable_if <
              mpl::and_ <
                mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >,
                mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >
              >
          >::type* = 0
        )
        : value_(fixed_point::detail::number_cast<unsigned_number<R,P,RP,OP,Opt>, unsigned_number, true, true>()(rhs).count())
      {
      }

      //! explicit constructor from a unsigned_number with larger range or better resolution
      template <int R, int P, typename RP, typename OP, typename Opt>
      explicit unsigned_number(unsigned_number<R,P,RP,OP,Opt> const& rhs
          , typename boost::disable_if <
              mpl::and_ <
                mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >,
                mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >
              >
          >::type* = 0
        )
        : value_(fixed_point::detail::number_cast<unsigned_number<R,P,RP,OP,Opt>, unsigned_number,
            mpl::less_equal < mpl::int_<R>, mpl::int_<Range> >::value,
            mpl::greater_equal < mpl::int_<P>, mpl::int_<Resolution> >::value
            >()(rhs).count())
      {
      }

      //! destructor
      //~unsigned_number() {} //= default;


      /**
       * Explicit construction from an index.
       *
       * @Params
       * @Param{i,the index.}
       *
       * @Requires <c>0<=i<=max_index</c>.
       */
      template <typename UT>
      explicit unsigned_number(index_tag<UT> i) : value_(i.get())
      {
        std::cout << __FILE__ << "[" <<__LINE__<<"] "<< int(i.get()) << std::endl;
        std::cout << __FILE__ << "[" <<__LINE__<<"] "<< int(min_index) << std::endl;
        std::cout << __FILE__ << "[" <<__LINE__<<"] "<< int(max_index) << std::endl;
        std::cout << __FILE__ << "[" <<__LINE__<<"] "<< int(digits) << std::endl;

        BOOST_ASSERT(i.get()>=min_index);
        BOOST_ASSERT(i.get()<=max_index);
      }

      // observers

      /**
       * Underlying integer type observer.
       *
       * @Returns the underlying representation.
       */
      BOOST_CONSTEXPR underlying_type count() const {return value_;}

      /**
       * @Returns the absolute zero.
       */
      static BOOST_CONSTEXPR unsigned_number zero()
      {
          return unsigned_number(index(0));
      }

      /**
       * @Returns the minimal value that can be represented.
       */
      static BOOST_CONSTEXPR unsigned_number min BOOST_PREVENT_MACRO_SUBSTITUTION ()
      {
        return unsigned_number(index(min_index));
      }

      /**
       * @Returns the maximal value that can be represented.
       */
      static BOOST_CONSTEXPR unsigned_number max BOOST_PREVENT_MACRO_SUBSTITUTION ()
      {
        return unsigned_number(index(max_index));
      }

      /**
       * @Returns the integral part of the fixed point number.
       */
      underlying_type integral_part() const
      {
        return count() >> resolution_exp;
      }

#if 0

      //! conversion factor.
      template <typename FP>
      static FP factor() const
      {
        if (Resolution>=0) return FP(1 << Resolution);
        else return FP(1)/(1 << -Resolution);
      }
      template <typename FP>
      static underlying_type integer_part(FP x)
      {
        return underlying_type(floor(x));
      }
      template <typename FP>
      static FP reconstruct(underlying_type index) const
      {
        BOOST_ASSERT(min_index <= k && k <= max_index);

        return k*factor<FP>();
      }
      template <typename FP>
      static underlying_type classify(FP x) const
      {
        if (x<min().as<FP>()) return min_index;
        if (x>max().as<FP>()) return max_index;
        return integer_part(x/factor());
      }

      //! implicit conversion from float
      unsigned_number(float x) : value_(classify(x))
      {}
      //! implicit conversion from double
      unsigned_number(double x) : value_(classify(x))
      {}
      //! implicit conversion from long double
      unsigned_number(long double x) : value_(classify(x))
      {}

      //! explicit conversion to FP.
      template <typename FP>
      FP as() const
      {
        return reconstruct<FP>(this->value_);
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
#endif

      // arithmetic

      /**
       * @Returns this instance.
       */
      unsigned_number  operator+() const
      {
        return *this;
      }
      /**
       * @Returns an instance of a signed fixed point nummber with the representation the negation of the representation of this.
       */
      signed_number<Range,Resolution,Rounding,Overflow,Optimization>
      operator-() const
      {
        return signed_number<Range,Resolution,Rounding,Overflow,Optimization>(index(-value_));
      }

      unsigned_number& operator++()
      {
        *this+=1;
        return *this;
      }
      unsigned_number  operator++(int)
      {
        unsigned_number tmp=*this;
        *this+=1;
        return tmp;
      }
      unsigned_number& operator--()
      {
        *this-=1;
        return *this;
      }
      unsigned_number  operator--(int)
      {
        unsigned_number tmp=*this;
        --value_;
        return tmp;
      }

      /**
       * @Effects As if <c>number_cast<unsigned_number>(*this+rhs)</c>
       * @Returns this instance.
       * @Throws Any exception the Overflow policy can throw.
       */
      unsigned_number& operator += (unsigned_number const& rhs)
      {
        unsigned_number tmp = number_cast<unsigned_number>((*this) + rhs);
        value_ = tmp.count();
        return *this;
      }

      /**
       * @Effects As if <c>number_cast<unsigned_number>(*this-rhs)</c>
       * @Returns this instance.
       * @Throws Any exception the Overflow policy can throw.
       */
      unsigned_number& operator-=(unsigned_number const& rhs)
      {
        unsigned_number tmp = number_cast<unsigned_number>((*this) - rhs);
        value_ = tmp.count();
        return *this;
      }
      /**
       * @Effects As if <c>number_cast<unsigned_number>(*this*rhs)</c>
       * @Returns this instance.
       * @Throws Any exception the Overflow policy can throw.
       */
      unsigned_number& operator*=(unsigned_number const& rhs)
      {
        unsigned_number tmp = number_cast<unsigned_number>((*this) * rhs);
        value_ = tmp.count();
        return *this;
      }

      /**
      * @Effects As if <c>divide<unsigned_number>(*this,rhs)</c>
      * @Returns this instance.
      * @Throws Any exception the Overflow policy can throw.
      */
      unsigned_number& operator/=(unsigned_number const& rhs)
      {
        unsigned_number tmp = divide<unsigned_number>(*this, rhs);
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
      unsigned_number<Range+N, Resolution+N, Rounding, Overflow, Optimization>
      virtual_scale() const
      {
        return unsigned_number<Range+N, Resolution+N, Rounding, Overflow, Optimization>(index(count()));
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
        if (N>=0)
        {
          value_ <<= N;
        }
        else
        {
          unsigned_number tmp=
              divide<unsigned_number<Range, Resolution, RP, Overflow, Optimization> >(*this,
              unsigned_number<-N+1, -N, Rounding, Overflow, Optimization>(index(1)));
          value_ = tmp.count();
        }
      }

    protected:
      //! The representation.
      underlying_type value_;
    };

    /**
     * unsigned_number compile time factory.
     *
     * @Returns an @c unsigned_number enough large to represent <c>Times*(2^Resolution)</c>.
     */
    template <int Times, int Resolution>
    BOOST_CONSTEXPR
    inline
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    unsigned_number< LOG2(ABS(Times+1)), Resolution>
#else
    unsigned_number< static_log2<mpl::abs<mpl::int_<Times+1> >::type::value>::value+Resolution, Resolution>
#endif
    to_unsigned_number()
    {
        return unsigned_number< static_log2<mpl::abs<mpl::int_<Times+1> >::type::value>::value+Resolution, Resolution>(index(Times));
    }
    /**
     * signed_number compile time factory.
     *
     * @Returns a @c signed_number enough large to represent <c>Times*(2^Resolution)</c>.
     */
    template <int Times, int Resolution>
    BOOST_CONSTEXPR
    inline
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    signed_number< LOG2(ABS(Times+1)), Resolution>
#else
    signed_number< static_log2<mpl::abs<mpl::int_<Times+1> >::type::value>::value+Resolution, Resolution>
#endif
    to_signed_number()
    {
        return signed_number< static_log2<mpl::abs<mpl::int_<Times+1> >::type::value>::value+Resolution, Resolution>(index(Times));
    }

//    /**
//     * unsigned_number compile time factory from integer and fractional parts
//     *
//     * @Returns an @c unsigned_number enough large to represent <c>Integral.Fractional</c>.
//     *
//     * @Example ratio_to_fp<ratio<314,100>,-32>
//     */
//    template <typename Ratio, int Resolution>
//    BOOST_CONSTEXPR
//    inline
//#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
//    unsigned_number< LOG2(ABS(Integral+1)), LOG2(ABS(Fractional+1))>
//#else
//    unsigned_number<
//      static_log2<mpl::abs<mpl::int_<Integral+1> >::type::value>::value,
//      static_log2<mpl::abs<mpl::int_<Fractional+1> >::type::value>::value>
//#endif
//    ratio_to_fp()
//    {
//      BOOST_CONSTEXPR intmax_t Resolution=static_log2<mpl::abs<mpl::int_<Fractional+1> >::type::value>::value;
//      return unsigned_number<
//            static_log2<mpl::abs<mpl::int_<Integral+1> >::type::value>::value,
//            Resolution
//            >(index(Times));
//    }


    // signed_number non-member arithmetic

    // mixed fixed point arithmetic

    /**
     * signed + signed -> signed.
     * @Returns <c>RT(incex(RT(lhs).count()+RT(rhs).count())</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    MAX(R1,R2)+1,
    MIN(P1,P2),
    CT(RP1,RP2),
    CT(OP1,OP2),
    CT(Opt1,Opt2)
#else
    mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
    mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
    typename common_type<RP1,RP2>::type,
    typename common_type<OP1,OP2>::type,
    typename common_type<Opt1,Opt2>::type
#endif
    >
    operator+(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     signed_number<
          mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
          mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;

      return result_type(index(result_type(lhs).count()+result_type(rhs).count()));
    }

    /**
     * unsigned + signed -> signed.
     * @Returns a signed fixed point enough large to avoid overflow.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    MAX(R1,R2)+1,
    MIN(P1,P2),
    CT(RP1,RP2),
    CT(OP1,OP2),
    CT(Opt1,Opt2)
#else
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
#endif
    >
    operator+(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     signed_number<
          mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
          mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;

      return result_type(index(result_type(lhs).count()+result_type(rhs).count()));
    }
    /**
     * signed + unsigned -> signed.
     * @Returns a signed fixed point enough large to avoid overflow.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    MAX(R1,R2)+1,
    MIN(P1,P2),
    CT(RP1,RP2),
    CT(OP1,OP2),
    CT(Opt1,Opt2)
#else
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
#endif
    >
    operator+(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     signed_number<
          mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
          mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;

      return result_type(index(result_type(lhs).count()+result_type(rhs).count()));
    }
    /**
     * unsigned + unsigned -> unsigned.
     * @Returns a unsigned fixed point enough large to avoid overflow.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    unsigned_number<
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    MAX(R1,R2)+1,
    MIN(P1,P2),
    CT(RP1,RP2),
    CT(OP1,OP2),
    CT(Opt1,Opt2)
#else
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
#endif
    >
    operator+(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     unsigned_number<
          mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
          mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;

      return result_type(index(result_type(lhs).count()+result_type(rhs).count()));
    }

    /**
     * signed - signed -> signed.
     * @Returns a signed fixed point enough large to avoid overflow.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    MAX(R1,R2)+1,
    MIN(P1,P2),
    CT(RP1,RP2),
    CT(OP1,OP2),
    CT(Opt1,Opt2)
#else
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
#endif
    >
    operator-(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     signed_number<
          mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
          mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;

      return result_type(index(result_type(lhs).count()-result_type(rhs).count()));
    }

    /**
     * unsigned - signed -> signed.
     * @Returns a signed fixed point enough large to avoid overflow.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    MAX(R1,R2)+1,
    MIN(P1,P2),
    CT(RP1,RP2),
    CT(OP1,OP2),
    CT(Opt1,Opt2)
#else
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
#endif
    >
    operator-(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     signed_number<
          mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
          mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;

      return result_type(index(result_type(lhs).count()-result_type(rhs).count()));
    }

    /**
     * signed - unsigned -> signed.
     * @Returns a signed fixed point enough large to avoid overflow.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    MAX(R1,R2)+1,
    MIN(P1,P2),
    CT(RP1,RP2),
    CT(OP1,OP2),
    CT(Opt1,Opt2)
#else
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
#endif
    >
    operator-(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     signed_number<
          mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
          mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;

      return result_type(index(result_type(lhs).count()-result_type(rhs).count()));
    }

    /**
     * unsigned - unsigned -> signed.
     * @Returns a signed fixed point enough large to avoid overflow.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    MAX(R1,R2)+1,
    MIN(P1,P2),
    CT(RP1,RP2),
    CT(OP1,OP2),
    CT(Opt1,Opt2)
#else
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
#endif
    >
    operator-(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     signed_number<
          mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
          mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;

      return result_type(index(result_type(lhs).count()-result_type(rhs).count()));
    }

    /**
     * signed * signed -> signed.
     * @Returns a signed fixed point enough large to avoid overflow.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
    R1+R2,
    P1+P2,
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    CT(RP1,RP2),
    CT(OP1,OP2),
    CT(Opt1,Opt2)
#else
    typename common_type<RP1,RP2>::type,
    typename common_type<OP1,OP2>::type,
    typename common_type<Opt1,Opt2>::type
#endif
    >
    operator*(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     signed_number<
          R1+R2,
          P1+P2,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;
      typedef typename result_type::underlying_type underlying_type;

      return result_type(index(underlying_type(lhs.count()) * rhs.count()));
    }

    /**
     * signed * unsigned -> signed.
     * @Returns a signed fixed point enough large to avoid overflow.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
    R1+R2,
    P1+P2,
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    CT(RP1,RP2),
    CT(OP1,OP2),
    CT(Opt1,Opt2)
#else
    typename common_type<RP1,RP2>::type,
    typename common_type<OP1,OP2>::type,
    typename common_type<Opt1,Opt2>::type
#endif
    >
    operator*(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     signed_number<
          R1+R2,
          P1+P2,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;
      typedef typename result_type::underlying_type underlying_type;

      return result_type(index(underlying_type(lhs.count()) * rhs.count()));
    }

    /**
     * unsigned * signed -> signed.
     * @Returns a signed fixed point enough large to avoid overflow.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
    R1+R2,
    P1+P2,
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    CT(RP1,RP2),
    CT(OP1,OP2),
    CT(Opt1,Opt2)
#else
    typename common_type<RP1,RP2>::type,
    typename common_type<OP1,OP2>::type,
    typename common_type<Opt1,Opt2>::type
#endif
    >
    operator*(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     signed_number<
          R1+R2,
          P1+P2,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;
      typedef typename result_type::underlying_type underlying_type;

      return result_type(index(underlying_type(lhs.count()) * rhs.count()));
    }

    /**
     * unsigned * unsigned -> unsigned.
     * @Returns a unsigned fixed point enough large to avoid overflow.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    unsigned_number<
    R1+R2,
    P1+P2,
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
    CT(RP1,RP2),
    CT(OP1,OP2),
    CT(Opt1,Opt2)
#else
    typename common_type<RP1,RP2>::type,
    typename common_type<OP1,OP2>::type,
    typename common_type<Opt1,Opt2>::type
#endif
    >
    operator*(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     unsigned_number<
          R1+R2,
          P1+P2,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;
      typedef typename result_type::underlying_type underlying_type;

      return result_type(index(underlying_type(lhs.count()) * rhs.count()));
    }

    /*
     * N = C*D+R
     * P*N = P*C*D+P*R
     * X=INT(P*N/D)=P*C
     * X/P <= N/D < (X+1)/P
     * 2X/2P <= N/D < 2(X+1)/2P
     *
     * exact    : X/P == N/D
     * near_down: X/P < N/D < (2X+1)/2P
     * half     :             (2X+1)/2P == N/D
     * near_up  :             (2X+1)/2P < N/D < (X+1)/P
     *
     *
     */

    /**
     * fixed point division giving the expected result type.
     * @Returns CT(lhs) / CT(rhs) taking in account the rounding policy of the expected result.
     */
    template <
              typename Res,
              int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    Res
    divide(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef Res result_type;
      typedef typename result_type::underlying_type underlying_type;

      typedef typename common_type<signed_number<R1,P1,RP1,OP1,Opt1>, signed_number<R2,P2,RP2,OP2,Opt2> >::type CT;
      //BOOST_STATIC_CONSTEXPR int P = Res::resolution_exp;

      //BOOST_STATIC_ASSERT((Res::digits>=(CT::digits-P)));
      BOOST_STATIC_ASSERT((Res::is_signed==CT::is_signed));
      BOOST_ASSERT_MSG(CT(rhs).count()!=0, "Division by 0");

//      underlying_type ci = detail::shift<typename CT::underlying_type, CT::digits, P>(CT(lhs).count()) / CT(rhs).count();
//      return result_type(index(ci)); // ....
      typedef typename result_type::rounding_type rounding_type;
      return result_type(index(rounding_type::template round_divide<Res>(CT(lhs), CT(rhs))));
    }

    /**
     * fixed point division giving the expected result type.
     * @Returns CT(lhs) / CT(rhs) taking in account the rounding policy of the expected result.
     */
    template <
              typename Res,
              int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    Res
    divide(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef Res result_type;
      typedef typename result_type::underlying_type underlying_type;
      typedef typename common_type<unsigned_number<R1,P1,RP1,OP1,Opt1>, unsigned_number<R2,P2,RP2,OP2,Opt2> >::type CT;
      //BOOST_STATIC_CONSTEXPR int P = Res::resolution_exp;

      //BOOST_STATIC_ASSERT((Res::digits>=(CT::digits-P)));
      BOOST_STATIC_ASSERT((Res::is_signed==CT::is_signed));
      BOOST_ASSERT_MSG(CT(rhs).count()!=0, "Division by 0");

//      underlying_type ci = detail::shift<typename CT::underlying_type, CT::digits, P>(CT(lhs).count()) / CT(rhs).count();
//      return result_type(index(ci)); // ....
      typedef typename result_type::rounding_type rounding_type;
      return result_type(index(rounding_type::template round_divide<Res>(CT(lhs), CT(rhs))));
    }

    /**
     * fixed point division giving the expected result type.
     * @Returns CT(lhs) / CT(rhs) taking in account the rounding policy of the expected result.
     */
    template <
              typename Res,
              int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    Res
    divide(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef Res result_type;
      typedef typename result_type::underlying_type underlying_type;
      typedef typename common_type<signed_number<R1,P1,RP1,OP1,Opt1>, unsigned_number<R2,P2,RP2,OP2,Opt2> >::type CT;
      //BOOST_STATIC_CONSTEXPR int P = Res::resolution_exp;

      //BOOST_STATIC_ASSERT((Res::digits>=(CT::digits-P)));
      BOOST_STATIC_ASSERT((Res::is_signed==CT::is_signed));
      BOOST_ASSERT_MSG(CT(rhs).count()!=0, "Division by 0");

//      underlying_type ci = detail::shift<typename CT::underlying_type, CT::digits, P>(CT(lhs).count()) / CT(rhs).count();
//      return result_type(index(ci)); // ....
      typedef typename result_type::rounding_type rounding_type;
      return result_type(index(rounding_type::template round_divide<Res>(CT(lhs), CT(rhs))));
    }

    /**
     * fixed point division giving the expected result type.
     * @Returns CT(lhs) / CT(rhs) taking in account the rounding policy of the expected result.
     */
    template <
              typename Res,
              int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    Res
    divide(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef Res result_type;
      typedef typename result_type::underlying_type underlying_type;
      typedef typename common_type<unsigned_number<R1,P1,RP1,OP1,Opt1>, signed_number<R2,P2,RP2,OP2,Opt2> >::type CT;
      //BOOST_STATIC_CONSTEXPR int P = Res::resolution_exp;

      //BOOST_STATIC_ASSERT((Res::digits>=(CT::digits-P)));
      BOOST_STATIC_ASSERT((Res::is_signed==CT::is_signed));
      BOOST_ASSERT_MSG(CT(rhs).count()!=0, "Division by 0");

//      underlying_type ci = detail::shift<typename CT::underlying_type, CT::digits, P>(CT(lhs).count()) / CT(rhs).count();
//      return result_type(index(ci)); // ....
      typedef typename result_type::rounding_type rounding_type;
      return result_type(index(rounding_type::template round_divide<Res>(CT(lhs), CT(rhs))));
    }

    /**
     * fixed point division  deducing the result type as <R1-P2, P1,R2>.
     * @Returns CT(lhs) / CT(rhs) taking in account the rounding policy of the result type.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      R1-P2,
      P1-R2,
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      CT(RP1,RP2),
      CT(OP1,OP2),
      CT(Opt1,Opt2)
#else
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
#endif
    >
    operator/(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     signed_number<
          R1-P2,
          P1-R2,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;

      return divide<result_type>(lhs,rhs);
    }

    /**
     * fixed point division  deducing the result type as <R1-P2, P1,R2>.
     * @Returns CT(lhs) / CT(rhs) taking in account the rounding policy of the result type.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      R1-P2,
      P1-R2,
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      CT(RP1,RP2),
      CT(OP1,OP2),
      CT(Opt1,Opt2)
#else
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
#endif
    >
    operator/(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     signed_number<
          R1-P2,
          P1-R2,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;

      return divide<result_type>(lhs,rhs);
    }

    /**
     * fixed point division  deducing the result type as <R1-P2, P1,R2>.
     * @Returns CT(lhs) / CT(rhs) taking in account the rounding policy of the result type.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      R1-P2,
      P1-R2,
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      CT(RP1,RP2),
      CT(OP1,OP2),
      CT(Opt1,Opt2)
#else
    typename common_type<RP1,RP2>::type,
    typename common_type<OP1,OP2>::type,
    typename common_type<Opt1,Opt2>::type
#endif
    >
    operator/(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     signed_number<
          R1-P2,
          P1-R2,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;

      return divide<result_type>(lhs,rhs);
    }

    /**
     * fixed point division  deducing the result type as <R1-P2, P1,R2>.
     * @Returns CT(lhs) / CT(rhs) taking in account the rounding policy of the result type.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    unsigned_number<
      R1-P2,
      P1-R2,
#if defined(BOOST_FIXED_POINT_DOXYGEN_INVOKED)
      CT(RP1,RP2),
      CT(OP1,OP2),
      CT(Opt1,Opt2)
#else
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
#endif
    >
    operator/(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef     unsigned_number<
          R1-P2,
          P1-R2,
          typename common_type<RP1,RP2>::type,
          typename common_type<OP1,OP2>::type,
          typename common_type<Opt1,Opt2>::type
        > result_type;

      return divide<result_type>(lhs,rhs);
    }


    // comparisons

    /**
     * @Returns As if <c>CT(lhs).count() == CT(rhs).count()</c> where CT is the common_type of the parameters..
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator==(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef typename common_type<signed_number<R1,P1,RP1,OP1,Opt1>, signed_number<R2,P2,RP2,OP2,Opt2> >::type CT;
      return CT(lhs).count() == CT(rhs).count();
    }

    /**
     * @Returns As if <c>CT(lhs).count() == CT(rhs).count()</c> where CT is the common_type of the parameters..
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator==(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef typename common_type<unsigned_number<R1,P1,RP1,OP1,Opt1>, unsigned_number<R2,P2,RP2,OP2,Opt2> >::type CT;
      return CT(lhs).count() == CT(rhs).count();
    }

    /**
     * @Returns <c>CT(lhs).count() != CT(rhs).count()</c> where CT is the common_type of the parameters..
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator!=(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return !(lhs == rhs);
    }

    /**
     * @Returns <c>CT(lhs).count() != CT(rhs).count()</c> where CT is the common_type of the parameters..
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator!=(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return !(lhs == rhs);
    }

    /**
     * @Returns <c>CT(lhs).count() < CT(rhs).count()</c> where CT is the common_type of the parameters..
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator<(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef typename common_type<signed_number<R1,P1,RP1,OP1,Opt1>, signed_number<R2,P2,RP2,OP2,Opt2> >::type CT;
      return CT(lhs).count() < CT(rhs).count();
    }

    /**
     * @Returns <c>CT(lhs).count() < CT(rhs).count()</c> where CT is the common_type of the parameters..
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator<(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef typename common_type<unsigned_number<R1,P1,RP1,OP1,Opt1>, unsigned_number<R2,P2,RP2,OP2,Opt2> >::type CT;
      return CT(lhs).count() < CT(rhs).count();
    }

    /**
     * @Returns <c>rhs < lhs</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator>(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return rhs < lhs;
    }
    /**
     * @Returns <c>rhs < lhs</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator>(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return rhs < lhs;
    }

    /**
     * @Returns <c>!(rhs < lhs)</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator<=(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return !(rhs < lhs);
    }
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    /**
     * @Returns <c>!(rhs < lhs)</c>.
     */
    inline
    bool
    operator<=(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return !(rhs < lhs);
    }

    /**
     * @Returns <c>!(lhs < rhs)</c>.
     */
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator>=(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return !(lhs < rhs);
    }
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    /**
     * @Returns <c>!(lhs < rhs)</c>.
     */
    inline
    bool
    operator>=(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return !(lhs < rhs);
    }

    /**
     * fixed point number cast.
     *
     * @Returns the conversion with possible reduced range and loss of resolution.
     */
    template <typename To, typename From>
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
  template <int R, int P, typename RP, typename OP, typename Opt>
  struct numeric_limits<boost::fixed_point::signed_number<R,P,RP,OP,Opt> >
  {
    typedef boost::fixed_point::signed_number<R,P,RP,OP,Opt> rep;
  public:
    BOOST_STATIC_CONSTEXPR bool is_specialized = true;
    inline static rep min() { return rep::min(); }
    inline static rep max() { return rep::max(); }
    inline static rep lowest() { return rep::lowest(); }
    BOOST_STATIC_CONSTEXPR int digits = rep::digits;
    //BOOST_STATIC_CONSTEXPR int digits10 = rep::digits10;
    //BOOST_STATIC_CONSTEXPR int max_digits10 = rep::max_digits10;
    BOOST_STATIC_CONSTEXPR bool is_signed = true;
    BOOST_STATIC_CONSTEXPR bool is_integer = false;
    BOOST_STATIC_CONSTEXPR bool is_exact  = true;
    BOOST_STATIC_CONSTEXPR int radix = 2;
    //inline static rep epsilon() { return rep::epsilon(); }
    //inline static rep round_error() { return rep::round_error(); }
    //BOOST_STATIC_CONSTEXPR int min_exponent = rep::min_exponent;
    //BOOST_STATIC_CONSTEXPR int min_exponent10  = rep::min_exponent10;
    //BOOST_STATIC_CONSTEXPR int max_exponent = rep::max_exponent;
    //BOOST_STATIC_CONSTEXPR int max_exponent10 = rep::max_exponent10;
    BOOST_STATIC_CONSTEXPR bool has_infinity = false;
    BOOST_STATIC_CONSTEXPR bool has_quiet_NaN = false;
    BOOST_STATIC_CONSTEXPR bool has_signaling_NaN = false;
    //BOOST_STATIC_CONSTEXPR float_denorm_style has_denorm = denorm_absent;
    //BOOST_STATIC_CONSTEXPR bool has_denorm_loss = false;
    //inline static rep infinity() { return rep::infinity(); }
    //inline static rep quiet_NaN() { return rep::quiet_NaN(); }
    //inline static rep signaling_NaN() { return rep::signaling_NaN(); }
    //inline static rep denorm_min() { return rep::denorm_min<Q>(); }
    //BOOST_STATIC_CONSTEXPR bool is_iec559 = false;
    BOOST_STATIC_CONSTEXPR bool is_bounded = true;
    BOOST_STATIC_CONSTEXPR bool is_modulo = RP::is_modulo;
    BOOST_STATIC_CONSTEXPR bool traps = true;
    //BOOST_STATIC_CONSTEXPR bool tinyness_before = rep::tinyness_before;
    BOOST_STATIC_CONSTEXPR float_round_style round_style = RP::round_style;
  };
}

#endif // header
