#include <string>
#include <Windows.h>

int HWID = 0;

int getHWinfo64() {
	DWORD VolumeSerialNumber = 0;
	GetVolumeInformation("c:\\", NULL, NULL, &VolumeSerialNumber, NULL, NULL, NULL, NULL);
	HWID = VolumeSerialNumber;
	return HWID;
}