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
#include <cctype>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <math.h>
#include <sstream>
#include <stdint.h>
#include <string>
#include <time.h>
#include "curl\curl.h"
#include "gui.h"
#include "md5.h"
#include "osfunc.h"
using namespace std;

deque<string> urls;
deque<deque<string>> files;
int32_t updaterindex = -1;
int32_t configindex = -1;
bool randomUrl = false;
string response = "";
string launch = "";
bool fail = false;
bool updaterchange = false;
bool configchange = false;
string proto = "";
uint32_t dlsize = 0;
uint32_t dlprog = 0;
uint32_t lastdlprog = 0;
uint64_t lastdlupdate = 0;
fstream fdl;
const uint32_t vshortsleeptimems = (uint32_t)(0.2 * 1000);
const uint32_t shortsleeptimems = 1 * 1000;
const uint32_t smedsleeptimems = (uint32_t)(2.5 * 1000);
const uint32_t medsleeptimems = 5 * 1000;
const uint32_t longsleeptimems = 10 * 1000;
string progname = "AutoUpdater v1.0.0";
const char* proguaname = "autoupdater/1.0.0";
map<string, int32_t> okresponses;

void strtoupper(string &str)
{
	transform(str.begin(), str.end(), str.begin(), (int32_t(*) (int32_t))toupper);
}

void strtolower(string &str)
{
	transform(str.begin(), str.end(), str.begin(), (int32_t(*) (int32_t))tolower);
}

void mkdir_recursive(const char* path)
{
	string spath = path;

	replace(spath.begin(), spath.end(), '\\', '/');

	int32_t num = 0;
	int32_t num2 = spath.find('/', num);
	int32_t lastnum = 0;
	bool part = true;
	string s;
	string cpath = "";

	while (part)
	{
		if (num2 == string::npos)
		{
			if (num < (int32_t)spath.length() && lastnum != num)
			{
				s = spath.substr(num);
			}
			else {
				break;
			}

			part = false;
		}
		else {
			lastnum = num;
			s = spath.substr(num, num2 - num);
			num = num2 + 1;
			num2 = spath.find('/', num);
		}

		if (cpath.length() > 0)
		{
			cpath.append("/");
		}

		cpath.append(s);

		osmkdir(cpath.c_str());
	}
}

string md5(string message)
{
	MD5_CTX m;
	unsigned char hash[16];

	MD5_Init(&m);

	for (size_t i = 0; i < (size_t)message.length(); i += 64)
	{
		MD5_Update(&m, (unsigned char*)message.substr(i, min(message.length() - i, 64)).c_str(), min(message.length() - i, 64));
	}

	MD5_Final(hash, &m);

	string output;

	for (int i = 0; i < 16; i++)
	{
		output += static_cast<ostringstream*>(&(ostringstream() << hex << setw(2) << hash[i]))->str();
	}

	strtoupper(output);

	return output;
}

void SetCurrentStep(string newstatus)
{
	guisetlabel(1, newstatus);
}

void SetCurrentItem(string newstatus)
{
	guisetlabel(2, newstatus);
}

void SetDownloadStatus(string newstatus)
{
	guisetlabel(3, newstatus);
}

void SetTotalProgress(uint32_t progress)
{
	guisetprogressbar(1, progress, 100);
}

