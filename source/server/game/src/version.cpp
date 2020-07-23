#include <stdio.h>
#include <stdlib.h>
#include "../../../common/file_ptr.h"

void WriteVersion()
{
	auto fp = msl::file_ptr("VERSION.txt", "w");

	if (fp)
	{
		fprintf(fp.get(), "game_server perforce revision: %s\n", _GIT_VERSION_);
		fprintf(fp.get(),"Hostname: %s Path: %s\n", _HOSTNAME_, _DIRECTORY_);
		return;
	}
	fprintf(stderr, "cannot open VERSION.txt\n");
	exit(0);
}

