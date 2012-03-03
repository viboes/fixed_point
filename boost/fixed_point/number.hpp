//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Vicente J. Botet Escriba 2012.
// Distributed under the Boost
// Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/fixed_point for documentation.
//
//////////////////////////////////////////////////////////////////////////////

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
      template <typename CT, int Bits, int P, bool IsSigned=is_signed<CT>::value, bool IsPositive=(P>0)>
      struct shift_impl;
      template <typename CT, int Bits, int P, bool IsSigned>
      struct shift_impl<CT,Bits, P, IsSigned, true> {
        BOOST_STATIC_CONSTEXPR std::size_t digits = Bits-P;
        typedef CT result_type;
        static  CT apply(CT v)
        {
          return v >> P;
        }
      };
      template <typename CT, int Bits, int P>
      struct shift_impl<CT,Bits, P,true,false> {
        BOOST_STATIC_CONSTEXPR std::size_t digits = Bits-P;
        typedef typename ::boost::int_t<Bits-P>::fast result_type;
        static result_type apply(CT v)
        {
          return result_type(v) << -P;
        }
      };
      template <typename CT, int Bits, int P>
      struct shift_impl<CT,Bits, P,false,false> {
        BOOST_STATIC_CONSTEXPR std::size_t digits = Bits-P;
        typedef typename ::boost::uint_t<Bits-P>::fast result_type;
        static  result_type apply(CT v)
        {
          return result_type(v) << -P;
        }
      };
      template <typename UT, int Bits, int P>
      typename shift_impl<UT,Bits,P>::result_type shift(UT v) {
        return shift_impl<UT,Bits,P>::apply(v);
      }
    }

    struct positive_overflow {};
    struct negative_overflow {};

    namespace round
    {
      struct fastest {
        //BOOST_STATIC_CONSTEXPR std::round_to_nearest  round_style = std::round_indeterminate;
      };
      struct negative {
        //BOOST_STATIC_CONSTEXPR std::round_to_nearest  round_style = std::round_toward_infinity;
        template <typename From, typename To>
        static typename To::underlying_type round(From const& rhs)
        {
          BOOST_STATIC_CONSTEXPR typename To::underlying_type d = To::resolution_exp-From::resolution_exp;

          return typename To::underlying_type(rhs.count()) >> d;
        }
        template <typename To, typename From>
        static typename To::underlying_type round_divide(From const& lhs, From const& rhs)
        {
          typedef typename To::underlying_type result_type;
          result_type ci = detail::shift<typename From::underlying_type, From::digits, To::resolution_exp>(lhs.count()) / rhs.count();
          if (ci>=0 )
          {
            return ci;
          } else {
            result_type ri = detail::shift<typename From::underlying_type, From::digits, To::resolution_exp>(lhs.count()) % rhs.count();
            if (ri==0)
              return ci;
            else
              return ci-1;
          }
        }
      };
      struct truncated {
        //BOOST_STATIC_CONSTEXPR std::round_to_nearest  round_style = std::round_toward_zero;
        template <typename From, typename To>
        static typename To::underlying_type round(From const& rhs)
        {
          BOOST_STATIC_CONSTEXPR typename To::underlying_type d = To::resolution_exp-From::resolution_exp;
          typename To::underlying_type m(((rhs.count()>0)?rhs.count():-rhs.count()));
          typename To::underlying_type s(((rhs.count()>0)?+1:-1));

          return s * (m >> d);
        }
        template <typename To, typename From>
        static typename To::underlying_type round_divide(From const& lhs, From const& rhs)
        {
          typedef typename To::underlying_type result_type;
          result_type ci = detail::shift<typename From::underlying_type, From::digits, To::resolution_exp>(lhs.count()) / rhs.count();
          return ci;
        }
      };
      struct positive {
        //BOOST_STATIC_CONSTEXPR std::round_to_nearest  round_style = std::round_toward_neg_infinity;
        template <typename From, typename To>
        static typename To::underlying_type round(From const& rhs)
        {
          BOOST_STATIC_CONSTEXPR typename To::underlying_type d = To::resolution_exp-From::resolution_exp;
          typename To::underlying_type w = (1<<d)-1;
          typename To::underlying_type i = rhs.count();

          return (i+w) >> d;
        }
        template <typename To, typename From>
        static typename To::underlying_type round_divide(From const& lhs, From const& rhs)
        {
          typedef typename To::underlying_type result_type;
          result_type ci = detail::shift<typename From::underlying_type, From::digits, To::resolution_exp>(lhs.count()) / rhs.count();
          if (ci>=0 )
          {
            result_type ri = detail::shift<typename From::underlying_type, From::digits, To::resolution_exp>(lhs.count()) % rhs.count();
            if (ri==0)
              return ci;
            else
              return ci+1;
          } else {
              return ci;
          }
        }
      };
      struct classic {
        //BOOST_STATIC_CONSTEXPR std::round_to_nearest  round_style =  std::round_to_nearest;
      };
      struct near_even {
        //BOOST_STATIC_CONSTEXPR std::round_to_nearest  round_style =  std::round_to_nearest;
      };
      struct nearest_odd {
        //BOOST_STATIC_CONSTEXPR std::round_to_nearest round_style;
      };
    }
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
      struct time {
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
    /**
     * fixed point unsigned number .
     * @tparam Range the 2-exponent of the range. The range is 0<=x<2^Range,
     * @tparam Resolution the 2-exponent of the step between two fixed point numbers
     * @tparam Round the rounding policy
     * @tparam Overflow the overflow policy
     * @tparam Optimization the Optimization policy
     */
    template <int Range, int Resolution, typename Rounding=round::negative, typename Overflow=overflow::exception, typename Optimization=optimization::space>
    class unsigned_number;

    /**
     * fixed point signed number .
     * @tparam Range the 2-exponent of the range. the range is -2^Range<x<2^Range.
     * @tparam Resolution the 2-exponent of the step between two fixed point numbers
     * @tparam Round the rounding policy
     * @tparam Overflow the overflow policy
     * @tparam Optimization the Optimization policy
     */
    template <int Range, int Resolution, typename Rounding=round::negative, typename Overflow=overflow::exception, typename Optimization=optimization::space>
    class signed_number;

    // named parameter like class, allowing to make a specific overload when the integer must be taken by the index.
    template <typename T>
    struct index_tag
    {
      typedef T type;
      T value;
      index_tag(T v) : value(v) {}
      T get() { return value; }

    };

    //! helper function to make easier the use of index_tag.
    template <typename T>
    struct index_tag<T> index(T v) { return index_tag<T>(v); }

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
        bool LE_Range=mpl::less_equal < mpl::int_<From::range_exp>, mpl::int_<To::range_exp> >::type::value,
        bool GE_Resolution=mpl::greater_equal < mpl::int_<From::resolution_exp>, mpl::int_<To::resolution_exp> >::type::value
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

          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
          // Overflow
          if (indx < To::min_index)
          {
            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          }
          // No round needed
          return To(index(underlying_type(rhs.count()) << (P1-P2)));
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

          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
          // Overflow impossible
          if (indx > To::max_index)
          {
            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          }
          if (indx < To::min_index)
          {
            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          }

          // No round needed
          return To(index(indx));
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

          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
          // Overflow impossible
          if (indx > To::max_index)
          {
            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          }
          if (indx < To::min_index)
          {
            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          }

          // No round needed
          return To(index(indx));
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

          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
          // Overflow
          if (indx > To::max_index)
          {
            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          }

          // No round needed
          return To(index(indx));
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

          underlying_type indx((underlying_type(rhs.count()) << (P1-P2)));
          // Overflow
          if (indx > To::max_index)
          {
            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          }

          // No round needed
          return To(index(indx));
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
          // Overflow could be possible because more resolution implies a bigger range when the range exponents are the same.
          underlying_type indx(((rhs.count()) >> (P2-P1)));
          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          {
            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          }
          if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
          {
            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          }

          // Round
          To res((index(RP2::template round<From,To>(rhs))));
          return res;
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
          underlying_type indx(((rhs.count()) >> (P2-P1)));
          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          {
            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          }
          if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
          {
            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          }

          // Round
          return To((index(RP2::template round<From,To>(rhs))));
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
          underlying_type indx(((rhs.count()) >> (P2-P1)));
          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          {
            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          }

          // Round
          return To((index(RP2::template round<From,To>(rhs))));
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
          // Overflow could be possible because more resolution implies a bigger range when the range exponents are the same.
          underlying_type indx(((rhs.count()) >> (P2-P1)));
          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          {
            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          }

          // Round
          return To((index(RP2::template round<From,To>(rhs))));
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
          // Overflow
          underlying_type indx(((rhs.count()) >> (P2-P1)));
          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          {
            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          }
          if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
          {
            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          }

          // Round
          return To(index(RP2::template round<From,To>(rhs)));
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
          underlying_type indx(((rhs.count()) >> (P2-P1)));
          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          {
            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          }

          // Round
          return To(index(RP2::template round<From,To>(rhs)));
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
          underlying_type indx(((rhs.count()) >> (P2-P1)));
          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          {
            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          }
          if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
          {
            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          }

          // Round
          return To(index(RP2::template round<From,To>(rhs)));
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
          underlying_type indx(((rhs.count()) >> (P2-P1)));
          if (rhs.count() > (typename From::underlying_type(To::max_index)<<(P2-P1)))
          {
            return To(index(OP2::template on_positive_overflow<To,underlying_type>(indx)));
          }
          if (rhs.count() < (typename From::underlying_type(To::min_index)<<(P2-P1)))
          {
            return To(index(OP2::template on_negative_overflow<To,underlying_type>(indx)));
          }

          // Round
          return To(index(RP2::template round<From,To>(rhs)));
        }
      };

    } // namespace detail
  } // namespace fixed_point

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

  namespace fixed_point {


    //! signed_number associated to a quantizer
    template <int Range, int Resolution, typename Rounding, typename Overflow, typename Optimization>
    class signed_number
    {
      BOOST_MPL_ASSERT_MSG(Range>=Resolution, RANGE_MUST_BE_GREATER_EQUAL_THAN_RESOLUTION, (mpl::int_<Range>,mpl::int_<Resolution>));


    public:

      //! The underlying integer type
      typedef typename Optimization::template signed_integer_type<Range,Resolution>::type underlying_type;

      // name the template parameters
      BOOST_STATIC_CONSTEXPR int range_exp = Range;
      BOOST_STATIC_CONSTEXPR int resolution_exp = Resolution;
      typedef Rounding rounding_type;
      typedef Overflow overflow_type;
      typedef Optimization optimization_type;

      BOOST_STATIC_CONSTEXPR bool is_signed = boost::is_signed<underlying_type>::value;
      BOOST_STATIC_CONSTEXPR std::size_t digits = detail::signed_integer_traits<underlying_type,Range,Resolution>::digits;
      BOOST_STATIC_CONSTEXPR underlying_type min_index = detail::signed_integer_traits<underlying_type,Range,Resolution>::const_min;
      BOOST_STATIC_CONSTEXPR underlying_type max_index = detail::signed_integer_traits<underlying_type,Range,Resolution>::const_max;


      template <int R, int P, typename RP, typename OP, typename Opt>
      static bool positive_overflows(signed_number<R,P,RP,OP,Opt> const& rhs)
      {
        return false;
      }
      template <int R, int P, typename RP, typename OP, typename Opt>
      static bool negative_overflows(signed_number<R,P,RP,OP,Opt> const& rhs)
      {
        return false;
      }


      //! construct/copy/destroy:
      signed_number() {} // = default;
      signed_number(signed_number const& rhs) : value_(rhs.value_) {} // = default;


      //! implicit constructor from a signed_number with no larger range and no better resolution
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
      //! implicit constructor from a signed_number with no larger range and no better resolution
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

      ~signed_number() {} //= default;


      //! explicit construction from an index.
      template <typename UT>
      explicit signed_number(index_tag<UT> i) : value_(i.get())
      {
        //std::cout << __FILE__ << "[" <<__LINE__<<"] "<<int(min_index) <<std::endl;
        //std::cout << __FILE__ << "[" <<__LINE__<<"] "<<int(max_index) <<std::endl;
        //std::cout << __FILE__ << "[" <<__LINE__<<"] "<<int(i.get()) <<std::endl;
        BOOST_ASSERT(i.get()>=min_index);
        BOOST_ASSERT(i.get()<=max_index);
      }

      //! observer

      underlying_type count() const {return value_;}

      static BOOST_CONSTEXPR signed_number zero()
      {
          return signed_number(index(0));
      }
      static BOOST_CONSTEXPR signed_number min BOOST_PREVENT_MACRO_SUBSTITUTION ()
      {
        return signed_number(index(min_index));
      }
      static BOOST_CONSTEXPR signed_number max BOOST_PREVENT_MACRO_SUBSTITUTION ()
      {
        return signed_number(index(max_index));
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
      signed_number(float x) : value_(classify(x))
      {}
      //! implicit conversion from double
      signed_number(double x) : value_(classify(x))
      {}
      //! implicit conversion from long double
      signed_number(long double x) : value_(classify(x))
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

      signed_number  operator+() const
      {
        return *this;
      }
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
      signed_number& operator += (signed_number const& rhs)
      {
        value_ += rhs.count();
        return *this;
      }
      signed_number& operator-=(const signed_number& rhs)
      {
        value_ -= rhs.count();
        return *this;
      }
      signed_number& operator*=(const signed_number& rhs)
      {
        value_ *= rhs.count();
        return *this;
      }
      signed_number& operator/=(const signed_number& rhs)
      {
        value_ /= rhs.count();
        return *this;
      }
    protected:
      underlying_type value_;
    };

    //! signed_number associated to a quantizer
    template <int Range, int Resolution, typename Rounding, typename Overflow, typename Optimization>
    class unsigned_number
    {
      BOOST_MPL_ASSERT_MSG(Range>=Resolution, RANGE_MUST_BE_GREATER_EQUAL_THAN_RESOLUTION, (mpl::int_<Range>,mpl::int_<Resolution>));


    public:

      //! The underlying integer type
      typedef typename Optimization::template signed_integer_type<Range,Resolution>::type underlying_type;

      // name the template parameters
      BOOST_STATIC_CONSTEXPR int range_exp = Range;
      BOOST_STATIC_CONSTEXPR int resolution_exp = Resolution;
      typedef Rounding rounding_type;
      typedef Overflow overflow_type;
      typedef Optimization optimization_type;

      BOOST_STATIC_CONSTEXPR bool is_signed = boost::is_signed<underlying_type>::value;
      BOOST_STATIC_CONSTEXPR std::size_t digits = detail::unsigned_integer_traits<underlying_type,Range,Resolution>::digits;
      BOOST_STATIC_CONSTEXPR underlying_type min_index = detail::unsigned_integer_traits<underlying_type,Range,Resolution>::const_min;
      BOOST_STATIC_CONSTEXPR underlying_type max_index = detail::unsigned_integer_traits<underlying_type,Range,Resolution>::const_max;


      //! construct/copy/destroy:
      unsigned_number() {} // = default;
      unsigned_number(unsigned_number const& rhs) : value_(rhs.value_) {} // = default;


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

      ~unsigned_number() {} //= default;


      //! explicit construction from an index.
      template <typename UT>
      explicit unsigned_number(index_tag<UT> i) : value_(i.get())
      {
        //std::cout << __FILE__ << "[" <<__LINE__<<"] "<<int(min_index) <<std::endl;
        //std::cout << __FILE__ << "[" <<__LINE__<<"] "<<int(max_index) <<std::endl;
        //std::cout << __FILE__ << "[" <<__LINE__<<"] "<<int(i.get()) <<std::endl;
        BOOST_ASSERT(i.get()>=min_index);
        BOOST_ASSERT(i.get()<=max_index);
      }

      //! observer

      underlying_type count() const {return value_;}

      static BOOST_CONSTEXPR unsigned_number zero()
      {
          return unsigned_number(index(0));
      }
      static BOOST_CONSTEXPR unsigned_number min BOOST_PREVENT_MACRO_SUBSTITUTION ()
      {
        return unsigned_number(index(min_index));
      }
      static BOOST_CONSTEXPR unsigned_number max BOOST_PREVENT_MACRO_SUBSTITUTION ()
      {
        return unsigned_number(index(max_index));
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

      unsigned_number  operator+() const
      {
        return *this;
      }
      signed_number<Range,Resolution,Rounding,Overflow,Optimization>
      operator-() const
      {
        return signed_number<Range,Resolution,Rounding,Overflow,Optimization>(index(-value_));
      }

#if 0
      unsigned_number& operator++(
          typename boost::enable_if <
            is_equal<mpl::int_<Resolution>, mpl::int_<0> >
          >::type* = 0
          )
      {
        ++value_;
        return *this;
      }
      unsigned_number  operator++(int
          , typename boost::enable_if <
            is_equal<mpl::int_<Resolution>, mpl::int_<0> >
          >::type* = 0
          )
      {
        unsigned_number tmp=*this;
        ++value_;
        return tmp;
      }
      unsigned_number& operator--(
          typename boost::enable_if <
            is_equal<mpl::int_<Resolution>, mpl::int_<0> >
          >::type* = 0
          )
      {
        --value_;
        return *this;
      }
      unsigned_number  operator--(int
          , typename boost::enable_if <
          is_equal<mpl::int_<Resolution>, mpl::int_<0> >
      >::type* = 0
          )
      {
        unsigned_number tmp=*this;
        --value_;
        return tmp;
      }
#endif
      unsigned_number& operator += (unsigned_number const& rhs)
      {
        value_ += rhs.count();
        return *this;
      }
      unsigned_number& operator-=(const unsigned_number& rhs)
      {
        value_ -= rhs.count();
        return *this;
      }
      unsigned_number& operator*=(const unsigned_number& rhs)
      {
        value_ *= rhs.count();
        return *this;
      }
      unsigned_number& operator/=(const unsigned_number& rhs)
      {
        value_ /= rhs.count();
        return *this;
      }
    protected:
      underlying_type value_;
    };

    template <int Times, int Resolution>
    BOOST_CONSTEXPR
    inline
    unsigned_number< static_log2<mpl::abs<mpl::int_<Times+1> >::type::value>::value, Resolution>
    to_unsigned_number()
    {
        return unsigned_number< static_log2<mpl::abs<mpl::int_<Times+1> >::type::value>::value, Resolution>(index(Times));
    }
    template <int Times, int Resolution>
    BOOST_CONSTEXPR
    inline
    signed_number< static_log2<mpl::abs<mpl::int_<Times+1> >::type::value>::value, Resolution>
    to_signed_number()
    {
        return signed_number< static_log2<mpl::abs<mpl::int_<Times+1> >::type::value>::value, Resolution>(index(Times));
    }

    // signed_number non-member arithmetic

    //!  +

    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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
    //! unsigned_number +
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    unsigned_number<
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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

    //!  -

    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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

    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      mpl::max<mpl::int_<R1>,mpl::int_<R2> >::type::value+1,
      mpl::min<mpl::int_<P1>,mpl::int_<P2> >::type::value,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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

    //!  *

    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      R1+R2,
      P1+P2,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      R1+R2,
      P1+P2,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      R1+R2,
      P1+P2,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    unsigned_number<
      R1+R2,
      P1+P2,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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
    //!  divide

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
      BOOST_STATIC_CONSTEXPR int P = Res::resolution_exp;

      BOOST_STATIC_ASSERT((Res::digits>=(CT::digits-P)));
      BOOST_STATIC_ASSERT((Res::is_signed==CT::is_signed));
      BOOST_ASSERT_MSG(CT(rhs).count()!=0, "Division by 0");

//      underlying_type ci = detail::shift<typename CT::underlying_type, CT::digits, P>(CT(lhs).count()) / CT(rhs).count();
//      return result_type(index(ci)); // ....
      typedef typename result_type::rounding_type rounding_type;
      return result_type(index(rounding_type::template round_divide<Res>(CT(lhs), CT(rhs))));
    }

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
      BOOST_STATIC_CONSTEXPR int P = Res::resolution_exp;

      BOOST_STATIC_ASSERT((Res::digits>=(CT::digits-P)));
      BOOST_STATIC_ASSERT((Res::is_signed==CT::is_signed));
      BOOST_ASSERT_MSG(CT(rhs).count()!=0, "Division by 0");

//      underlying_type ci = detail::shift<typename CT::underlying_type, CT::digits, P>(CT(lhs).count()) / CT(rhs).count();
//      return result_type(index(ci)); // ....
      typedef typename result_type::rounding_type rounding_type;
      return result_type(index(rounding_type::template round_divide<Res>(CT(lhs), CT(rhs))));
    }

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
      BOOST_STATIC_CONSTEXPR int P = Res::resolution_exp;

      BOOST_STATIC_ASSERT((Res::digits>=(CT::digits-P)));
      BOOST_STATIC_ASSERT((Res::is_signed==CT::is_signed));
      BOOST_ASSERT_MSG(CT(rhs).count()!=0, "Division by 0");

//      underlying_type ci = detail::shift<typename CT::underlying_type, CT::digits, P>(CT(lhs).count()) / CT(rhs).count();
//      return result_type(index(ci)); // ....
      typedef typename result_type::rounding_type rounding_type;
      return result_type(index(rounding_type::template round_divide<Res>(CT(lhs), CT(rhs))));
    }

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
      BOOST_STATIC_CONSTEXPR int P = Res::resolution_exp;

      BOOST_STATIC_ASSERT((Res::digits>=(CT::digits-P)));
      BOOST_STATIC_ASSERT((Res::is_signed==CT::is_signed));
      BOOST_ASSERT_MSG(CT(rhs).count()!=0, "Division by 0");