void UpdateDownloadProgress(bool reset)
{
	if (reset)
	{
		guisetprogressbar(2, 0, 1);
	}

	if (dlsize < 1)
	{
		return;
	}

	uint64_t now = osgettimems();
	uint64_t msecs = now - lastdlupdate;

	if (msecs < 500 && (dlprog < dlsize || dlprog == lastdlprog || msecs < 10))
	{
		return;
	}

	double dlrate = ((dlprog - lastdlprog) / msecs) * 1000.0;
	double dlratek = dlrate / 1024.0;
	double dlratem = dlratek / 1024.0;

	double dlprogk = dlprog / 1024.0;
	double dlprogm = dlprogk / 1024.0;

	double dlsizek = dlsize / 1024.0;
	double dlsizem = dlsizek / 1024.0;

	lastdlprog = dlprog;
	lastdlupdate = osgettimems();

	string s;

	s = "Downloaded ";

	if (dlprogm < 0.8)
	{
		if (dlprogk < 0.8)
		{
			s += static_cast<ostringstream*>(&(ostringstream() << fixed << setprecision(2) << dlprog))->str();
			s += " B";
		}
		else {
			s += static_cast<ostringstream*>(&(ostringstream() << fixed << setprecision(2) << dlprogk))->str();
			s += " kB";
		}
	}
	else {
		s += static_cast<ostringstream*>(&(ostringstream() << fixed << setprecision(2) << dlprogm))->str();
		s += " MB";
	}

	s += " out of ";

	if (dlsizem < 0.8)
	{
		if (dlsizek < 0.8)
		{
			s += static_cast<ostringstream*>(&(ostringstream() << fixed << setprecision(2) << dlsize))->str();
			s += " B";
		}
		else {
			s += static_cast<ostringstream*>(&(ostringstream() << fixed << setprecision(2) << dlsizek))->str();
			s += " kB";
		}
	}
	else {
		s += static_cast<ostringstream*>(&(ostringstream() << fixed << setprecision(2) << dlsizem))->str();
		s += " MB";
	}

	s += " (";

	if (dlratem < 0.8)
	{
		if (dlratek < 0.8)
		{
			s += static_cast<ostringstream*>(&(ostringstream() << fixed << setprecision(2) << dlrate))->str();
			s += " B";
		}
		else {
			s += static_cast<ostringstream*>(&(ostringstream() << fixed << setprecision(2) << dlratek))->str();
			s += " kB";
		}
	}
	else {
		s += static_cast<ostringstream*>(&(ostringstream() << fixed << setprecision(2) << dlratem))->str();
		s += " MB";
	}

	s += "/s)";

	guisetprogressbar(2, dlprog, dlsize);
	SetDownloadStatus(s);
}

// Current config file format is a simple list of urls, one per line with no section headers. C style comments are allowed (but not on the same line as a value)
void LoadConfig()
{
	SetCurrentStep("Opening config file...");

	string fname = "autoupdater.cfg";
	fstream f(fname, fstream::in);

	if (f)
	{
		urls.clear();

		f.seekg(0, f.end);
		int32_t size = (int32_t)f.tellg();
		f.seekg(0, f.beg);

		string s;

		SetCurrentStep("Reading config file...");

		while (!f.eof())
		{
			getline(f, s);

			if (s.length() > 0 && s.substr(0, 1) != " " && s.substr(0, 2) != "//")
			{
				if (s.substr(0, 10) == "randomUrl=")
				{
					if (s.substr(10) == "TRUE")
					{
						randomUrl = true;
					}
					else {
						randomUrl = false;
					}
				}
				else if (s.substr(0, 10) == "launchApp=") {
					launch = s.substr(10);
				}
				else if (s.substr(0, 9) == "progname=") {
					progname = s.substr(9);
				}
				else {
					urls.push_back(s);
				}
			}
		}

		f.close();

		if (urls.size() == 0)
		{
			SetCurrentItem("Error: No urls found in config file");
			fail = true;
		}
	}
	else {
		SetCurrentItem("Error: No config file");
		fail = true;
	}
}

bool isDoubleSpace(char lhs, char rhs)
{
	return (lhs == rhs) && (lhs == ' ');
}

