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

int modCount = 0;
int line = 1;
int linecount = -1;

string text;
int idx = 0;

class module
{
public:
    map<string,int> symVal;
	int countVal;
	
	vector<string> symName;
	map<string,int> uselist;
	int countName;
	
	vector<string> ins;
	int countIns;

	int offset;

	module *next;
	module *prev;
};

module *modules = new module();
module *head = modules;

map<string,int> allVal;
map<string,int> ValMod;

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

map<string,int> ValErr;
map<string,int> ValUse;
map<string,int> ValDef;
vector<string> InsErr;
int totalIns = 0;

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

static inline void filter()
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

void ParseVal1()
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
		filter();
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

		filter();
		errline = line;
	    erridx = idx;
	    errlinecount = linecount;

		for(;text[idx] >= '0' && text[idx] <= '9';idx++)
			tmp += text[idx];

		if(tmp.size() == 0)
			syntaxError(NUM_EXPECTED,errline,erridx,errlinecount);

		val = atoi(tmp.c_str());
		tmp.clear();

		modules->symVal[valname] = val + modules->offset;
	}
}

void ParseTable1()
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

    count = atoi(tmp.c_str());
	if(count > 16)
		syntaxError(TOO_MANY_USE_IN_MODULE,errline,erridx,errlinecount);
	tmp.clear();

	for(;count != 0;count--)
	{
	    filter();
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
		tmp.clear();
	}
	
}

void ParseIns1()
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
	    filter();
	    errline = line;
	    erridx = idx;
	    errlinecount = linecount;
		
	    if(text[idx] == 'R' || text[idx] == 'I' || text[idx] == 'E' || text[idx] == 'A')
			tmp += text[idx++];
		else
			syntaxError(ADDR_EXPECTED,errline,erridx,errlinecount);

		filter();
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

void ParseVal2()
{
	int count = modules->countVal;
	
    for(;text[idx] >= '0' && text[idx] <= '9';idx++); 

	for(;count != 0;count--)
	{
		filter();	
	    for(;text[idx] != ' ' && text[idx] != '\n' && text[idx] != '\t';idx++);
		filter();
		for(;text[idx] >= '0' && text[idx] <= '9';idx++);
	}
}

void ParseTable2()
{
    string tmp;
	int count;

	for(;text[idx] >= '0' && text[idx] <= '9';idx++)
        tmp += text[idx]; 

    modules->countName = atoi(tmp.c_str());
	count = modules->countName;
	tmp.clear();

	for(;count != 0;count--)
	{
	    filter();
		
	    for(;text[idx] != ' ' && text[idx] != '\n' && text[idx] != '\t';idx++)
	        tmp += text[idx];

		modules->symName.push_back(tmp);	
		modules->uselist[tmp] = 0;
		tmp.clear();
	}
	
}

void ParseIns2()
{
    string tmp;
	int count = modules->countIns;

	for(;text[idx] >= '0' && text[idx] <= '9';idx++);

	for(;count != 0;count--)
	{
	    filter();
		
	    if(text[idx] == 'R' || text[idx] == 'I' || text[idx] == 'E' || text[idx] == 'A')
			tmp += text[idx++];

		filter();

		for(;text[idx] >= '0' && text[idx] <= '9';idx++)
			tmp += text[idx];
		
		modules->ins.push_back(tmp);
		tmp.clear();
	}
}

void ParseModule1()
{
    if(modules->prev != NULL)
		modules->offset = modules->prev->offset + modules->prev->countIns;
	else
		modules->offset = 0;	
	
    ParseVal1();
	filter();
	ParseTable1();
	filter();
	ParseIns1();
}

void ParseModule2()
{	
    ParseVal2();
	filter();
	ParseTable2();
	filter();
	ParseIns2();
}

void Passone(string filename)
{
    text = readFileIntoString(filename.c_str());
	filter();
	for(;idx < text.size() - 1 && text[idx + 1] != EOF;)
	{
		ParseModule1();
        modCount++;
		
		modules->next = new module();
		modules->next->prev = modules;
		modules = modules->next;

		filter();
    }
	modules = head;
	idx = 0;
}

void Passtwo()
{
    filter();
    for(;idx < text.size() - 1 && text[idx + 1] != EOF;)
	{
		ParseModule2();
		modules = modules->next;
		filter();
    }
	modules = head;
}

