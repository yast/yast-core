#include <string>
#include <iostream>
#include <string.h>

#include <y2util/PathInfo.h>

using namespace std;

int main(void)
{
	string directory="bla/blub/zonk";
	int ret=0;
	if((ret=PathInfo::assert_dir(directory)))
	{
		cerr << strerror(ret) << endl;
		return 1;
	}
	return 0;
}
