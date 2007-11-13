#include <iostream>
#include <y2util/Y2SLog.h>

using namespace std;

int main()
{
  DBG << "test" << endl;
  D__ << "test" << endl;
  DBG << "test" << endl;
#define Y2SLOG "real"
//  Y2SLog::dbg_enabled_bm.setEnabled (true);
  D__ << "test";
  DBG << "test";
  return 0;
}
