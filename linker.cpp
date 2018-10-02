#include <iostream>
#include <cstring>
#include <map>
#include <string.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <vector>

using namespace std;

int line = 1;
int linecount = -1;
int idx = 0;

// Metadata in each modules
class module
{
public:
    int moduleNO;

	// Uselist
	vector<string> symName;
	map<string,int> uselist;

	// Counts
	int countVal;
	int countName;
	int countIns;

	int offset;

	module *next;
	module *prev;
};

module *modules = new module();
module *head = modules;

// Symbol Table
map<string,int> allVal;       
map<string,int> ValMod;

// Error Type
static string errstr[] = 
{
    "NUM_EXPECTED",           // Number expect
    "SYM_EXPECTED",           // Symbol Expected
    "ADDR_EXPECTED",          // Addressing Expected which is A/E/I/R
    "SYM_TOO_LONG",           // Symbol Name is too long
    "TOO_MANY_DEF_IN_MODULE", // > 16
    "TOO_MANY_USE_IN_MODULE", // > 16
    "TOO_MANY_INSTR",         // total num_instr exceeds memory size (512)
};

enum errors
{
    NUM_EXPECTED, SYM_EXPECTED, ADDR_EXPECTED, SYM_TOO_LONG, TOO_MANY_DEF_IN_MODULE, TOO_MANY_USE_IN_MODULE, TOO_MANY_INSTR
};

int totalIns = 0;
int countIns = 0;

string readFileIntoString(const char * filename)
{
    ifstream ifile(filename);
    ostringstream buf;
    char ch;
    while(buf && ifile.get(ch))
        buf.put(ch);
    return buf.str();
}

static inline void syntaxError(int e,int errline,int erridx,int errlinecount)
{
    cout<< "Parse Error line " << errline <<" offset " << (erridx - errlinecount) <<": " << errstr[e] << endl;
	exit(0);
}

static inline void filter(string text)
{
    for(;idx < text.size() - 1 && text[idx + 1] != EOF && (text[idx] == ' ' || text[idx] == '\n' || text[idx] == '\t');idx++)
	{
	    if(text[idx] == '\n')
		{
		    line++;
			linecount = idx;
	    }
	}
}

void ReadSym(string text,map<string,int>&symVal)
{
    string tmp;
	int count;
	string valname;
	int val;

	int errline = line;
	int erridx = idx;
	int errlinecount = linecount;

	int flag = 0;
	
    for(;text[idx] >= '0' && text[idx] <= '9';idx++)
        tmp += text[idx]; 

	if(tmp.size() == 0)
		syntaxError(NUM_EXPECTED,errline,erridx,errlinecount);

	modules->countVal = atoi(tmp.c_str());
	count = modules->countVal;
	if(count > 16)
		syntaxError(TOO_MANY_DEF_IN_MODULE,errline,erridx,errlinecount);
	tmp.clear();

	for(;count != 0;count--)
	{
		filter(text);
		errline = line;
	    erridx = idx;
	    errlinecount = linecount;
		
	    for(;text[idx] != ' ' && text[idx] != '\n' && text[idx] != '\t';idx++)
	    {
		    if(text[idx] >= 'a' && text[idx] <= 'z' || text[idx] >= 'A' && text[idx] <= 'Z')
				flag = 1;
		    tmp += text[idx];
	    }

		if(tmp.size() == 0 || flag == 0)
			syntaxError(SYM_EXPECTED,errline,erridx,errlinecount);
		if(tmp.size() > 16)
			syntaxError(SYM_TOO_LONG,errline,erridx,errlinecount);

		valname = tmp;
		flag = 0;
		tmp.clear();

		filter(text);
		errline = line;
	    erridx = idx;
	    errlinecount = linecount;

		for(;text[idx] >= '0' && text[idx] <= '9';idx++)
			tmp += text[idx];

		if(tmp.size() == 0)
			syntaxError(NUM_EXPECTED,errline,erridx,errlinecount);

		val = atoi(tmp.c_str());
		tmp.clear();

		symVal[valname] = val + modules->offset;
	}
}

