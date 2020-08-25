#include <stdio.h>
#include <systemcall_impl.h>
#include <vhd.h>

struct vhd src;

int main(int argc, char** argv)
{
	if (vhd_open(&src, "win7fixed.vhd",
		OPEN_RAW_OK) == -1) {
		return -1;
	}

	return 0;
}
