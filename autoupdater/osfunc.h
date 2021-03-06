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

#pragma once

#if !defined(_OSFUNC_H)
#define _OSFUNC_H

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

std::string oscurrentprocessid();
void oskillprocess(const char* pid);
char* osgetcwd(char* dest, int32_t size);
bool oschdir(const char* path);
bool osmkdir(const char* path);
bool osrmdir(const char* path);
int32_t oscountfiles(const char* path);
bool osrmfile(const char* path);
uint64_t osgettimems();
void osgosleep(uint32_t ms);
void oslaunchprogram(char** argv);

#endif