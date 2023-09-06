#include <catch.hpp>

#include "util/misc.hpp"

using namespace explo;

TEST_CASE("misc-pmod")
{
	int n = 3;
	int v = 0;

	REQUIRE(pmod(v - 3, n) == 0);
	REQUIRE(pmod(v - 2, n) == 1);
	REQUIRE(pmod(v - 1, n) == 2);
	REQUIRE(pmod(v, n) == 0);
	REQUIRE(pmod(v + 1, n) == 1);
	REQUIRE(pmod(v + 2, n) == 2);
	REQUIRE(pmod(v + 3, n) == 0);
}