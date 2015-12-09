/*
	c++ autoupdater
	Copyright(C) 2015  gmt2001
	https://github.com/gmt2001

	This program is free software : you can redistribute it and / or modify
	it under the terms of the GNU Affero General Public License as published
	by the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <sys/time.h>
#include <time.h>
#include "osfunc.h"
using namespace std;

const char* oscurrentprocessid()
{
	pid_t dpid = getpid();

	char str[100];
	sprintf(str, "%d", (int32_t)dpid);

	const char* ret = str;

	return ret;
}

void oskillprocess(const char* pid)
{
	int32_t k = kill((pid_t)atoi(pid), 0);

	struct timespec stime;
	stime.tv_sec = 0;
	stime.tv_nsec = 500 * 1000000;

	for (int32_t i = 0; i < 10 && k == 0; i++)
	{
		nanosleep(stime, NULL);

		k = kill((pid_t)atoi(pid), 0);
	}

	if (k == 0)
	{
		kill((pid_t)atoi(pid), SIGKILL);
	}
}

char* osgetcwd(char* dest, int32_t size)
{
	return getcwd(dest, size);
}

bool oschdir(const char* path)
{
	int32_t res = chdir(path);

	return res == 0;
}

bool osmkdir(const char* path)
{
	int32_t res = mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

	return res == 0;
}

bool osrmdir(const char* path)
{
	int32_t res = rmdir(path);

	return res == 0;
}

int32_t oscountfiles(const char* path)
{
	int32_t count = 0;

	string ftarget = path;
	ftarget.append("/");
	ftarget.append("*");

	DIR* file;
	struct dirent* filedata;
	file = opendir(ftarget.c_str());

	if (file)
	{
		while (filedata = readdir(file))
		{
			if (strcmp(filedata->d_name, ".") != 0 && strcmp(filedata->d_name, "..") != 0)
			{
				count++;
			}
		}
	}

	closedir(file);

	return count;
}

bool osrmfile(const char* path)
{
	int32_t res = remove(path);

	return res == 0;
}

uint64_t osgettimems()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return (uint64_t)((uint64_t)tp.tv_sec * 1000 + (uint64_t)((uint64_t)tp.tv_usec / 1000));
}

void osgosleep(uint32_t ms)
{
	struct timespec ts = { 0 };
	ts.tv_sec = 0;
	ts.tv_nsec = ms * 1000000L;
	nanosleep(&ts, (struct timespec *)NULL);
}