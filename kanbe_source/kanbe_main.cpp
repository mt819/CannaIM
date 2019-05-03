/********************************************
	kanbe_main.cpp
*********************************************/

#include <stdlib.h>
#include "kanBeApp.h"


int main()
{
	kanBeApp *kanBeApplication;

	kanBeApplication = new kanBeApp();
	kanBeApplication->Run();
	delete kanBeApplication;

	return EXIT_SUCCESS;
}