//Web file is in format, with C style comments allowed (but not on the same line as a value)
//op [md5] len:fname len:fpath [len:fpath2]
//op is one of (a)dd or update file, (c)opy file, (m)ove file, (r)emove file
//md5 is the md5 hashsum of the file, not put if op is "r"
//len is the length of a filename or path, used to allow spaces
//fpath/fpath2 is relative to the directory that autoupdater.exe is in (the working directory)
//fpath2 is used as the destination for op "m" and op "c"
void ParseFileListResponse()
{
	SetCurrentStep("Parsing file list...");

	string::iterator itr = remove(response.begin(), response.end(), '\r');
	response.erase(itr, response.end());
	replace(response.begin(), response.end(), '\t', ' ');
	itr = unique(response.begin(), response.end(), isDoubleSpace);
	response.erase(itr, response.end());

	string s;
	deque<string> ds;
	deque<string> ds2;
	int32_t snum = 0;
	int32_t lastsnum = 0;
	int32_t snum2 = response.find('\n', snum);
	int32_t num = 0;
	int32_t num2 = 0;
	int32_t len = 0;
	bool line = true;

	while (line)
	{
		ds.clear();
		num = 0;
		num2 = 0;

		if (snum2 == string::npos)
		{
			if (snum < (int32_t)response.length() && lastsnum != snum)
			{
				s = response.substr(snum);
			}
			else {
				break;
			}

			line = false;
		}
		else {
			lastsnum = snum;
			s = response.substr(snum, snum2 - snum);
			snum = snum2 + 1;
			snum2 = response.find('\n', snum);
		}

		if (s.length() > 0 && s.substr(0, 1) != " " && s.substr(0, 2) != "//")
		{
			num2 = s.find(' ', num);
			ds.push_back(s.substr(num, num2 - num)); //op

			if (s.substr(0, 1) != "r")
			{
				num = num2 + 1;
				num2 = s.find(' ', num);

				ds.push_back(s.substr(2, num2 - 2)); //md5
			}

			num = num2 + 1;
			num2 = s.find(':', num);

			len = atoi(s.substr(num, num2 - num).c_str());//fname len
			ds.push_back(s.substr(num2 + 1, len));//fname

			num = num2 + len + 2;
			num2 = s.find(':', num);

			len = atoi(s.substr(num, num2 - num).c_str());//fpath len
			ds.push_back(s.substr(num2 + 1, len));//fpath

			if (s.substr(0, 1) == "m" || s.substr(0, 1) == "c")
			{
				num = num2 + len + 2;
				num2 = s.find(':', num);

				len = atoi(s.substr(num, num2 - num).c_str());//fpath2 len
				ds.push_back(s.substr(num2 + 1, len));//fpath2
			}

			if (s.substr(0, 1) != "r" && ds[2] == "autoupdater.exe" && (ds[3] == "" || ds[3] == "."))
			{
				updaterindex = files.size();
			}
			else if (s.substr(0, 1) != "r" && ds[2] == "autoupdater.cfg" && (ds[3] == "" || ds[3] == ".")) {
				configindex = files.size();
			}

			files.push_back(ds);

			if (s.substr(0, 1) == "m")
			{
				ds2.clear();

				ds2.push_back("a");
				ds2.push_back(ds[1]);
				ds2.push_back(ds[2]);
				ds2.push_back(ds[4]);

				files.push_back(ds2);
			}
		}
	}
}

void FileCopy(string fpath, string fname, string fpath2, string fname2)
{
	string fromfile;
	string tofile;
	string topath;
	string s;
	
	fromfile = osgetcwd(NULL, 256);

	if (fpath.length() > 0)
	{
		fromfile += "/";
		fromfile += fpath;
	}

	fromfile += "/";
	fromfile += fname;

	tofile = osgetcwd(NULL, 256);
	topath = osgetcwd(NULL, 256);

	if (fpath2.length() > 0)
	{
		tofile += "/";
		tofile += fpath2;
		topath += "/";
		topath += fpath2;
	}

	tofile += "/";
	tofile += fname2;

	mkdir_recursive(topath.c_str());

	fstream f(const_cast<const char*>(fromfile.c_str()), fstream::in | fstream::binary);

	fstream f2(const_cast<const char*>(tofile.c_str()), fstream::out | fstream::trunc | fstream::binary);

	s = "Copying file ";
	s += fname;
	s += "...";
	SetCurrentItem(s);

	f2.seekg(0, f2.beg);

	f.seekg(0, f.end);
	int32_t size = (int32_t)f.tellg();
	f.seekg(0, f.beg);

	dlsize = (uint32_t)size;
	dlprog = 0;
	lastdlupdate = osgettimems();

	UpdateDownloadProgress(true);

	char buff[1024];
	int32_t num = (int32_t)ceil(size / 1024.0);

	for (int32_t i = 0; i < num; i++)
	{
		f.read(buff, 1024);
		f2.write(buff, (i + 1 == num ? size - (i * 1024) : 1024));

		dlprog = (uint32_t)f.tellg();
		UpdateDownloadProgress(false);
	}

	f.close();
	f2.close();
}

