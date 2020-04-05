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
	
	public:
		Parser(std::string filename);
		~Parser();
		bool hasMoreCommands() { return !vmFile.eof(); }
		void advance();
		C_TYPE commandType() { return type; }
		std::string arg1() { return argument1; }
		int arg2() { return argument2; }
	};
	
	class CodeWriter {
	private:
		int stackSize;
		std::ofstream asmFile;
		const int STACK_ADDRESS = 256;
		const int POINTER = 3;
		const int TEMP = 5;
		int unique;
		std::string name;
	public:
		CodeWriter(std::string fileName);
		~CodeWriter();
		// chapter 7
		void setFileName(std::string fileName);
		void writeArithmetic(std::string command);
		void writePushPop(std::string command, std::string segment, int index);
		void close() { asmFile.close(); }
		// chapter 8
	};
	std::string* vmFiles;
	std::string asmFileName;
	CodeWriter* CW;
public:
	VMTranslator(std::string path);
	~VMTranslator();
};
