#include <string>
#include <iostream>
#include <y2util/ExternalProgram.h>

using namespace std;

int main(int argc, char* argv[])
{
    if(argc<2)
    {
	cout << "must specify directory to chroot to" << endl;
	return 1;
    }
    char* aa[] = { "ls" , "-al", NULL };
//    string aa = "ls";
    ExternalProgram* prog = new ExternalProgram(aa, ExternalProgram::Stderr_To_Stdout,
	false, -1, true, argv[1]);
    if(!prog) return 1;
    string line;
    
    for(line = prog->receiveLine();
	line.length() > 0;
	line = prog->receiveLine())
    {
	cout << line;
    }
    cout << "program exited with " << prog->close() << endl;
    return 0;    
}
