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

#include <algorithm>
#include <stdint.h>
#include <string>
#include "curses.h"
#include "gui.h"
using namespace std;

int32_t ystep = 10;
int32_t maxsize = 10;
int32_t mod = 0;

int32_t getxcenter(string s)
{
	int32_t len = s.length();
	return ((maxsize / 2) - (len / 2) + 2);
}

void guiinit(string progname)
{
	initscr();
	noecho();
	curs_set(0);

	if (has_colors() == TRUE)
	{
		start_color();

		init_pair(1, COLOR_WHITE, COLOR_WHITE);
		init_pair(2, COLOR_CYAN, COLOR_CYAN);
	}

	int32_t maxy = getmaxy(stdscr);
	ystep = (int32_t)floor(maxy / 4);

	int32_t maxx = getmaxx(stdscr);
	maxsize = maxx - 4;
	
	if (maxy >= 20)
	{
		mod = 1;
	}
	
	guichangeprogname(progname);

	string hor = "+";
	string ver = "|";
	for (int32_t i = 0; i < maxsize - 2; i++)
	{
		hor += "-";
		ver += " ";
	}

	hor += "+";
	ver += "|";
	
	mvaddstr(ystep + 1, getxcenter(hor), hor.c_str());
	mvaddstr(ystep + 2, getxcenter(ver), ver.c_str());
	mvaddstr(ystep + 3, getxcenter(ver), ver.c_str());
	mvaddstr(ystep + 4, getxcenter(hor), hor.c_str());
	mvaddstr((ystep * 2) + 1 + mod, getxcenter(hor), hor.c_str());
	mvaddstr((ystep * 2) + 2 + mod, getxcenter(ver), ver.c_str());
	mvaddstr((ystep * 2) + 3 + mod, getxcenter(ver), ver.c_str());
	mvaddstr((ystep * 2) + 4 + mod, getxcenter(hor), hor.c_str());
	refresh();
}

void guichangeprogname(string progname)
{
	guisetlabel(0, progname);
}

void guishutdown()
{
	endwin();
}

void guisetlabel(int32_t index, string text)
{
	int32_t y;

	switch (index)
	{
	case 1:
		y = ystep - mod;
		break;
	case 2:
		y = ystep * 2;
		break;
	case 3:
		y = (ystep * 3) + mod;
		break;
	default:
		if (ystep > 3)
		{
			y = 2;
		}
		else if (ystep == 3)
		{
			y = 1;
		}
		else
		{
			y = 0;
		}
	}
	
	string text1 = text;
	string text2;
	if ((int32_t)text.length() > maxsize)
	{
		if (mod > 0) {
			text1 = text.substr(0, maxsize);
			text2 = text.substr(maxsize);

			if ((int32_t)text2.length() > maxsize)
			{
				text2 = text2.substr(0, maxsize - 3);
				text2 += "...";
			}
		}
		else
		{
			text1 = text.substr(0, maxsize - 3);
			text1 += "...";
		}
	}

	move(y, 0);
	clrtoeol();
	mvaddstr(y, getxcenter(text1), text1.c_str());

	if (mod > 0)
	{
		move(y + 1, 0);
		clrtoeol();
		mvaddstr(y + 1, getxcenter(text2), text2.c_str());
	}

	refresh();
}

void guisetprogressbar(int32_t index, uint32_t value, uint32_t max)
{
	int32_t y;

	if (max == 0) {
		max = 1;
	}

	if (value > max) {
		value = max;
	}

	float progress = (float)value / (float)max;

	if (progress > 1.0f)
	{
		progress = 1.0f;
	}

	int32_t pos = (int32_t)((maxsize - 2) * progress);

	switch (index)
	{
	case 1:
		y = ystep + 2;
		break;
	case 2:
		y = (ystep * 2) + 2 + mod;
		break;
	default:
		y = (ystep * 3) + 2 + (mod * 2);
	}

	string bar;
	string barpos;
	string nobar;
	for (int32_t i = 0; i < maxsize - 2; i++)
	{
		if (i < pos)
		{
			bar += "=";
		}
		else if (i == pos && progress > 0.0f && progress < 1.0f)
		{
			barpos += ">";
		}
		else
		{
			nobar += " ";
		}
	}

	if (has_colors() == TRUE)
	{
		attron(COLOR_PAIR(1));
	}

	mvaddstr(y, 3, bar.c_str());
	mvaddstr(y + 1, 3, bar.c_str());

	if (has_colors() == TRUE)
	{
		attroff(COLOR_PAIR(1));
		attron(COLOR_PAIR(2));
	}

	mvaddstr(y, 3 + bar.length(), barpos.c_str());
	mvaddstr(y + 1, 3 + bar.length(), barpos.c_str());

	if (has_colors() == TRUE)
	{
		attroff(COLOR_PAIR(2));
	}

	mvaddstr(y, 3 + bar.length() + barpos.length(), nobar.c_str());
	mvaddstr(y + 1, 3 + bar.length() + barpos.length(), nobar.c_str());

	refresh();
}