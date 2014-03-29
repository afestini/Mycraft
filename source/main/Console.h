#ifndef CONSOLE_INCLUDED
#define CONSOLE_INCLUDED

#include <string>
#include <map>
#include <list>
#include <vector>
#include "Material.h"
using std::string;

#define CONSOLE_BUFFER_SIZE 200

class Console {
public:
	enum pType {INT, FLOAT, BOOL, STRING, ENUM};
	struct Param {
		string Name;
		pType Type;
		Param(const char* name, pType type) : Name(name), Type(type) {}
	};
	struct Command {
		string Name;
		std::vector<Param> Params;

		virtual void Execute(string* str)=0;
		virtual ~Command() {}
	};
	static bool CmdCmp(Command* a, Command* b) {return a->Name<b->Name;}
private:
	struct CommandNode {
		CommandNode* parent;
		std::map<char, CommandNode*> Child;
		Command* Data;
	};

	static unsigned Background;
	static string param[10];
	static int curParameter;
	static CommandNode* curNode;
	static std::vector<Command*> Commands;
	static CommandNode CommandTree;
	static unsigned const BufferSize;
	static string Buffer[CONSOLE_BUFFER_SIZE];
	static unsigned BufferPos;
	static unsigned ConsoleHeight;
	static unsigned Scroll;
	static string Prompt;
	static string CommandString;
	static bool CommandNotFound;
	
	static void ParseCommand(unsigned char c);
	static void ParseParameter(unsigned char c);
	static void CompleteCommand();
	static void AddParam();
public:
	static bool AutoType;
	static void AddCommand(Command* c);
	static bool ReceiveKey(unsigned c);
	static void Render();
	static void Output(string out);
	static void ListCommands();
	static void ExecuteScript(const char* file);
	static void LoadBackground(const char* file);
};

#endif
