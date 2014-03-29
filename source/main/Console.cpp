#include <algorithm>
#include "OpenGL.h"
#include "Console.h"
#include "Material.h"
#include "glfont.h"
#include "Globals.h"

string Console::param[10];
bool Console::AutoType=1;
int Console::curParameter=-1;
Console::CommandNode Console::CommandTree;
Console::CommandNode* Console::curNode=&Console::CommandTree;
std::vector<Console::Command*> Console::Commands;
const unsigned Console::BufferSize=CONSOLE_BUFFER_SIZE;
string Console::Buffer[Console::BufferSize];
unsigned Console::Background=0;
unsigned Console::BufferPos=0;
unsigned Console::ConsoleHeight=45;
unsigned Console::Scroll=0;
string Console::Prompt=":>";
string Console::CommandString="";
bool Console::CommandNotFound = false;

#include "ConsoleCommands.h"

void Console::LoadBackground(const char* file) {Background=MaterialManager::LoadTexture(file,0);}

void Console::AddCommand(Command* c) {
	CommandTree.parent=0;
	Commands.push_back(c);

	CommandNode* node=&CommandTree;
	string::iterator p=c->Name.begin();
	while (p!=c->Name.end()) {
		if (node->Child.find(*p) == node->Child.end()) {
			node->Child[*p]=new CommandNode();
			node->Child[*p]->parent=node;
			node->Child[*p]->Data=0;
		}
		node=node->Child[*p++];
	}
	node->Data=Commands.back();
	std::sort(Commands.begin(), Commands.end(), Console::CmdCmp);
}

void Console::ListCommands() {
	Output("\nAvailable Console Commands:\n");
	std::vector<Command*>::iterator it;
	for (it=Commands.begin(); it!=Commands.end(); ++it) {
		Output((*it)->Name);
		for (std::vector<Param>::iterator p=(*it)->Params.begin(); p!=(*it)->Params.end(); ++p) {
			Output(" <"+p->Name+">");
		}
		Output("\n");
	}
	Output("\n");
}

void Console::AddParam() {
	if (param[curParameter][0]=='.' && param[curParameter].size()==1) param[curParameter]="0.0";
	if (++curParameter > 9) curParameter=9;
	param[curParameter]="";
}

void Console::ParseParameter(unsigned char c) {
	switch (curNode->Data->Params[curParameter].Type) {
	case FLOAT: 
		if (c==46 && param[curParameter].find('.') == string::npos)
				param[curParameter] += c;
	case INT: if (!(c<48 || c>60)) param[curParameter]+=c; return;
	case BOOL: if (!(c<48 || c>49)) {param[curParameter]+=c; AddParam();}	return;
	case ENUM: return;
	case STRING: param[curParameter]+=c; return;
	}
}

void Console::CompleteCommand() {
	while (curNode->Child.size() == 1 && !curNode->Data) {
		CommandString+=curNode->Child.begin()->first;
		curNode = curNode->Child.begin()->second;		
	}
	if (curNode->Child.empty()) param[curParameter=0].clear();
}

void Console::ParseCommand(unsigned char c) 
{
	if (c != ' ' || curNode != &CommandTree)
	{
		std::map<char, CommandNode*>::iterator it = curNode->Child.find(c);
		if (it != curNode->Child.end()) {
			CommandString+=c;
			curNode=it->second;
			CommandNotFound = false;
			if (AutoType) CompleteCommand();
		}
		else
			CommandNotFound = true;
	}
}

