#include "systemclass.h"

int main()
{
	SystemClass sys;
	sys.Init();
	sys.Run();
	sys.Shutdown();

	perror("over");
	return 0;
}