#include <string>
#include <iostream>
#include <y2util/ExternalProgram.h>

using namespace std;

int main(int argc, char* argv[])
{
    const char * chroot = "/";
    if(argc >= 2)
    {
	chroot = argv[1];
    }
    const char* aa[] = { "ls" , "-al", NULL };
//    string aa = "ls";
    ExternalProgram* prog = new ExternalProgram(aa, ExternalProgram::Stderr_To_Stdout,
	false, -1, true, chroot);
    if(!prog) return 1;
    string line;
    
    for(line = prog->receiveLine();
	line.length() > 0;
	line = prog->receiveLine())
    {
	cout << line;
    }
    cout << "program exited with " << prog->close() << endl;
    return prog->close();
}