bool Console::ReceiveKey(unsigned c) {
	switch (c) {
	case 298:
		if (ConsoleHeight+Scroll<BufferSize-1) ++Scroll;
		return 1;
	case 299:
		if (Scroll) --Scroll;
		return 1;
	case 295:
		if (curParameter>-1) {
			if (param[curParameter].size()) {
				param[curParameter].erase(param[curParameter].size()-1);
				return 1;
			}
			else {
				--curParameter;
				return 1;
			}
		}
		if (!curNode->parent) return 1;
		do {
			curNode=curNode->parent;
			CommandString.erase(CommandString.size()-1);
		}
		while (AutoType && curNode->parent && curNode->Child.size()==1);
		return 1;
	case 32:
		if (curParameter==-1) {
			if (curNode->Data) param[curParameter=0]="";
			else if(!AutoType) CompleteCommand();
		}
		else if (param[curParameter]!="") AddParam();
		return 1;
	case 294:
		if (curParameter>-1 && param[curParameter]!="") AddParam();
		{string out=" : " + CommandString;
		for (int i=0; i<=curParameter; ++i) out+=' '+param[i];
		Output(out+'\n');}
		
		if (curNode->Child.size() && !curNode->Data) 
		{
			if (CommandNotFound)
				Output("Unknown command\n");
		}
		else if (curParameter < (int)curNode->Data->Params.size()) Output("Incomplete parameters\n");
		else curNode->Data->Execute(param);
		Output("\n");
		CommandString="";
		CommandNotFound = false;
		curNode=&CommandTree;
		curParameter=-1;
		Scroll=0;
		return 1;
	default:
		if (c>301 && c<312) c-=254;
		else if (c>63 && c<91) c+=32;
		if (!(c>96 && c<123) && !(c>43 && c<58)) return 0;
		
		if (curParameter==-1) 
                        ParseCommand( (unsigned char)(c) );
		else if (curParameter < (int)curNode->Data->Params.size()) 
                        ParseParameter( (unsigned char)(c) );
		return 1;
	}
}

void Console::Output(std::string out) {
	for (string::iterator it=out.begin(); it!=out.end(); ++it) {
		if (*it!='\n') Buffer[BufferPos]+=*it;
		else {
			if (++BufferPos>=BufferSize) BufferPos=0;
			Buffer[BufferPos]="";
		}
	}
}

void Console::Render() 
{
	if (Background) 
	{
		glEnable(GL_TEXTURE_2D);
		MaterialManager::BindTexture(0, Background);
		MaterialManager::SetColor(1,1,1,.8f);
	}
	else 
	{
		MaterialManager::DisableTextures();
		MaterialManager::SetColor(.0f,.1f,.3f,.8f);
	}
	glBegin(GL_QUADS);
	glTexCoord2f(0,0); glVertex2f(1, 1);
	glTexCoord2f(0,1); glVertex2f(1, 99);
	glTexCoord2f(1,1); glVertex2f(99, 99);
	glTexCoord2f(1,0); glVertex2f(99, 1);
	glEnd();

	MaterialManager::SetColor(.2f, .8f, 1,1);

	if (!Background) 
		glEnable(GL_TEXTURE_2D);

	unsigned line=1;
	int pos=BufferPos-(ConsoleHeight+Scroll);

	if (pos<0) pos+=BufferSize;

	while (line<=ConsoleHeight) 
	{
		Font::Print(Buffer[pos++], 2.f, 2.f*line++, 2.f);
                if (pos >= BufferSize) pos=0;
	}

	string out='\n' + Prompt + CommandString;
	for (int i=0; i<=curParameter; ++i) out+=' '+param[i];
	
	if (curParameter==-1 || curParameter < (int)curNode->Data->Params.size()) 
		out+='_';
	
	if (curParameter>-1) 
	{
		out+="^00ff00";
		for (int j=curParameter; j < (int)curNode->Data->Params.size(); ++j) {
			if (j==curParameter+1) out+="^00aa00";
			out+="<"+curNode->Data->Params[j].Name+">";
		}
	}
	Font::Print(out, 2.f, 2.f*line, 2.f);
	MaterialManager::SetColor(1,1,1,1);
}

void Console::ExecuteScript(const char* filename) 
{
	FILE* file=fopen(filename, "r");
	if (!file) 
	{
		char out[50];
		_snprintf(out, 50, "Scriptfile %s not found!\n", filename);
		Output(out);
		return;
	}

	bool at=AutoType;
	AutoType=0;

	int c=0;
	bool newline=1;
	while ((c = fgetc(file) ) != EOF) {
		if (newline)
			while (isspace(c)) c=fgetc(file);
		
		if (newline && c=='#') {
			while (c!=10) c=fgetc(file);
		}
		else if (c==10) {
			ReceiveKey(294);
			newline=1;
		}
		else {
			ReceiveKey(c);
			newline=0;
		}
	}

	AutoType=at;
	fclose(file);
}
