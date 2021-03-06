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

#include "stdafx.h"

#include <conio.h>
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <Windows.h>
#include "osfunc.h"
using namespace std;

extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

string oscurrentprocessid()
{
	DWORD dpid = GetCurrentProcessId();

	char str[100];
	sprintf(str, "%d", dpid);

	string ret = str;

	return ret;
}

void oskillprocess(const char* pid)
{
	HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, TRUE, (DWORD)atoi(pid));

	DWORD res = WaitForSingleObject(h, 5 * 1000);

	if (res == WAIT_TIMEOUT)
	{
		TerminateProcess(h, EXIT_SUCCESS);
	}
}

char* osgetcwd(char* dest, int32_t size)
{
	return _getcwd(dest, size);
}

bool oschdir(const char* path)
{
	int32_t res = _chdir(path);

	return res == 0;
}

bool osmkdir(const char* path)
{
	int32_t res = _mkdir(path);

	return res == 0;
}

bool osrmdir(const char* path)
{
	int32_t res = _rmdir(path);

	return res == 0;
}

int32_t oscountfiles(const char* path)
{
	int32_t count = 0;

	string ftarget = path;
	ftarget.append("/");
	ftarget.append("*");

	intptr_t file;
	_finddata_t filedata;
	file = _findfirst(ftarget.c_str(), &filedata);

	if (file != -1)
	{
		do
		{
			if (strcmp(filedata.name, ".") != 0 && strcmp(filedata.name, "..") != 0)
			{
				count++;
			}
		} while (_findnext(file, &filedata) == 0);
	}

	_findclose(file);

	return count;
}

bool osrmfile(const char* path)
{
	int32_t res = remove(path);

	return res == 0;
}

template <class Dest, class Source>
inline Dest bit_cast(const Source& source) {
	Dest dest;
	memcpy(&dest, &source, sizeof(dest));
	return dest;
}

uint64_t FileTimeToMilliseconds(const FILETIME& ft) {
	return ((bit_cast<uint64_t, FILETIME>(ft) / 10) / 1000) - 11644473600000;
}

uint64_t osgettimems()
{
	SYSTEMTIME st;
	GetSystemTime(&st);

	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);

	return FileTimeToMilliseconds(ft);
}

void osgosleep(uint32_t ms)
{
	Sleep(ms);
}

std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;

	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);

	wchar_t* buf = new wchar_t[len];

	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);

	std::wstring r(buf);

	delete[] buf;

	return r;
}

void oslaunchprogram(char** argv)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	string s;

	for (int i = 0; argv[i] != NULL; i++)
	{
		if (argv[i] != NULL)
		{
			if (s.length() > 0)
			{
				s += " ";
			}
			else if (i == 0)
			{
				s += "\"";
			}

			s += argv[i];

			if (i == 0)
			{
				s += "\"";
			}
		}
	}

	std::wstring stemp = s2ws(s);
	LPWSTR args = (&stemp[0]);

	if (CreateProcess(NULL, args, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_NO_WINDOW | DETACHED_PROCESS, NULL, NULL, &si, &pi) == false)
	{
		printf("*** ERROR: exec failed\n");
		exit(EXIT_FAILURE);
	}
}