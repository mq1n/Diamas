#include <stdio.h>
#include <stdlib.h>

void WriteVersion()
{
	FILE* fp(fopen("VERSION.txt", "w"));

	if (fp)
	{
		fprintf(fp, "game_server perforce revision: %s\n", _GIT_VERSION_);
		fprintf(fp, "Hostname: %s Path: %s\n", _HOSTNAME_, _DIRECTORY_);
		fclose(fp);
	}
	else
	{
		fprintf(stderr, "cannot open VERSION.txt\n");
		exit(0);
	}
}