//      underlying_type ci = detail::shift<typename CT::underlying_type, CT::digits, P>(CT(lhs).count()) / CT(rhs).count();
//      return result_type(index(ci)); // ....
      typedef typename result_type::rounding_type rounding_type;
      return result_type(index(rounding_type::template round_divide<Res>(CT(lhs), CT(rhs))));
    }

    //!  /
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      R1-P2,
      P1-R2,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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

    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      R1-P2,
      P1-R2,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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

    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    signed_number<
      R1-P2,
      P1-R2,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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

    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    unsigned_number<
      R1-P2,
      P1-R2,
      typename common_type<RP1,RP2>::type,
      typename common_type<OP1,OP2>::type,
      typename common_type<Opt1,Opt2>::type
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

    //  ==
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator==(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef typename common_type<signed_number<R1,P1,RP1,OP1,Opt1>, signed_number<R2,P2,RP2,OP2,Opt2> >::type CT;
      return CT(lhs).count() == CT(rhs).count();
    }

    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator==(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef typename common_type<unsigned_number<R1,P1,RP1,OP1,Opt1>, unsigned_number<R2,P2,RP2,OP2,Opt2> >::type CT;
      return CT(lhs).count() == CT(rhs).count();
    }

    //  !=
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator!=(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return !(lhs == rhs);
    }
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator!=(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return !(lhs == rhs);
    }

    //  <
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator<(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef typename common_type<signed_number<R1,P1,RP1,OP1,Opt1>, signed_number<R2,P2,RP2,OP2,Opt2> >::type CT;
      return CT(lhs).count() < CT(rhs).count();
    }

    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator<(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      typedef typename common_type<unsigned_number<R1,P1,RP1,OP1,Opt1>, unsigned_number<R2,P2,RP2,OP2,Opt2> >::type CT;
      return CT(lhs).count() < CT(rhs).count();
    }

    //  >
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator>(signed_number<R1,P1,RP1,OP1,Opt1> const& lhs, signed_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return rhs < lhs;
    }
    template <int R1, int P1, typename RP1, typename OP1, typename Opt1,
              int R2, int P2, typename RP2, typename OP2, typename Opt2>
    inline
    bool
    operator>(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return rhs < lhs;
    }

    //  <=
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
    inline
    bool
    operator<=(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return !(rhs < lhs);
    }

    //  >=
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
    inline
    bool
    operator>=(unsigned_number<R1,P1,RP1,OP1,Opt1> const& lhs, unsigned_number<R2,P2,RP2,OP2,Opt2> const& rhs)
    {
      return !(lhs < rhs);
    }

    // number_cast
    template <typename To, typename From>
    To number_cast(From const& f)
    {
      return fixed_point::detail::number_cast<From, To>()(f);
    }
  }
}

