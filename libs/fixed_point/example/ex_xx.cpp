// Copyright (C) 2012 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <boost/fixed_point/number.hpp>
#include <boost/detail/lightweight_test.hpp>

using namespace boost::fixed_point;

struct pixel
{
  ureal_t<8, 0> r, g, b, a;
};

pixel blend(pixel a, pixel b)
{
  BOOST_AUTO(scale, (to_ureal_t<255, 0> ()));
  BOOST_AUTO(a_r, a.r / scale);
  BOOST_AUTO(b_r, b.r / scale);
  (void) a_r;
  (void) b_r;
  BOOST_AUTO(aia, b.a * (to_ureal_t<1, 0> () - a.a));
  BOOST_AUTO(c_a, a.a + aia);
  BOOST_AUTO(c_r, (a.r * a.a + b.r * aia) / c_a);
  pixel c;
  c.a = number_cast<ureal_t<8, 0> > (c_a * to_ureal_t<255, 0> ());
  c.r = number_cast<ureal_t<8, 0> > (c_r * to_ureal_t<255, 0> ());
  return c;
}

int main()
{
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -2, round::negative> n;
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2, round::negative> n;
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2, round::truncated> n;
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2, round::positive> n;
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -2, round::negative> n( (index(1)));
    BOOST_TEST(n.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -2, round::negative> n = to_ureal_t<1, 0> (); //
    std::cout << int(n.count()) << std::endl;
    BOOST_TEST(n.count() == 4);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2, round::negative> n( (index(1)));
    BOOST_TEST(n.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2, round::truncated> n( (index(1)));
    BOOST_TEST(n.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2, round::positive> n( (index(1)));
    BOOST_TEST(n.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<1, -32> n1( (index(1U)));
    ureal_t<64, 31, round::negative> n2(n1);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 0);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<8, 0> n1( (index(255)));
    ureal_t<8, 0, round::negative> n2(n1);
    BOOST_TEST(n2.count() == 255);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<9, -1> n1( (index(-254)));
    real_t<7, 0, round::negative> n2(n1);
    BOOST_TEST(n2.count() == -127);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -2> n1( (index(1)));
    ureal_t<2, -2, round::negative> n2(n1);
    BOOST_TEST(n1.count() == 1);
    BOOST_TEST(n2.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(1)));
    real_t<2, -2, round::negative> n2(n1);
    BOOST_TEST(n1.count() == 1);
    BOOST_TEST(n2.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(1)));
    real_t<2, -2, round::negative> n2;
    n2 = n1;
    BOOST_TEST(n1.count() == 1);
    BOOST_TEST(n2.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(1)));
    real_t<2, -3, round::negative> n2(n1);
    BOOST_TEST(n1.count() == 1);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 2);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(1)));
    real_t<2, -3, round::negative> n2;
    n2 = n1;
    BOOST_TEST(n1.count() == 1);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 2);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -2> n1( (index(1)));
    ureal_t<2, -3, round::negative> n2(n1);
    BOOST_TEST(n1.count() == 1);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 2);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -3> n1( (index(1)));
    ureal_t<2, -2, round::negative> n2(n1);
    BOOST_TEST(n1.count() == 1);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 0);
  }
  //  {
  //    ureal_t<2,-3> n1((index(1)));
  //    ureal_t<2,-2> n2;
  //    n2=n1; // compile must fail as conversion required
  //  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -3> n1( (index(1)));
    ureal_t<2, -2, round::negative> n2;
    n2 = number_cast<ureal_t<2, -2, round::negative> > (n1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -3> n1( (index(1)));
    real_t<2, -2, round::negative> n2(n1);
    BOOST_TEST(n1.count() == 1);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 0);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -3> n1( (index(1)));
    real_t<2, -2, round::positive> n2(n1);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -3> n1( (index(1)));
    real_t<2, -2, round::truncated> n2(n1);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 0);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -3> n1( (index(0)));
    real_t<2, -2, round::negative> n2(n1);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 0);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -3> n1( (index(0)));
    ureal_t<2, -2, round::negative> n2(n1);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 0);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -3> n1( (index(2)));
    real_t<2, -2, round::negative> n2(n1);
    BOOST_TEST(n1.count() == 2);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -3> n1( (index(2)));
    real_t<2, -2, round::positive> n2(n1);
    BOOST_TEST(n1.count() == 2);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -3> n1( (index(2)));
    real_t<2, -2, round::truncated> n2(n1);
    BOOST_TEST(n1.count() == 2);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -3> n1( (index(-1)));
    real_t<2, -2, round::negative> n2(n1);
    BOOST_TEST(n1.count() == -1);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == -1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -3> n1( (index(-1)));
    real_t<2, -2, round::positive> n2(n1);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 0);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -3> n1( (index(-1)));
    real_t<2, -2, round::truncated> n2(n1);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == 0);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -3> n1( (index(-2)));
    real_t<2, -2, round::negative> n2(n1);
    BOOST_TEST(n1.count() == -2);
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n2.count() == -1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -3> n1( (index(-2)));
    real_t<2, -2, round::positive> n2(n1);
    BOOST_TEST(n1.count() == -2);
    BOOST_TEST(n2.count() == -1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -3> n1( (index(-2)));
    real_t<2, -2, round::truncated> n2(n1);
    BOOST_TEST(n1.count() == -2);
    BOOST_TEST(n2.count() == -1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(-7)));
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -1> n1( (index(7)));
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(15)));
    try
    {
      real_t<2, -1> n2(n1);
      BOOST_TEST(false);
    }
    catch (positive_overflow &)
    {
    }
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -2> n1( (index(15)));
    try
    {
      ureal_t<2, -1> n2(n1);
      BOOST_TEST(false);
    }
    catch (positive_overflow &)
    {
    }
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(-15)));
    try
    {
      real_t<2, -1> n2(n1);
      BOOST_TEST(false);
    }
    catch (negative_overflow &)
    {
    }
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<3, -1> n1( (index(15)));
    try
    {
      real_t<2, -1> n2(n1);
      BOOST_TEST(false);
    }
    catch (positive_overflow &)
    {
    }
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<3, -1> n1( (index(-15)));
    try
    {
      real_t<2, -1> n2(n1);
      BOOST_TEST(false);
    }
    catch (negative_overflow &)
    {
    }
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<3, -2> n1( (index(31)));
    try
    {
      real_t<2, -1> n2(n1);
      BOOST_TEST(false);
    }
    catch (positive_overflow &)
    {
    }
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<3, -2> n1( (index(-31)));
    try
    {
      real_t<2, -1> n2(n1);
      BOOST_TEST(false);
    }
    catch (negative_overflow &)
    {
    }
  }
  /////////////////////////////////////////////////////////////////
  // C(int)
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<4, -1> n1(1);
    BOOST_TEST(n1.count() == 2);
    BOOST_TEST(n1.as_int() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<4, -1> n1(-1);
    BOOST_TEST(n1.count() == -2);
    BOOST_TEST(n1.as_int() == -1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<4, -1> n1(1U);
    BOOST_TEST(n1.count() == 2);
    BOOST_TEST(n1.as_unsigned_int() == 1U);
  }
  //  {
  //    std::cout << __FILE__ << "[" <<__LINE__<<"]"<<std::endl;
  //    ureal_t<4,-1> n1(-1); // compile must fail as ambiguous
  //  }
  /////////////////////////////////////////////////////////////////
  // C(float)
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<4, -1> n1(0.5f);
    BOOST_TEST(n1.count() == 1);
    BOOST_TEST(n1.as_float() == 0.5f);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<4, -1> n1(-0.5f);
    BOOST_TEST(n1.count() == -1);
    BOOST_TEST(n1.as_float() == -0.5f);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<4, -1> n1(0.5f);
    BOOST_TEST(n1.count() == 1);
    BOOST_TEST(n1.as_float() == 0.5f);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<4, -1, round::negative, ::overflow::saturate> n1(-0.5f);
    BOOST_TEST(n1.count() == 0);
    BOOST_TEST(n1.as_float() == 0.0f);
  }
  /////////////////////////////////////////////////////////////////
  // C(double)
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<4, -1> n1(0.5);
    BOOST_TEST(n1.count() == 1);
    BOOST_TEST(n1.as_double() == 0.5);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<4, -1> n1(-0.5);
    BOOST_TEST(n1.count() == -1);
    BOOST_TEST(n1.as_double() == -0.5);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<4, -1> n1(0.5);
    BOOST_TEST(n1.count() == 1);
    BOOST_TEST(n1.as_double() == 0.5);
  }
  /////////////////////////////////////////////////////////////////
  // C(long double)
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<4, -1> n1(0.5l);
    BOOST_TEST(n1.count() == 1);
    BOOST_TEST(n1.as_long_double() == 0.5l);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<4, -1> n1(-0.5l);
    BOOST_TEST(n1.count() == -1);
    BOOST_TEST(n1.as_long_double() == -0.5l);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<4, -1> n1(0.5l);
    BOOST_TEST(n1.count() == 1);
    BOOST_TEST(n1.as_long_double() == 0.5l);
  }

  /////////////////////////////////////////////////////////////////
  // unary plus
  {
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2(+n1);
    BOOST_TEST(n2.count() == 7);
  }
  {
    ureal_t<2, -1> n1( (index(3)));
    ureal_t<2, -1> n2(+n1);
    BOOST_TEST(n2.count() == 3);
  }
  /////////////////////////////////////////////////////////////////
  // unary minus
  {
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2(-n1);
    BOOST_TEST(n2.count() == -7);
    real_t<2, -1> n3(-n2);
    BOOST_TEST(n3.count() == 7);
  }
  {
    ureal_t<2, -1> n1( (index(3)));
    real_t<2, -1> n2(-n1);
    BOOST_TEST(n2.count() == -3);
    real_t<2, -1> n3(-n2);
    BOOST_TEST(n3.count() == 3);
  }
  /////////////////////////////////////////////////////////////////
  // plus
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(7)));
    real_t<3, -1> n3 = n1 + n2;
    BOOST_TEST(n3.count() == 14);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -2> n1( (index(7)));
    ureal_t<2, -2> n2( (index(7)));
    real_t<3, -2> n3 = n1 + n2;
    BOOST_TEST(n3.count() == 14);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -2> n1( (index(7)));
    real_t<2, -2> n2( (index(7)));
    real_t<3, -2> n3 = n2 + n1;
    BOOST_TEST(n3.count() == 14);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(7)));
    BOOST_AUTO(n3, n1 + n2);
    BOOST_TEST(n3.count() == 14);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(-7)));
    real_t<2, -1> n2( (index(-7)));
    real_t<3, -1> n3 = n1 + n2;
    BOOST_TEST(n3.count() == -14);
  }
  /////////////////////////////////////////////////////////////////
  // +=
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(3)));
    real_t<2, -1> n2( (index(3)));
    n1 += n2;
    BOOST_TEST(n1.count() == 6);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<4, -1> n1( (index(3)));
    BOOST_TEST(n1.count() == 3);
    n1 += convert(1.0);
    BOOST_TEST(n1.count() == 5);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<4, -1> n1( (index(3)));
    n1 += convert(1);
    BOOST_TEST(n1.count() == 5);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<4, -1> n1( (index(3)));
    n1 += convert(1u);
    BOOST_TEST(n1.count() == 5);
  }
  /////////////////////////////////////////////////////////////////
  // ++()
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<4, -1> n1( (index(3)));
    ++n1;
    BOOST_TEST(n1.count() == 5);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<4, -1> n1( (index(3)));
    ++n1;
    BOOST_TEST(n1.count() == 5);
  }
  /////////////////////////////////////////////////////////////////
  // ++(int)
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<4, -1> n1( (index(3)));
    real_t<4, -1> n2 = n1++;
    BOOST_TEST(n1.count() == 5);
    BOOST_TEST(n2.count() == 3);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<4, -1> n1( (index(3)));
    ureal_t<4, -1> n2 = n1++;
    BOOST_TEST(n1.count() == 5);
    BOOST_TEST(n2.count() == 3);
  }
  /////////////////////////////////////////////////////////////////
  // --()
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<4, -1> n1( (index(3)));
    --n1;
    BOOST_TEST(n1.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<4, -1> n1( (index(3)));
    --n1;
    BOOST_TEST(n1.count() == 1);
  }
  /////////////////////////////////////////////////////////////////
  // minus
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(7)));
    real_t<3, -1> n3 = n1 - n2;
    BOOST_TEST(n3.count() == 0);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -2> n1( (index(7)));
    ureal_t<2, -2> n2( (index(7)));
    real_t<3, -2> n3 = n1 - n2;
    BOOST_TEST(n3.count() == 0);
  }
  /////////////////////////////////////////////////////////////////
  // -=
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(7)));
    n1 -= n2;
    std::cout << int(n1.count()) << std::endl;
    BOOST_TEST(n1.count() == 0);
  }
  /////////////////////////////////////////////////////////////////
  // multiply
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(7)));
    real_t<4, -2> n3 = n1 * n2;
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == 49);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -2> n1( (index(7)));
    ureal_t<2, -2> n2( (index(7)));
    ureal_t<4, -4> n3 = n1 * n2;
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == 49);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(7)));
    ureal_t<2, -2> n2( (index(7)));
    real_t<4, -4> n3 = n1 * n2;
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == 49);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<2, -2> n1( (index(7)));
    real_t<2, -2> n2( (index(7)));
    real_t<4, -4> n3 = n1 * n2;
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == 49);
  }

  /////////////////////////////////////////////////////////////////
  // virtual_scale
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<6, -2> n1( (index(7)));
    real_t<8, 0> n2 = n1.virtual_scale<2> ();
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n1.count() == 7);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<6, -2> n1( (index(7)));
    ureal_t<8, 0> n2 = n1.virtual_scale<2> ();
    std::cout << int(n2.count()) << std::endl;
    BOOST_TEST(n1.count() == 7);
  }
  /////////////////////////////////////////////////////////////////
  // scale_up
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<6, -2> n1( (index(7)));
    n1.scale_up<2> ();
    std::cout << int(n1.count()) << std::endl;
    BOOST_TEST(n1.count() == 28);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<6, -2> n1( (index(7)));
    n1.scale_up<2> ();
    std::cout << int(n1.count()) << std::endl;
    BOOST_TEST(n1.count() == 28);
  }
  /////////////////////////////////////////////////////////////////
  // scale
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<6, -2> n1( (index(7)));
    n1.scale<2, round::truncated> ();
    std::cout << int(n1.count()) << std::endl;
    BOOST_TEST(n1.count() == 28);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<6, -2> n1( (index(4)));
    n1.scale<-2, round::truncated> ();
    std::cout << int(n1.count()) << std::endl;
    BOOST_TEST(n1.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<6, -2> n1( (index(7)));
    n1.scale<2, round::truncated> ();
    std::cout << int(n1.count()) << std::endl;
    BOOST_TEST(n1.count() == 28);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<6, -2> n1( (index(4U)));
    n1.scale<-2, round::truncated> ();
    std::cout << int(n1.count()) << std::endl;
    BOOST_TEST(n1.count() == 1);
  }
  /////////////////////////////////////////////////////////////////
  // *=
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<6, -1, round::truncated> n1( (index(7)));
    real_t<6, -1, round::truncated> n2( (index(3)));
    n1 *= n2;
    std::cout << int(n1.count()) << std::endl;
    BOOST_TEST(n1.count() == 10); // The exact result 21/4 rounds to 10/2.
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<6, -1, round::truncated> n1( (index(7)));
    ureal_t<6, -1, round::truncated> n2( (index(3)));
    n1 *= n2;
    std::cout << int(n1.count()) << std::endl;
    BOOST_TEST(n1.count() == 10); // The exact result 21/4 rounds to 10/2.
  }
  //  {
  //    std::cout << __FILE__ << "[" <<__LINE__<<"]"<<std::endl;
  //    ureal_t<6,-1, round::truncated> n1((index(7)));
  //    real_t<6,-1, round::truncated> n2((index(3)));
  //    n1*=n2; // compile fails
  //    std::cout << int(n1.count()) << std::endl;
  //    BOOST_TEST(n1.count()==10); // The exact result 21/4 rounds to 10/2.
  //  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    ureal_t<6, -1, round::truncated> n1( (index(7)));
    real_t<6, -1, round::truncated> n2( (index(3)));
    n1 *= number_cast<ureal_t<6, -1, round::truncated> > (n2); // force cast
    std::cout << int(n1.count()) << std::endl;
    BOOST_TEST(n1.count() == 10); // The exact result 21/4 rounds to 10/2.
  }
  /////////////////////////////////////////////////////////////////
  // /=
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<3, -2, round::truncated> n1( (index(1)));
    real_t<3, -2, round::truncated> n2( (index(7)));
    n1 /= n2;
    BOOST_TEST(n1.count() == 0);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<3, -2, round::truncated> n1( (index(7)));
    real_t<3, -2, round::truncated> n2( (index(3)));
    n1 /= n2;
    std::cout << int(n1.count()) << std::endl;
    BOOST_TEST(n1.count() == 9); // 7*4/3
  }
  // divide
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(1)));
    real_t<2, -1> n2( (index(7)));
    real_t<3, -2> n3 = divide<real_t<3, -2, round::truncated> > (n1, n2);
    BOOST_TEST(n3.count() == 0);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(1)));
    real_t<2, -1> n2( (index(7)));
    real_t<3, -2> n3 = divide<real_t<3, -2, round::negative> > (n1, n2);
    BOOST_TEST(n3.count() == 0);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(1)));
    real_t<2, -1> n2( (index(7)));
    real_t<3, -2> n3 = divide<real_t<3, -2, round::positive> > (n1, n2);
    BOOST_TEST(n3.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(15)));
    real_t<2, -2> n2( (index(1)));
    real_t<4, -1> n3 = divide<real_t<4, -1, round::truncated> > (n1, n2);
    BOOST_TEST(n3.count() == 30);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(15)));
    real_t<2, -2> n2( (index(1)));
    real_t<4, -1> n3 = divide<real_t<4, -1, round::negative> > (n1, n2);
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == 30);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(15)));
    real_t<2, -2> n2( (index(1)));
    real_t<4, -1> n3 = divide<real_t<4, -1, round::positive> > (n1, n2);
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == 30);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(-15)));
    real_t<2, -2> n2( (index(1)));
    real_t<4, -1> n3 = divide<real_t<4, -1, round::negative> > (n1, n2);
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == -30);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(-15)));
    real_t<2, -2> n2( (index(1)));
    real_t<4, -1> n3 = divide<real_t<4, -1, round::positive> > (n1, n2);
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == -30);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(15)));
    real_t<2, -2> n2( (index(7)));
    real_t<4, -1> n3 = divide<real_t<4, -1, round::negative> > (n1, n2);
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == 4);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(15)));
    real_t<2, -2> n2( (index(7)));
    real_t<4, -1> n3 = divide<real_t<4, -1, round::positive> > (n1, n2);
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == 5);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(-15)));
    real_t<2, -2> n2( (index(7)));
    real_t<4, -1> n3 = divide<real_t<4, -1, round::negative> > (n1, n2);
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == -5);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(-15)));
    real_t<2, -2> n2( (index(7)));
    real_t<4, -1> n3 = divide<real_t<4, -1, round::positive> > (n1, n2);
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == -4);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(15)));
    real_t<2, -1> n2( (index(1)));
    real_t<4, -1> n3 = divide<real_t<4, -1, round::truncated> > (n1, n2);
    BOOST_TEST(n3.count() == 15);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(15)));
    real_t<2, -1> n2( (index(1)));
    real_t<4, 0> n3 = divide<real_t<4, 0, round::truncated> > (n1, n2);
    BOOST_TEST(n3.count() == 7);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(15)));
    real_t<2, -1> n2( (index(1)));
    real_t<4, 1> n3 = divide<real_t<4, 1, round::truncated> > (n1, n2);
    BOOST_TEST(n3.count() == 3);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -2> n1( (index(15)));
    real_t<2, -1> n2( (index(1)));
    real_t<4, 2> n3 = divide<real_t<4, 2, round::truncated> > (n1, n2);
    BOOST_TEST(n3.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(1)));
    real_t<2, -1> n2( (index(7)));
    real_t<3, -6> n3 = divide<real_t<3, -6, round::truncated> > (n1, n2);
    BOOST_TEST(n3.count() == 9);
  }
  {
    ureal_t<2, -1> n1( (index(1)));
    ureal_t<2, -2> n2( (index(7)));
    ureal_t<4, -6> n3 = divide<ureal_t<4, -6, round::truncated> > (n1, n2);
    BOOST_TEST(n3.count() == 18);
  }
  {
    real_t<2, -1> n1( (index(1)));
    ureal_t<2, -2> n2( (index(7)));
    real_t<4, -6> n3 = divide<real_t<4, -6, round::truncated> > (n1, n2);
    BOOST_TEST(n3.count() == 18);
  }
  {
    ureal_t<2, -1> n1( (index(1)));
    real_t<2, -2> n2( (index(7)));
    real_t<4, -6> n3 = divide<real_t<4, -6, round::truncated> > (n1, n2);
    BOOST_TEST(n3.count() == 18);
  }
  {
    ureal_t<2, -1> n1( (index(1)));
    real_t<2, -2> n2( (index(7)));
    real_t<4, -3> n3 = n1 / n2;
    BOOST_TEST(n3.count() == 2);
  }
  {
    real_t<2, -1> n1( (index(1)));
    real_t<2, -2> n2( (index(7)));
    real_t<6, -3> n3 = divide<real_t<6, -3, round::truncated> > (n1, n2);
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == 2);
  }
  {
    real_t<2, -1> n1( (index(-1)));
    real_t<2, -1> n2( (index(7)));
    real_t<3, -6> n3 = divide<real_t<3, -6, round::truncated> > (n1, n2);
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == -9);
  }
  {
    real_t<2, -1> n1( (index(1)));
    real_t<2, -1> n2( (index(7)));
    real_t<3, -3, round::negative> n3 = n1 / n2;
    BOOST_TEST(n3.count() == 1);
  }
  {
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(1)));
    real_t<3, -6> n3 = divide<real_t<3, -6, round::truncated> > (n1, n2);
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == 7 * 64);
  }
  {
    real_t<4, 1> n1( (index(1)));
    real_t<4, 1> n2( (index(7)));
    real_t<3, -6> n3 = divide<real_t<3, -6, round::truncated> > (n1, n2);
    BOOST_TEST(n3.count() == 9);
  }
  {
    real_t<4, 1> n1( (index(1)));
    real_t<4, 1> n2( (index(7)));
    real_t<3, -3> n3 = divide<real_t<3, -3, round::truncated> > (n1, n2);
    BOOST_TEST(n3.count() == 1);
  }
  {
    real_t<4, 1> n1( (index(7)));
    real_t<4, 1> n2( (index(1)));
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    std::cout << int(n2.count()) << std::endl;
    real_t<3, -6> n3 = divide<real_t<3, -6, round::truncated> > (n1, n2);
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    std::cout << int(n3.count()) << std::endl;
    BOOST_TEST(n3.count() == 7 * 64);
  }
  /////////////////////////////////////////////////////////////////
  // equal
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(7)));
    BOOST_STATIC_CONSTEXPR bool b = n1 == n2;
    BOOST_TEST(b);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -2> n2( (index(14)));
    BOOST_TEST(n1 == n2);
  }
  /////////////////////////////////////////////////////////////////
  // not_equal
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(1)));
    BOOST_TEST(n1 != n2);
  }
  /////////////////////////////////////////////////////////////////
  // gt
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(1)));
    BOOST_TEST(n1 > n2);
  }
  /////////////////////////////////////////////////////////////////
  // ge
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(1)));
    BOOST_TEST(n1 >= n2);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(7)));
    BOOST_TEST(n1 >= n2);
  }
  /////////////////////////////////////////////////////////////////
  // lt
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(1)));
    BOOST_TEST(n2 < n1);
  }
  // le
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(1)));
    BOOST_TEST(n2 <= n1);
  }
  //  {
  //    ureal_t<2,-1> n1((index(-7))); // assertion failed
  //  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<1, -2> n2( (index(1)));
    BOOST_TEST(n2 <= n1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    real_t<2, -1> n1( (index(7)));
    real_t<2, -1> n2( (index(7)));
    BOOST_TEST(n2 <= n1);
  }
  //  {
  //    real_t<32,-32> n; // compile fail
  //    std::cout  << sizeof(real_t<32,-32>::underlying_type) << std::endl;
  //    std::cout  << ::std::numeric_limits<long>::digits << std::endl;
  //    std::cout  << (int)(sizeof(boost::long_long_type) * CHAR_BIT) << std::endl;
  //  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    BOOST_AUTO(scale, (to_ureal_t<255, 0> ()));
    BOOST_TEST(scale.count() == 255);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    BOOST_AUTO(scale, (to_real_t<255, 0> ()));
    BOOST_TEST(scale.count() == 255);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    BOOST_AUTO(scale, (to_ureal_t<1, -12> ()));
    BOOST_TEST(scale.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    BOOST_AUTO(scale, (to_real_t<1, -12> ()));
    BOOST_TEST(scale.count() == 1);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    BOOST_AUTO(scale, (to_ureal_t<3, 10> ()));
    BOOST_TEST(scale.count() == 3);
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    typedef ureal_t<8, 0> T;
    std::cout << T::min_index << std::endl;
    std::cout << T::max_index << std::endl;
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    typedef real_t<8, 0> T;
    std::cout << T::min_index << std::endl;
    std::cout << T::max_index << std::endl;
    std::cout << sizeof(long int) << std::endl;
  }

  /////////////////////////////////////////////////////////////////
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    typedef real_t<15, -16> fp_15__16; // Signed fixed-point values with 15 bits of integer part
    // and 16 bits of fractional part.
    fp_15__16 f1, f2;
    f1 = convert(1.2345); // Conversion from floating point.
    f2 = convert(f1 + 2); // Mixed arithmetic with integers.
    f2 = convert(f1 / f2); // Arithmetic on fixed-point values.
    //std::cout<< float(f1) << std::endl;
  }
  {
    std::cout << __FILE__ << "[" << __LINE__ << "]" << std::endl;
    typedef ureal_t<15, -16> ufp_15__16; // Signed fixed-point values with 15 bits of integer part
    // and 16 bits of fractional part.
    ufp_15__16 f1, f2;
    f1 = convert(1.2345); // Conversion from floating point.
    f2 = convert(f1 + 2U); // Mixed arithmetic with integers.
    f2 = convert(f1 / f2); // Arithmetic on fixed-point values.
    //std::cout<< float(f1) << std::endl;
  }


  return boost::report_errors();
}

