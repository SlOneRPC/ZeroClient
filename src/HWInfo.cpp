#include <string>
#include <Windows.h>

int HWID = 0;
//TODO use other identifers other than volume serial number
int getHWinfo64() {
	DWORD VolumeSerialNumber = 0;
	GetVolumeInformation("c:\\", NULL, NULL, &VolumeSerialNumber, NULL, NULL, NULL, NULL);
	HWID = VolumeSerialNumber;
	return HWID;
}