void StoreUselist(string text)
{
    string tmp;
	int count;

	int errline = line;
	int erridx = idx;
	int errlinecount = linecount;

	int flag = 0;

	for(;text[idx] >= '0' && text[idx] <= '9';idx++)
        tmp += text[idx]; 

	if(tmp.size() == 0)
		syntaxError(NUM_EXPECTED,errline,erridx,errlinecount);

	modules->countName = atoi(tmp.c_str());
	count = modules->countName;
	if(count > 16)
		syntaxError(TOO_MANY_USE_IN_MODULE,errline,erridx,errlinecount);
	tmp.clear();

	for(;count != 0;count--)
	{
	    filter(text);
		errline = line;
	    erridx = idx;
	    errlinecount = linecount;
		
	    for(;text[idx] != ' ' && text[idx] != '\n' && text[idx] != '\t';idx++)
		{
		    if(text[idx] >= 'a' && text[idx] <= 'z' || text[idx] >= 'A' && text[idx] <= 'Z')
				flag = 1;
		    tmp += text[idx];
	    }

		if(tmp.size() == 0 || flag == 0)
			syntaxError(SYM_EXPECTED,errline,erridx,errlinecount);
		if(tmp.size() > 16)
			syntaxError(SYM_TOO_LONG,errline,erridx,errlinecount);

		flag = 0;
		modules->symName.push_back(tmp);	
		modules->uselist[tmp] = 0;
		tmp.clear();
	}
	
}

void DetectIns(string text)
{
    string tmp;
	int count;

	int errline = line;
	int erridx = idx;
	int errlinecount = linecount;

	for(;text[idx] >= '0' && text[idx] <= '9';idx++)
        tmp += text[idx]; 

	if(tmp.size() == 0)
		syntaxError(NUM_EXPECTED,errline,erridx,errlinecount);

    modules->countIns = atoi(tmp.c_str());
	count = modules->countIns;
	totalIns += count;
	if(totalIns > 512)
		syntaxError(TOO_MANY_INSTR,errline,erridx,errlinecount);
	tmp.clear();

	for(;count != 0;count--)
	{
	    filter(text);
	    errline = line;
	    erridx = idx;
	    errlinecount = linecount;
		
	    if(text[idx] == 'R' || text[idx] == 'I' || text[idx] == 'E' || text[idx] == 'A')
			tmp += text[idx++];
		else
			syntaxError(ADDR_EXPECTED,errline,erridx,errlinecount);

		filter(text);
		errline = line;
	    erridx = idx;
	    errlinecount = linecount;

		for(;text[idx] >= '0' && text[idx] <= '9';idx++)
			tmp += text[idx];

        if(tmp.size() == 0)
		    syntaxError(NUM_EXPECTED,errline,erridx,errlinecount);
		tmp.clear();
	}

}

void BuildSymtable(map<string,int> symVal,map<string,int>&ValErr)
{
    map<string,int>:: iterator iter;
	for(iter = symVal.begin(); iter != symVal.end();iter++)
	{
		if(allVal.find(iter->first) != allVal.end())
		{
			ValErr[iter->first] = 1;
			continue;
		}
		else
		{
			ValMod[iter->first] = modules->moduleNO;
			allVal[iter->first] = iter->second;
		}

		if(iter->second > modules->offset + modules->countIns - 1)
		{
			cout <<"Warning: Module " << modules->moduleNO << ": "<< iter->first << " too big " << iter->second << " (max=" << (modules->offset + modules->countIns - 1) <<") assume zero relative" <<endl;
			allVal[iter->first] = 0;
		}
	}
}

void PassUseless(string text)
{
	int count = modules->countVal;
	
    for(;text[idx] >= '0' && text[idx] <= '9';idx++); 

	for(;count != 0;count--)
	{
		filter(text);	
	    for(;text[idx] != ' ' && text[idx] != '\n' && text[idx] != '\t';idx++);
		filter(text);
		for(;text[idx] >= '0' && text[idx] <= '9';idx++);
	}
    filter(text);
	count = modules->countName;
	
	for(;text[idx] >= '0' && text[idx] <= '9';idx++);

	for(;count != 0;count--)
	{
	    filter(text);	
	    for(;text[idx] != ' ' && text[idx] != '\n' && text[idx] != '\t';idx++);
	}
}