size_t handle_header(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	uint32_t numbytes = size * nmemb;
	char lastchar = *((char *)ptr + numbytes - 1);
	string s = "";

	*((char *)ptr + numbytes - 1) = '\0';
	s.append((char *)ptr);
	s.append(1, lastchar);
	*((char *)ptr + numbytes - 1) = lastchar;

	strtoupper(s);

	if (proto == "http" || proto == "https")
	{
		if (s.substr(0, 15) == "CONTENT-LENGTH:")
		{
			string::iterator itr = remove(s.begin(), s.end(), ' ');
			s.erase(itr, s.end());

			dlsize = atoi(s.substr(15).c_str());
			dlprog = 0;
			lastdlupdate = osgettimems();

			UpdateDownloadProgress(true);
		}
	}
	else if (proto == "ftp") {

		if (s.substr(0, 3) == "213")
		{
			string::iterator itr = remove(s.begin(), s.end(), ' ');
			s.erase(itr, s.end());

			dlsize = atoi(s.substr(3).c_str());
			dlprog = 0;
			lastdlupdate = osgettimems();

			UpdateDownloadProgress(true);
		}
	}

	return size * nmemb;
}

size_t handle_file_list_data(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	uint32_t numbytes = size * nmemb;
	char lastchar = *((char *)ptr + numbytes - 1);

	*((char *)ptr + numbytes - 1) = '\0';
	response.append((char *)ptr);
	response.append(1, lastchar);
	*((char *)ptr + numbytes - 1) = lastchar;

	dlprog += numbytes;
	UpdateDownloadProgress(false);

	return size * nmemb;
}

void ReadFileList()
{
	deque<string> tmp(urls);

	int32_t num = 0;
	bool good = false;
	int32_t trynum = 1;
	int32_t numurls = (int32_t)tmp.size();

	string url = "";
	response = "";

	CURL* curl;
	CURLcode res;
	int32_t srvres = 0;
	curl = curl_easy_init();

	string s;

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_USERAGENT, proguaname);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_file_list_data);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, handle_header);
		curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS | CURLPROTO_FTP);

		while (!good && tmp.size() > 0)
		{
			response = "";

			if (randomUrl)
			{
				srand((uint32_t)time(NULL));

				num = rand() % tmp.size();
			}

			s = "Retrieving file list from the web, attempt #";
			s += static_cast<ostringstream*>(&(ostringstream() << trynum++))->str();
			s += "/";
			s += static_cast<ostringstream*>(&(ostringstream() << numurls))->str();
			s += "...";
			SetCurrentStep(s);

			url = tmp[num];
			tmp.erase(tmp.begin() + num);

			proto = url.substr(0, url.find(":"));
			strtolower(proto);

			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

			res = curl_easy_perform(curl);
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &srvres);

			if (res == CURLE_OK && srvres == okresponses[proto])
			{
				good = true;
				ParseFileListResponse();
			}
			else if (tmp.size() == 0) {
				s = "Error: Failed to retrieve file list from the web, error code #";
				s += static_cast<ostringstream*>(&(ostringstream() << (int32_t)res))->str();
				s += ".";
				s += static_cast<ostringstream*>(&(ostringstream() << srvres))->str();
				SetDownloadStatus(s);
				fail = true;
			}
		}

		curl_easy_cleanup(curl);
	}
	else {
		SetDownloadStatus("Error: Failed to open network connection");
		fail = true;
	}
}

size_t handle_file_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	uint32_t numbytes = size * nmemb;

	fdl.write((char *)buffer, numbytes);

	dlprog += numbytes;
	UpdateDownloadProgress(false);

	return size * nmemb;
}

