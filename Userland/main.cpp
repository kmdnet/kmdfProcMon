#include<Windows.h>

#include "Header.h"

int main(int argc, char *argv[]) {

	ioctl_event_init();
	ioctl_event();
	//ioctl_event_stop();

	return 0;
}