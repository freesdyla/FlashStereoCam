#include "FlirBlackFlySCameraControl.h"

int main()
{
	FlirBlackFlySCameraControl stereo;

	stereo.Start();

	getchar();
	stereo.Stop();
	return 0;
}
