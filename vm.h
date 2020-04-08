#pragma once
#include <string>
#include <fstream>
#include <stack>

class VMTranslator {
private:
	class Parser {
	public:
		enum C_TYPE {
			C_ARITHMETIC,
			C_PUSH,
			C_POP,
			C_LABEL,
			C_GOTO,
			C_IF,
			C_FUNCTION,
			C_RETURN,
			C_CALL,
			C_COMMENT
		};
		
	private:
		std::string currentCmd;
		std::ifstream vmFile;
		C_TYPE type;
		std::string argument1;
		int argument2;
		std::string vmFileName;
	
	public:
		Parser() { argument2 = 0; type = C_COMMENT; }
		void initParser(std::string filename);
		~Parser();
		bool hasMoreCommands() { return !vmFile.eof(); }
		void advance();
		C_TYPE commandType() { return type; }
		std::string arg1() { return argument1; }
		int arg2() { return argument2; }
		std::string getVMFileName() { return vmFileName; }
	};
	
	class CodeWriter {
	private:
		std::ofstream asmFile;
		const int POINTER = 3;
		const int TEMP = 5;
		int unique;
		std::string name;	// vm file name
	public:
		CodeWriter(std::string fileName);
		~CodeWriter();
		// chapter 7
		void setFileName(std::string fileName);
		void writeArithmetic(std::string command);
		void writePushPop(std::string command, std::string segment, int index);
		void updateStack(std::string command);
		void getStackData(int num);
		void close() { asmFile.close(); }
		// chapter 8
		void writeInit();
		void writeLabel(std::string label);
		void writeGoto(std::string label);
		void writeIf(std::string label);
		void writeCall(std::string functionName, int numArgs);
		void writeFunction(std::string functionName, int numLocals);
		void writeReturn();
		std::string getFileName() { return name; }
	};
	std::string* vmFiles;
	std::string asmFileName;
	CodeWriter* CW;
	Parser* prsr;
	void translate(Parser& p, CodeWriter* CW);
	int vmCount;	// keeps track of number of vm files encountered in a directory
public:
	VMTranslator(std::string path);
	~VMTranslator();
	void startOutput();
};
