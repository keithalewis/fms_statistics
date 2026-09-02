// fms_statistics.t.cpp - test fms_statistics.h
#include <cassert>
#include "fms_statistics.h"

using namespace fms;


int main()
{
	iterable::list_test();
	iterable::interval_test();
	iterable::vector v1({ 1,2 });
	iterable::vector v2	({ 3,4 });

	//iterable::zip_test();
	//iterable::interval<int*>::test();
	
	return 0;
}