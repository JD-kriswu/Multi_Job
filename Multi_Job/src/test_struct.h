#ifndef __TEST_STRUCT__
#define __TEST_STRUCT__


#include <string>

using namespace std;

typedef struct dao_test
{
	int  ruleId;
	string  statType;

	dao_test(){}

	template <class AR>
	
	AR& serialize(AR& ar)
	{
		return ar& ruleId & statType;
	}
}dao_test;

#endif