void DownloadFile(string fpath, string rfname, string fname)
{
	deque<string> tmp(urls);

	int32_t num = 0;
	bool good = false;
	int32_t trynum = 1;
	int32_t numurls = (int32_t)tmp.size();

	string url = "";
	response = "";

	CURL* curl;
	CURLcode res;
	int32_t srvres = 0;
	curl = curl_easy_init();

	string s;
	string tempfilename;
	string filepath;

	tempfilename = osgetcwd(NULL, 256);
	filepath = osgetcwd(NULL, 256);

	if (fpath.length() > 0)
	{
		tempfilename += "/";
		tempfilename += fpath;
		filepath += "/";
		filepath += fpath;
	}

	tempfilename += "/";
	tempfilename += fname;
	tempfilename += ".au.tmp";

	mkdir_recursive(filepath.c_str());

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_USERAGENT, proguaname);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_file_data);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, handle_header);
		curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS | CURLPROTO_FTP);

		while (!good && tmp.size() > 0)
		{
			fdl.open(const_cast<const char*>(tempfilename.c_str()), fstream::out | fstream::trunc | fstream::binary);

			fdl.seekg(0, fdl.beg);

			if (randomUrl)
			{
				srand((uint32_t)time(NULL));

				num = rand() % tmp.size();
			}

			s = "Retrieving file ";
			s += rfname;
			s += " from the web, attempt #";
			s += static_cast<ostringstream*>(&(ostringstream() << trynum++))->str();
			s += "/";
			s += static_cast<ostringstream*>(&(ostringstream() << numurls))->str();
			s += "...";
			SetCurrentItem(s);

			url = tmp[num];
			tmp.erase(tmp.begin() + num);

			int32_t pos = url.find_last_of("/");
			url = url.substr(0, pos).append("/");

			if (fpath.length() > 0)
			{
				url = url.append(fpath).append("/");
			}

			url = url.append(rfname);

			proto = url.substr(0, url.find(":"));
			strtolower(proto);

			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

			res = curl_easy_perform(curl);
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &srvres);

			if (res == CURLE_OK && srvres == okresponses[proto])
			{
				good = true;
			}
			else if (tmp.size() == 0) {
				s = "Error: Failed to retrieve file ";
				s += rfname;
				s += " from the web, error code #";
				s += static_cast<ostringstream*>(&(ostringstream() << (int32_t)res))->str();
				s += ".";
				s += static_cast<ostringstream*>(&(ostringstream() << srvres))->str();
				SetDownloadStatus(s);
				fail = true;
			}

			fdl.close();

			if (good)
			{
				FileCopy(fpath, fname.append(".au.tmp"), fpath, fname);
			}

			osrmfile(tempfilename.c_str());
		}

		curl_easy_cleanup(curl);
	}
	else {
		SetDownloadStatus("Error: Failed to open network connection");
		fail = true;
	}
}

string GetFileHash(string fpath, string fname)
{
	string filename;
	string output;

	filename = osgetcwd(NULL, 256);

	if (fpath.length() > 0)
	{
		filename += "/";
		filename += fpath;
	}

	filename += "/";
	filename += fname;

	fstream f(filename.c_str(), fstream::in | fstream::binary);

	if (f) {
		f.seekg(0, f.end);
		size_t fsize = (size_t)f.tellg();
		f.seekg(0, f.beg);

		char buff[64];
		unsigned char hash[16];
		MD5_CTX m;

		MD5_Init(&m);

		for (size_t i = 0; i < fsize; i += 64)
		{
			f.read(buff, 64);
			MD5_Update(&m, buff, (uint32_t)min(f.gcount(), 64));
		}

		MD5_Final(hash, &m);

		for (int i = 0; i < 16; i++)
		{
			output += static_cast<ostringstream*>(&(ostringstream() << hex << setw(2) << hash[i]))->str();
		}

		f.close();
	}

	strtoupper(output);

	return output;
}

