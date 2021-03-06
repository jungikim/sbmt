//  Copyright (c) 2006, Giovanni P. Deretta
//
//  This code may be used under either of the following two licences:
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the "Software"), to deal 
//  in the Software without restriction, including without limitation the rights 
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
//  copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in 
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
//  THE SOFTWARE. OF SUCH DAMAGE.
//
//  Or:
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/coro/coro.hpp>
#include <iostream>
#include <boost/test/unit_test.hpp>

namespace coros = boost::coros;
using coros::coro;

 
struct my_result {};
struct my_parm {};

typedef coro<my_result(my_parm)> coro_type;
typedef coro<void()> coro_vv_type;
typedef coro<int()> coro_iv_type;
typedef coro<void(int)> coro_vi_type;

typedef boost::tuple<my_result, my_result> result_tuple;
typedef boost::coros::tuple_traits<my_result, my_result> result_tuple_tag;
typedef coro<result_tuple_tag (my_parm)> coro_tuple_type;

typedef boost::tuple<my_parm, my_parm> parm_tuple;
typedef coro<my_result(my_parm, my_parm)> coro_tuple2_type;

my_result foo(coro_type::self& self, my_parm parm) {
  int i = 10;
  my_result t;
  while(--i) {
    std::cout<<i <<", in coro, yielding\n";
    parm = self.yield(t);
  }
  std::cout<<i <<", in coro, exiting\n";
  return t;
}

struct foo_functor {
  typedef my_result result_type;
  my_result operator()(coro_type::self& self, my_parm parm) {
    return foo(self, parm);
  }
};

foo_functor
make_foo_functor() {
  return foo_functor();
}

int bar(coro_iv_type::self& self) {
  self.yield(0);
  return 0;
}

result_tuple baz(coro_tuple_type::self& self, my_parm) {
  self.yield(my_result(), my_result()); 
  return boost::make_tuple(my_result(), my_result());
}

my_result barf(coro_tuple2_type::self& self, my_parm a, my_parm b) {
  boost::tie(a, b) = self.yield(my_result());
  return my_result();
}

void vi(coro_vi_type::self& self, int i) {
  i = self.yield();
}

void vv(coro_vv_type::self& self) {
  self.yield();
}

typedef coro<void(int&)> coro_ref_type;
void ref(coro_ref_type::self& self, int& x) {
  x = 10;
  self.yield();
}

void test_create() {
  coro <my_result(my_parm)> empty;
  coro <my_result(my_parm)> coro(foo);
  BOOST_CHECK(!empty);
  swap(empty, coro);
  BOOST_CHECK(empty);
  swap(empty, coro);

  coro_type coro_functor(make_foo_functor());
  my_parm t;

  /* Void parameters are supported */
  coro_iv_type iv_coro(bar);
  /* Void results are supported */
 coro_vi_type vi_coro (vi);
  /* Void values and parameters are supported*/  
  coro_vv_type void_coro (vv); 
  /* Tuple result types are supported */
  coro_tuple_type tuple_coro(baz);
  /* Variable arity coros are supported */
  coro_tuple2_type tuple2_coro(barf);
  tuple2_coro(my_parm(), my_parm());
  /* references are supported */
  coro_ref_type ref_coro(ref);
  int x = 0;
  ref_coro(x);
  BOOST_CHECK(x == 10);

  while(coro && coro_functor) {
    coro(t);
    coro_functor(t);
  }
  BOOST_CHECK(!(coro && coro_functor));
}

boost::unit_test::test_suite* init_unit_test_suite( int argc, char* argv[] )
{
    boost::unit_test::test_suite *test = BOOST_TEST_SUITE("create coro test");

    test->add(BOOST_TEST_CASE(&test_create));

    return test;
}