void GenerateInsOutput(string text,map<string,int>&ValUse)
{
    string ins;
	string InsErr;
	int count = modules->countIns;

	for(;text[idx] >= '0' && text[idx] <= '9';idx++);

	for(;count != 0;count--)
	{
	    filter(text);
		
	    if(text[idx] == 'R' || text[idx] == 'I' || text[idx] == 'E' || text[idx] == 'A')
			ins += text[idx++];

		filter(text);

		for(;text[idx] >= '0' && text[idx] <= '9';idx++)
			ins += text[idx];
				
	    string str;
		int tmp;
		int size = ins.size();
		if(size > 5)
	    {
	        if(ins[0] == 'I')
				InsErr = "Error: Illegal immediate value; treated as 9999";
			else
				InsErr = "Error: Illegal opcode; treated as 9999";
			tmp = 9999;
			goto out;
		}			
		if(ins[0] == 'I')
		{
			tmp = atoi(ins.substr(1,size - 1).c_str());
			InsErr = "No";
		}
		else if(ins[0] == 'A')
		{
			tmp = atoi(ins.substr(2,4).c_str());
			if(tmp >= 512)
			{
				tmp = atoi((ins.substr(1,1) + "000").c_str());
				InsErr = "Error: Absolute address exceeds machine size; zero used";
			}
			else
			{
				tmp = atoi((ins.substr(1,4)).c_str());
				InsErr = "No";
			}
		}
		else if(ins[0] == 'R' )
		{
			if(size == 5)
			    tmp = atoi(ins.substr(2,4).c_str());
			else
				tmp = atoi(ins.substr(1,size - 1).c_str());
			if(tmp > modules->offset + modules->countIns - 1)
			{
				tmp = atoi((ins.substr(1,1)).c_str()) * 1000 + modules->offset;
				InsErr = "Error: Relative address exceeds module size; zero used";
			}
			else
			{
				tmp = atoi((ins.substr(1,size - 1)).c_str()) + modules->offset;
				InsErr = "No";
			}
		}
		else if(ins[0] == 'E')
		{
			tmp = atoi((ins.substr(2,4).c_str()));
			if(tmp >= modules->countName)
			{
				tmp = atoi((ins.substr(1,4)).c_str());
				InsErr = "Error: External address exceeds length of uselist; treated as immediate";
				goto out;
			}
			str = modules->symName[tmp];
			if(str.size() == 0 || allVal.find(str) == allVal.end() )
			{
				modules->uselist[str]++;
				str = "Error: " + str + " is not defined; zero used";
				tmp = 0 + atoi((ins.substr(1,4)).c_str()) - tmp;
				InsErr = str;
			}
			else
			{
				tmp = allVal[str] + atoi((ins.substr(1,4)).c_str()) - tmp;
				ValUse[str]++;
				modules->uselist[str]++;
				InsErr = "No";
			}
		}
		out:
		str = to_string(tmp);
		for(;str.size() < 4;)
			str = "0" + str;
		ins = str;

        if(countIns < 10)
	        cout<< "00" << countIns << ":" << " " << ins;
		else if(countIns >= 10 && countIns < 100)
			cout<< "0" << countIns << ":" << " " << ins;
		else if(countIns >= 100)
			cout<< countIns << ":" << " " << ins;
	    if(InsErr != "No")
			cout<< " " << InsErr;
		cout<< endl;
		countIns++;

		ins.clear();
	}
}

void ParseAndStoreModuleSym(string text,map<string,int>&ValErr)
{
    if(modules->prev != NULL)
    {
		modules->offset = modules->prev->offset + modules->prev->countIns;
		modules->moduleNO = modules->prev->moduleNO + 1;
    }
	else
	{
		modules->offset = 0;
		modules->moduleNO = 1;
	}
	map<string,int> symVal;
    ReadSym(text,symVal);
	filter(text);
	StoreUselist(text);
	filter(text);
	DetectIns(text);
	BuildSymtable(symVal,ValErr);
}

void Passone(string filename)
{
    string text = readFileIntoString(filename.c_str());
	map<string,int> ValErr;
	filter(text);
	for(;idx < text.size() - 1 && text[idx + 1] != EOF;)
	{
		ParseAndStoreModuleSym(text,ValErr);		
		modules->next = new module();
		modules->next->prev = modules;
		modules = modules->next;

		filter(text);
    }
	modules = head;
	idx = 0;

    map<string,int> :: iterator iter;
	cout<< "Symbol Table" << endl;
	for(iter = allVal.begin(); iter != allVal.end();iter++)
	{
	    cout<< iter->first << "=" << iter->second;
		if(ValErr[iter->first] == 1)
			cout<< " Error: This variable is multiple times defined; first value used";
		cout<< endl;
	}
	cout<< endl;
}

void Passtwo(string filename)
{
    string text = readFileIntoString(filename.c_str());
    map<string,int>:: iterator iter;
	map<string,int> ValUse;
	cout<< "Memory Map" << endl;
    filter(text);
    for(;idx < text.size() - 1 && text[idx + 1] != EOF;)
	{
	    map<string,int> use;
		PassUseless(text);
	    filter(text);
	    GenerateInsOutput(text,ValUse);
	    for(iter = modules->uselist.begin();iter != modules->uselist.end();iter++)
	    {
		    if(iter->second == 0)
		        cout<<"Warning: Module " << modules->moduleNO << ": " << iter->first << " appeared in the uselist but was not actually used" << endl;
	    }
		modules = modules->next;
		filter(text);
    }
	modules = head;

    cout<< endl;
	for(iter = allVal.begin(); iter != allVal.end();iter++)
	{
	    
		if(ValUse[iter->first] == 0)
			cout<< "Warning: Module " << ValMod[iter->first] << ": " << iter->first << " was defined but never used" << endl;
	}
	
	cout<< endl;
}

int main(int argc,char* argv[])
{
    string fn = argv[1];
	Passone(fn);
	Passtwo(fn);
    return 0;
}

