// fms_statistics.t.cpp - test fms_statistics.h
#include <cassert>
#include "fms_statistics.h"

using namespace fms;


int main()
{
	iterable::interval_test();
	iterable::array_test();

	//iterable::zip_test();
	
	return 0;
}