void CheckUpdater()
{
	SetCurrentStep("Checking if autoupdater has been updated...");

	if (updaterindex < 0)
	{
		return;
	}

	if (files[updaterindex][0] != "a" && files[updaterindex][0] != "c")
	{
		SetCurrentItem("Error: File list has invalid operation on autoupdater");
		fail = true;
		return;
	}

	string hash = GetFileHash("", "autoupdater.exe");

	strtoupper(files[updaterindex][1]);

	if (files[updaterindex][1] != hash)
	{
		updaterchange = true;

		DownloadFile("", "autoupdater.exe", "autoupdater-new.exe");
	}

	if (fail)
	{
		SetCurrentItem("Failed to download new autoupdater");
		return;
	}

	if (files[updaterindex][0] == "c")
	{
		if (files[updaterindex][1] != hash)
		{
			FileCopy("", "autoupdater-new.exe", files[updaterindex][4], "autoupdater.exe");
		}
		else {
			FileCopy("", "autoupdater.exe", files[updaterindex][4], "autoupdater.exe");
		}
	}
}

void CheckConfig()
{
	SetCurrentStep("Checking if autoupdater config has been updated...");

	if (configindex < 0)
	{
		return;
	}

	if (files[configindex][0] != "a" && files[configindex][0] != "c")
	{
		SetCurrentItem("Error: File list has invalid operation on autoupdater config");
		fail = true;
		return;
	}

	string hash = GetFileHash("", "autoupdater.cfg");

	strtoupper(files[configindex][1]);

	if (files[configindex][1] != hash)
	{
		configchange = true;

		DownloadFile("", "autoupdater.cfg", "autoupdater.cfg");
	}

	if (fail)
	{
		SetCurrentItem("Failed to download new autoupdater config");
		return;
	}

	if (files[configindex][0] == "c")
	{
		FileCopy("", "autoupdater.cfg", files[configindex][4], "autoupdater.cfg");
	}
}

void CheckFiles()
{
	SetCurrentStep("Checking if files have been updated...");

	string hash;
	string s;
	string filename;
	string filepath;

	size_t num = files.size();

	int32_t n1;
	int32_t n2 = (num - (updaterindex > -1 ? 1 : 0) - (configindex > -1 ? 1 : 0));

	for (size_t i = 0; i < num; i++)
	{
		if (i == updaterindex || i == configindex)
		{
			continue;
		}

		n1 = (i + 1 - ((int32_t)i > updaterindex && updaterindex > -1 ? 1 : 0) - ((int32_t)i > configindex && configindex > -1 ? 1 : 0));
		s = "Checking if files have been updated...";
		s += static_cast<ostringstream*>(&(ostringstream() << n1))->str();
		s += "/";
		s += static_cast<ostringstream*>(&(ostringstream() << n2))->str();
		SetCurrentStep(s);

		if (files[i][0] == "r")
		{
			filename = osgetcwd(NULL, 256);
			filepath = osgetcwd(NULL, 256);

			if (files[i][2].length() > 0)
			{
				filename += "/";
				filename += files[i][2];
				filepath += "/";
				filepath += files[i][2];
			}

			filename += "/";
			filename += files[i][1];

			osrmfile(filename.c_str());

			int32_t fcount = oscountfiles(filepath.c_str());

			if (fcount == 0)
			{
				osrmdir(filepath.c_str());
			}
		}
		else {
			hash = GetFileHash(files[i][3], files[i][2]);
			strtoupper(files[i][1]);

			if (files[i][1] != hash)
			{
				if (files[i][0] == "m")
				{
					filename = osgetcwd(NULL, 256);
					filepath = osgetcwd(NULL, 256);

					if (files[i][3].length() > 0)
					{
						filename += "/";
						filename += files[i][3];
						filepath += "/";
						filepath += files[i][3];
					}

					filename += "/";
					filename += files[i][2];

					osrmfile(filename.c_str());

					int32_t fcount = oscountfiles(filepath.c_str());

					if (fcount == 0)
					{
						osrmdir(filepath.c_str());
					}
				}
				else if (files[i][0] == "c") {
					DownloadFile(files[i][3], files[i][2], files[i][2]);
					FileCopy(files[i][3], files[i][2], files[i][4], files[i][2]);
				}
				else {
					DownloadFile(files[i][3], files[i][2], files[i][2]);
				}
			}
			else {
				if (files[i][0] == "m") {
					FileCopy(files[i][3], files[i][2], files[i][4], files[i][2]);

					filename = osgetcwd(NULL, 256);
					filepath = osgetcwd(NULL, 256);

					if (files[i][3].length() > 0)
					{
						filename += "/";
						filename += files[i][3];
						filepath += "/";
						filepath += files[i][3];
					}

					filename += "/";
					filename += files[i][2];

					osrmfile(filename.c_str());

					int32_t fcount = oscountfiles(filepath.c_str());

					if (fcount == 0)
					{
						osrmdir(filepath.c_str());
					}
				}
				else if (files[i][0] == "c") {
					FileCopy(files[i][3], files[i][2], files[i][4], files[i][2]);
				}
			}
		}

		SetTotalProgress(30 + (int32_t)floor((i + 1) * (70.0 / (num - (updaterindex > -1 ? 1.0 : 0.0) - (configindex > -1 ? 1.0 : 0.0)))));
	}

	if (fail)
	{
		SetDownloadStatus("Warning: Some files failed to download");
		osgosleep(medsleeptimems);
	}
}

