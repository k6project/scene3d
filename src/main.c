#include "fbx_utils.h"

#include <string.h>

#define AppMain main

#ifndef _MSC_VER
#include <getopt.h>
#else // _MSC_VER
#include "xgetopt.h"
#endif // _MSC_VER

typedef struct CommandLine
{
	const char* InFile;
    const char* OutFile;
} CommandLine;

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
        case 'o':
            cmd->OutFile = optarg;
            break;
        default:
			break;
		}
	}
}

int AppMain(int argc, char** argv)
{
	CommandLine cmd;
	memset(&cmd, 0, sizeof(cmd));
	AppParseCommandLine(argc, argv, &cmd);
	if (cmd.InFile)
	{
		FbxLoadScene(cmd.InFile);
	}
    return 0;
}