namespace std {
  // numeric limits trait specializations
  template <int R, int P, typename RP, typename OP, typename Opt>
  struct numeric_limits<boost::fixed_point::signed_number<R,P,RP,OP,Opt> >
  {
    typedef boost::fixed_point::signed_number<R,P,RP,OP,Opt> rep;
  public:
    BOOST_STATIC_CONSTEXPR bool is_specialized = true;
    inline static rep min() { return rep::min(); }
    inline static rep max() { return rep::max(); }
    inline static rep lowest() { return rep::lowest(); }
    //BOOST_STATIC_CONSTEXPR int digits = Q::digits;
    //BOOST_STATIC_CONSTEXPR int digits10 = Q::digits10;
    //BOOST_STATIC_CONSTEXPR int max_digits10 = Q::max_digits10;
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

#define BOOST_TYPEOF_SILENT
#include <boost/typeof/typeof.hpp>

#include BOOST_TYPEOF_INCREMENT_REGISTRATION_GROUP()

BOOST_TYPEOF_REGISTER_TEMPLATE(boost::fixed_point::signed_number, (int)(int)(typename)(typename)(typename))
BOOST_TYPEOF_REGISTER_TEMPLATE(boost::fixed_point::unsigned_number, (int)(int)(typename)(typename)(typename))
#endif // header