int32_t main(int32_t argc, char* argv[])
{
	okresponses["http"] = 200;
	okresponses["https"] = 200;
	okresponses["ftp"] = 226;
	
	bool isTemp = false;
	string pid = "";
	string tmp;
	bool showGui = true;

	for (int i = 0; i < argc; i++)
	{
		tmp = argv[i];

		//Disables the gui
		if (tmp == "-nogui")
		{
			showGui = false;
		}

		//Denotes that the current process is a temporary copy used for automatic updating of the autoupdater
		if (tmp == "-temp")
		{
			isTemp = true;
		}

		//Denotes the process id for the parent autoupdater.exe, passed to a temp run
		if (tmp.substr(0, 3) == "-p=")
		{
			pid = tmp.substr(3);
		}
	}

	if (showGui)
	{
		guiinit(progname);
	}

	if (pid.length() > 3)
	{
		oskillprocess(pid.c_str());
	}

	if (isTemp)
	{
		FileCopy("", "autoupdater-new.exe", "", "autoupdater.exe");

		string s = osgetcwd(NULL, 256);
		s += "/autoupdater-new.exe";
		osrmfile(s.c_str());

		pid = oscurrentprocessid();

		pid = "-p=" + pid;

		s = osgetcwd(NULL, 256);
		s += "/autoupdater.exe";

		char **args = new char*[2];
		args[0] = const_cast<char*>(s.c_str());
		args[1] = const_cast<char*>(pid.c_str());

		guishutdown();

		oslaunchprogram(args);

		return EXIT_SUCCESS;
	}

	string f = osgetcwd(NULL, 256);
	f += "/autoupdater-tmp.exe";
	osrmfile(f.c_str());

	osgosleep(vshortsleeptimems);
	SetCurrentStep("Testing that MD5 function works properly...1/5");

	string s = md5("The quick brown fox jumps over the lazy dog");
	string h = "9e107d9d372bb6826bd81d3542a419d6";

	strtoupper(h);

	if (s != h)
	{
		SetCurrentItem("Error: Test failed");

		osgosleep(longsleeptimems);
		guishutdown();

		return EXIT_FAILURE;
	}

	SetTotalProgress(2);
	osgosleep(vshortsleeptimems);
	SetCurrentStep("Testing that MD5 function works properly...2/5");

	s = md5("The quick brown fox jumps over the lazy dog.");
	h = "e4d909c290d0fb1ca068ffaddf22cbd0";

	strtoupper(h);

	if (s != h)
	{
		SetCurrentItem("Error: Test failed");

		osgosleep(longsleeptimems);
		guishutdown();

		return EXIT_FAILURE;
	}

	SetTotalProgress(4);
	osgosleep(vshortsleeptimems);
	SetCurrentStep("Testing that MD5 function works properly...3/5");

	s = md5("");
	h = "d41d8cd98f00b204e9800998ecf8427e";

	strtoupper(h);

	if (s != h)
	{
		SetCurrentItem("Error: Test failed");

		osgosleep(longsleeptimems);
		guishutdown();

		return EXIT_FAILURE;
	}

	SetTotalProgress(6);
	osgosleep(vshortsleeptimems);
	SetCurrentStep("Testing that MD5 function works properly...4/5");

	s = md5("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
	h = "d174ab98d277d9f5a5611c2c9f419d9f";

	strtoupper(h);

	if (s != h)
	{
		SetCurrentItem("Error: Test failed");

		osgosleep(longsleeptimems);
		guishutdown();

		return EXIT_FAILURE;
	}

	SetTotalProgress(8);
	osgosleep(vshortsleeptimems);
	SetCurrentStep("Testing that MD5 function works properly...5/5");

	s = md5("12345678901234567890123456789012345678901234567890123456789012345678901234567890");
	h = "57edf4a22be3c955ac49da2e2107b67a";

	strtoupper(h);

	if (s != h)
	{
		SetCurrentItem("Error: Test failed");

		osgosleep(longsleeptimems);
		guishutdown();

		return EXIT_FAILURE;
	}

	SetTotalProgress(10);
	osgosleep(vshortsleeptimems);

	curl_global_init(CURL_GLOBAL_ALL);

	LoadConfig();

	if (fail)
	{
		osgosleep(longsleeptimems);
		guishutdown();

		curl_global_cleanup();

		return EXIT_FAILURE;
	}

	guichangeprogname(progname);

	SetTotalProgress(15);
	osgosleep(shortsleeptimems);

	ReadFileList();

	if (fail)
	{
		osgosleep(longsleeptimems);
		guishutdown();

		curl_global_cleanup();

		return EXIT_FAILURE;
	}

	SetTotalProgress(20);
	osgosleep(shortsleeptimems);
	
	CheckUpdater();

	if (fail)
	{
		osgosleep(longsleeptimems);
		guishutdown();

		curl_global_cleanup();

		return EXIT_FAILURE;
	}

	SetTotalProgress(25);
	osgosleep(shortsleeptimems);

	CheckConfig();

	if (fail)
	{
		osgosleep(longsleeptimems);
		guishutdown();

		curl_global_cleanup();

		return EXIT_FAILURE;
	}

	if (configchange)
	{
		osgosleep(shortsleeptimems);
		LoadConfig();

		if (fail)
		{
			osgosleep(longsleeptimems);
			guishutdown();

			curl_global_cleanup();

			return EXIT_FAILURE;
		}
	}

	curl_global_cleanup();

	if (!updaterchange)
	{
		SetTotalProgress(30);
		osgosleep(shortsleeptimems);
		CheckFiles();

		SetCurrentStep("Update complete...");

		if (launch.length() > 0)
		{
			SetCurrentItem("Launching the program...");
		}
		else
		{
			SetCurrentItem("Autoupdater is now exiting...");
		}

		SetTotalProgress(100);
		osgosleep(smedsleeptimems);

		string s;
		s = osgetcwd(NULL, 256);
		s += "/";
		s += launch;

		char **args = new char*[1];
		args[0] = const_cast<char*>(s.c_str());

		guishutdown();

		if (launch.length() > 0)
		{
			oslaunchprogram(args);
		}
	}
	else
	{
		SetCurrentStep("AutoUpdater has been updated...");
		SetCurrentItem("Launching program to update the autoupdater...");

		pid = oscurrentprocessid();

		pid = "-p=" + pid;

		FileCopy("", "autoupdater-new.exe", "", "autoupdater-tmp.exe");

		string s;
		s = osgetcwd(NULL, 256);
		s += "/autoupdater-tmp.exe";

		char **args = new char*[3];
		args[0] = const_cast<char*>(s.c_str());
		args[1] = "-temp";
		args[2] = const_cast<char*>(pid.c_str());

		guishutdown();

		oslaunchprogram(args);
	}

    return EXIT_SUCCESS;
}