void OutAllVal()
{
	map<string,int>:: iterator iter;
    for(int i = 0;i < modCount;i++)
    {
		for(iter = modules->symVal.begin(); iter != modules->symVal.end();iter++)
		{
		    if(allVal[iter->first] != 0)
			{
				ValErr[iter->first] = 1;
				continue;
		    }
			else
			{
			    ValMod[iter->first] = i + 1;
				ValDef[iter->first] = 1;
			    allVal[iter->first] = iter->second;
			}

			if(iter->second > modules->offset + modules->countIns - 1)
			{
			    cout <<"Warning: Module " << (i + 1) << ": "<< iter->first << " too big " << iter->second << " (max=" << (modules->offset + modules->countIns - 1) <<") assume zero relative" <<endl;
				allVal[iter->first] = 0;
			}
		}

		modules = modules->next;
    }
	modules = head;

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

void OutAllIns()
{
    for(int i = 0;i < modCount;i++)
	{
	    for(int j = 0;j < modules->countIns;j++)
	    {
	        string str;
			int tmp;
			int size = modules->ins[j].size();
	        if(size > 5)
	        {
	            if(modules->ins[j][0] == 'I')
	                InsErr.push_back("Error: Illegal immediate value; treated as 9999");
				else
				    InsErr.push_back("Error: Illegal opcode; treated as 9999");
				tmp = 9999;
				goto out;
	        }
			
	        if(modules->ins[j][0] == 'I')
			{
			    tmp = atoi(modules->ins[j].substr(1,size - 1).c_str());
				InsErr.push_back("No");
	        }
			else if(modules->ins[j][0] == 'A')
			{
			    tmp = atoi(modules->ins[j].substr(2,4).c_str());
				if(tmp >= 512)
				{
				    tmp = atoi((modules->ins[j].substr(1,1) + "000").c_str());
				    InsErr.push_back("Error: Absolute address exceeds machine size; zero used");
				}
				else
				{
				    tmp = atoi((modules->ins[j].substr(1,4)).c_str());
				    InsErr.push_back("No");
				}
			}
			else if(modules->ins[j][0] == 'R' )
			{
			    if(size == 5)
					tmp = atoi(modules->ins[j].substr(2,4).c_str());
				else
					tmp = atoi(modules->ins[j].substr(1,size - 1).c_str());
				if(tmp > modules->offset + modules->countIns - 1)
				{
				    tmp = atoi((modules->ins[j].substr(1,1)).c_str()) * 1000 + modules->offset;
				    InsErr.push_back("Error: Relative address exceeds module size; zero used");
				}
				else
				{
				    tmp = atoi((modules->ins[j].substr(1,size - 1)).c_str()) + modules->offset;
					InsErr.push_back("No");
				}
			}
			else if(modules->ins[j][0] == 'E')
			{
			    tmp = atoi((modules->ins[j].substr(2,4).c_str()));
				if(tmp >= modules->countName)
				{
				    tmp = atoi((modules->ins[j].substr(1,4)).c_str());
				    InsErr.push_back("Error: External address exceeds length of uselist; treated as immediate");
					goto out;
				}
				str = modules->symName[tmp];
				if(str.size() == 0 || ValDef[str] != 1)
				{
				    modules->uselist[str]++;
				    str = "Error: " + str + " is not defined; zero used";
				    tmp = 0 + atoi((modules->ins[j].substr(1,4)).c_str()) - tmp;
				    InsErr.push_back(str);
				}
				else
				{
				    tmp = allVal[str] + atoi((modules->ins[j].substr(1,4)).c_str()) - tmp;
					ValUse[str]++;
					modules->uselist[str]++;
					InsErr.push_back("No");
				}
			}
			out:
			str = to_string(tmp);
			for(;str.size() < 4;)
			    str = "0" + str;
			modules->ins[j] = str;
	    }
		
		modules = modules->next;
	}

	modules = head;

	map<string,int>:: iterator iter;
	cout<< "Memory Map" << endl;
    int countIns = 0;
	for(int i = 0;i < modCount;i++)
	{
	    for(int j = 0;j < modules->countIns;j++,countIns++)
	    {
	        if(countIns < 10)
	            cout<< "00" << countIns << ":" << " " << modules->ins[j];
		    else if(countIns >= 10 && countIns < 100)
			    cout<< "0" << countIns << ":" << " " << modules->ins[j];
		    else if(countIns >= 100)
			    cout<< countIns << ":" << " " << modules->ins[j];
			if(InsErr[countIns] != "No")
			    cout<< " " << InsErr[countIns];
		    cout<< endl;
	    }
		for(iter = modules->uselist.begin();iter != modules->uselist.end();iter++)
		{
		    if(iter->second == 0)
				cout<<"Warning: Module " << (i + 1) << ": " << iter->first << " appeared in the uselist but was not actually used" << endl;
		}
		modules = modules->next;

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
	OutAllVal();
	Passtwo();
	OutAllIns();
    return 0;
}

