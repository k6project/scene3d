#include "fbx_utils.h"

#include <string.h>

#define AppMain main

#ifndef _MSC_VER
#include <getopt.h>
#else // _MSC_VER
#include "xgetopt.h"
#endif // _MSC_VER

struct CommandLine_
{
	const char* InFile;
};

typedef struct CommandLine_ CommandLine;

void AppParseCommandLine(int argc, char** argv, CommandLine* cmd)
{
	int option = 0;
	while ((option = getopt(argc, argv, "i:o:")) != -1)
	{
		switch (option)
		{
		case 'i':
			cmd->InFile = optarg;
			break;
		default:
			break;
		}
	}
}

void AppMain(int argc, char** argv)
{
	CommandLine cmd;
	memset(&cmd, 0, sizeof(cmd));
	AppParseCommandLine(argc, argv, &cmd);
	if (cmd.InFile)
	{
		FbxLoadScene();
	}
}
