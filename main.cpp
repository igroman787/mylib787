// -*- coding: utf_8 -*-
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <dirent.h>
#include <time.h>
#include <ctime>
#include "json.hpp"
#include <unistd.h> // sleep
using namespace std;
using json = nlohmann::json;

// Global variables
json localdb;
json localbuffer;

class Bcolors {
	// This class is designed to display text in color format
	public:
		string DEBUG = "\033[95m";
		string INFO = "\033[94m";
		string OKGREEN = "\033[92m";
		string WARNING = "\033[93m";
		string ERROR = "\033[91m";
		string ENDC = "\033[0m";
		string BOLD = "\033[1m";
		string UNDERLINE = "\033[4m";
};


void print(int item) {
	cout << item << endl;
}
void print(string text) {
	cout << text << endl;
}

string ts2dt(const time_t timestamp) {
	struct tm * timeinfo;
	char buffer [80];
	timeinfo = gmtime(&timestamp);
	strftime(buffer,80,"%d.%m.%Y, %H:%M:%S (UTC)",timeinfo);
	return buffer;
}

void Ljust(string& str, int n, char ch = ' ') {
	int len = str.length();
	int need = n - len;
	for (int i=0; i<need; i++) {
		str += ch;
	}
}

void AddLog(string inputText, string mode = "info") {
	time_t timestamp;
	char buffer [80];
	string logText, timeText, modeText, threadText, colorStart;
	Bcolors bcolors;


	timestamp = time(nullptr);
	timeText = ts2dt(timestamp);

	// Pass if set log level
	if (localdb["logLevel"] != "debug" && mode == "debug") {
		return;
	}
	if (localdb["isIgnorLogWarning"] == true && mode == "warning") {
		return;
	}

	// Set color mode
	if (mode == "info") {
		colorStart = bcolors.INFO + bcolors.BOLD;
	}
	else if (mode == "warning") {
		colorStart = bcolors.WARNING + bcolors.BOLD;
	}
	else if (mode == "error") {
		colorStart = bcolors.ERROR + bcolors.BOLD;
	}
	else if (mode == "debug") {
		colorStart = bcolors.DEBUG + bcolors.BOLD;
	}
	else {
		colorStart = bcolors.UNDERLINE + bcolors.BOLD;
	}
	snprintf(buffer, sizeof(buffer), "%s[%s]%s", colorStart.c_str(), mode.c_str(), bcolors.ENDC.c_str());
	modeText = buffer;

	// Set color thread
	if (mode == "error") {
		colorStart = bcolors.ERROR + bcolors.BOLD;
	}
	else {
		colorStart = bcolors.OKGREEN + bcolors.BOLD;
	}
	// threadText = "{0}{1}{2}".format(colorStart, "<{0}>".format(GetThreadName()).ljust(14, ' '), bcolors.ENDC)
	snprintf(buffer, sizeof(buffer), "%s<%s>%s", colorStart.c_str(), "0", bcolors.ENDC.c_str());
	threadText = buffer;

	Ljust(modeText, 23);
	Ljust(timeText, 28);
	Ljust(threadText, 21);

	logText = modeText + timeText + threadText + inputText;

	print(logText);
}

wstring s2ws(const string& str) {
	using convert_typeX = codecvt_utf8<wchar_t>;
	wstring_convert<convert_typeX, wchar_t> converterX;
	return converterX.from_bytes(str);
}

string ws2s(const wstring& wstr) {
	using convert_typeX = codecvt_utf8<wchar_t>;
	wstring_convert<convert_typeX, wchar_t> converterX;
	return converterX.to_bytes(wstr);
}

int Count(string text, string sub) {
	int out = 0;
	int start = 0;
	int textlen = text.length();
	while (true) {
		start = text.find(sub, start) + sub.length();
		if (start <= 0 || start > textlen) {
			break;
		}
		else {
			out++;
		}
	}
	return out;
}

vector<string> Split(string text, char delimiter = ' ') {
	int start = 0;
	int end;
	int total;
	string buff;
	vector<string> arr;

	while (true) {
		end = text.find(delimiter, start);
		if (end < 0) {
			break;
		}
		total = end-start;
		buff = text.substr(start, total);
		arr.push_back(buff);
		start = end+1;
	}
	return arr;
}

string ReadFile(string fname) {
	ifstream file;
	string buffer;

	file.open(fname);
	if (file.is_open() == false) {
		cout << "Can't open file: " << fname << endl;
	}
	string content( (istreambuf_iterator<char>(file) ),
					(istreambuf_iterator<char>()     ) );
	return content;
}

string as(string dirname) {
	int a = dirname.size()-1;
	char s = dirname[a];
	if (s != '/') {
		dirname.append("/");
	}
	return dirname;
}

vector<string> OpenDirectory(string dirname, string mod) {
	vector<string> output;

	const char *c_dirname = dirname.c_str();
	DIR *dir = opendir(c_dirname);
	if (dir) {
		struct dirent *ent;
		while ((ent = readdir(dir)) != NULL) {
			dirname = ent->d_name;
			if (dirname != "." && dirname != "..") {
				int a = dirname.find(".");
				if (a < 0 && mod == "dirs") {
					output.push_back(dirname);
				}
				else if (a > 0 && mod == "files") {
					output.push_back(dirname);
				}
			}
		}
		closedir(dir);
	}
	else {
		string error_msg = "Error opening directory: ";
		print(error_msg.append(dirname));
	}
	return output;
}

vector<string> ReadDirectory(string dirname) {
	vector<string> buff;
	vector<string> buff2;
	vector<string> output;

	buff = OpenDirectory(dirname, "files");
	for (string item : buff) {
		output.push_back(as(dirname).append(item));
	}

	buff = OpenDirectory(dirname, "dirs");
	for (string item : buff) {
		string dirname2 = as(dirname);
		dirname2 = dirname2.append(item);
		buff2 = ReadDirectory(dirname2);
		for (string item2 : buff2) {
			output.push_back(item2);
		}
	}
	return output;
}

bool IsItemInArray(vector<string>& arr, string& str) {
	for (string item : arr) {
		if (item == str) {
			return true;
		}
	}
	return false;
}

bool IsItemInArray(vector<int>& arr, int& i) {
	for (int item : arr) {
		if (item == i) {
			return true;
		}
	}
	return false;
}

void JsonTest() {
	localbuffer["name"] = "myname";
	localbuffer["nothing"] = nullptr;
	localbuffer["answer"]["everything"] = 42;
	localbuffer["companies"] = { "Infopulse", "TM" };
	localbuffer["user"] = { {"name", "tangro"}, {"active", true} };

	json j = "{ \"active\": true, \"pi\": 3.141 }"_json;
	json j2 = json::parse("{ \"active\": true, \"pi\": 3.141 }");
	string s = j.dump();
	j.size();
	j.empty();
	j.type();
	j.clear();


	localbuffer["companies"].push_back("foo");
	localbuffer["nothing"].push_back(1);
	
	json o;
	o["foo"] = 23;
	o["bar"] = false;
	if (o.find("foo") != o.end()) {
		print("yep!");
	}

	print(localbuffer.dump(4));
}

///
/// Start of the program
///

int main(int argc, char * argv []) {
	AddLog("Start of the program");
	localdb["logLevel"] = "debug";

	//JsonTest();
	for (int i=0; i<1000; i++) {
		sleep(1);
		AddLog("hi");
	}

	return 0;
}
