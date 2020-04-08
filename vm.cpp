#include "vm.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

using namespace std;

/*VM TRANSLATOR FUNCTIONS*/
VMTranslator::VMTranslator(std::string path) { // initialize parsers here
	vmCount = 0;
	if (fs::path(path).extension() == ".vm") {
		// open vm file
		vmFiles = new string[1];
		*(vmFiles) = fs::path(path).string();
		asmFileName = vmFiles[0].substr(0, vmFiles[0].length() - 3) + ".asm";
		vmCount = 1;
	}
	else {
		asmFileName = fs::path(path).string();
		// parse directory for any vm files, then add to list of vm files to be processed
		// get vm file count
		for (auto& p : fs::directory_iterator(path))
			if (p.path().extension() == ".vm")
				vmCount++;
		// allocate space for vmFiles, then add filenames to vmFiles
		vmFiles = new string[vmCount];
		int i = 0;
		for (auto& p : fs::directory_iterator(path))
			if (p.path().extension() == ".vm") {
				*(vmFiles + i) = p.path().string();
				i++;
			}
	}
	// parser & codewriter initialization
	CW = new CodeWriter(asmFileName);
	prsr = new Parser[vmCount];
	for (int a = 0; a < vmCount; a++)
		prsr[a].initParser(vmFiles[a]);

	// writeInit call here
	return;
}

void VMTranslator::translate(Parser& p, CodeWriter* CW) {
	// set function name here
	CW->setFileName(p.getVMFileName());
	while (p.hasMoreCommands()) {
		p.advance();
		if (p.commandType() == Parser::C_ARITHMETIC)
			CW->writeArithmetic(p.arg1());
		else if (p.commandType() == Parser::C_PUSH) {
			CW->writePushPop("push", p.arg1(), p.arg2());
		}

		else if (p.commandType() == Parser::C_POP) {
			CW->writePushPop("pop", p.arg1(), p.arg2());
		}
	/*	else if (p.commandType() == Parser::C_LABEL) {
			CW->writeLabel(p.arg1());
		}
		else if (p.commandType() == Parser::C_GOTO) {
			CW->writeGoto(p.arg1());
		}
		else if (p.commandType() == Parser::C_IF) {
			CW->writeIf(p.arg1());
		}
		else if (p.commandType() == Parser::C_FUNCTION)
			CW->writeFunction(p.arg1(), p.arg2());
		else if (p.commandType() == Parser::C_CALL)
			CW->writeCall(p.arg1(), p.arg2());
		else if (p.commandType() == Parser::C_RETURN)
			CW->writeReturn();*/
	}
}

void VMTranslator::startOutput() {
	for (int i = 0; i < vmCount; i++)
		translate(prsr[i], CW);
}

VMTranslator::~VMTranslator() {
	delete[] vmFiles;
	delete CW;
	return;
}

/*PARSER FUNCTIONS*/
void VMTranslator::Parser::initParser(string filename) {
	currentCmd = "";
	argument1 = "";
	argument2 = 0;
	type = C_COMMENT;
	vmFileName = filename;
	vmFile.open(filename);
	if (!vmFile.is_open())
		return;
	return;
}

VMTranslator::Parser::~Parser() {
	vmFile.close();
}

void VMTranslator::Parser::advance() {
	string command;
	if (!hasMoreCommands())
		return;
	getline(vmFile, currentCmd);
	stringstream ss(currentCmd);
	getline(ss, command, ' ');

	// if comment ignore
	if (command == "//") {
		type = C_COMMENT;
		return;
	}
	// determine command type
	if (command == "push")
		type = C_PUSH;
	else if (command == "pop")
		type = C_POP;
	/*else if (command == "label")
		type = C_LABEL;
	else if (command == "goto")
		type = C_GOTO;
	else if (command == "if-goto")
		type = C_IF;
	else if (command == "function")
		type = C_FUNCTION;
	else if (command == "call")
		type = C_CALL;
	else if (command == "return")
		type = C_RETURN;*/
	else 	
		type = C_ARITHMETIC;

	// extract arguments
	if (type == C_PUSH || type == C_POP || type == C_FUNCTION || type == C_CALL) {
		string x;	// for arg2 int
		getline(ss, argument1, ' ');
		getline(ss, x, ' ');
		argument2 = stoi(x);
	}
	else if (type == C_LABEL || type == C_GOTO || type == C_IF) {
		getline(ss, argument1, ' ');	// getting symbol
	}
	else if (type == C_ARITHMETIC || type == C_RETURN)
		argument1 = command;
	return;
}

/*CODEWRITER FUNCTIONS*/
VMTranslator::CodeWriter::CodeWriter(string fileName) {
	string y;
	if (fileName.substr(fileName.length() - 4, 3) == ".vm") {
		y = fileName.substr(0, fileName.length() - 3) + ".asm";
		asmFile.open(y);
	}
	else {
		y = fileName.substr(0, fileName.length() - 1);
		size_t x = y.find_last_of("\\");
		y = y.erase(0, x) + ".asm";
		asmFile.open(fileName + y);
	}
	unique = 0;
}

VMTranslator::CodeWriter::~CodeWriter() {
	close();
}

// change input file (in situations where theres more than one .vm) ? filename will be made outside of this function
void VMTranslator::CodeWriter::setFileName(string fileName) {
	// fileName supplied by parser
	name = fileName.substr(0, fileName.length() - 3);
	size_t x = name.find_last_of("\\");
	name = name.erase(0, x + 1);
}

void VMTranslator::CodeWriter::updateStack(string command) {	// updates the stack pointer
	if(command == "push")
		asmFile << "@1\nD=A\n@SP\nM=M+D\n";
	else if (command == "pop")
		asmFile << "@1\nD=A\n@SP\nM=M-D\n";
}

void VMTranslator::CodeWriter::getStackData(int num) {	// grabs either the first value or the first two values on stack
	if (num == 2) {	// getting an x and y; x in RAM[5], y in D
		asmFile << "@SP\nD=M\n@2\nD=D-A\nA=D\n"; // get x's address
		asmFile << "D=M\n@5\nM=D\n";	// storing x in temp RAM[5]
		asmFile << "@SP\nD=M\n@1\nD=D-A\nA=D\nD=M\n"; // get y
		asmFile << "@5\n";	// get to RAM[5], since x is there
	}
	if (num == 1) {
		asmFile << "@SP\nD=M\n@1\nD=D-A\nA=D\n"; // get y
	}
}

void VMTranslator::CodeWriter::writeArithmetic(string command) {
	// remember to decrement pseudo stack pointer after completing binary operations
	if (command == "add") {
		getStackData(2);
		asmFile << "D=M+D\n"; // x + y
		asmFile << "@5\nM=D\n"; // store result in RAM[5]
		for(int i = 0; i < 2; i++)
			updateStack("pop");		// pop values from stack
		asmFile << "@5\nD=M\n";	// get result again
		asmFile << "@SP\nA=M\nM=D\n";	// store new value at top of stack
		updateStack("push");	// update stack pointer
	}
	else if (command == "sub") {
		getStackData(2);
		asmFile << "D=M-D\n"; // x - y
		asmFile << "@5\nM=D\n"; // store result in RAM[5]
		for(int i = 0; i < 2; i++)
			updateStack("pop");		// pop values from stack
		asmFile << "@5\nD=M\n";	// get result again
		asmFile << "@SP\nA=M\nM=D\n";	// store new value at top of stack
		updateStack("push");	// update stack pointer
	}
	else if (command == "neg") {
		getStackData(1);
		asmFile << "M=-M\n";	// negate y
	}
	else if (command == "eq") {	// x = y?
		string notEqual;
		string next;
		// label creation
		notEqual = "eq" + to_string(unique);
		next = "eqNext" + to_string(unique);
		unique++; // get new unique number
		writeArithmetic("sub");
		updateStack("pop"); // get to subtraction result
		asmFile << "@SP\nA=M\nD=M\n";	// D = top of stack
		asmFile << "@" + notEqual + '\n';
		asmFile << "D;JNE\n";	// jump to eq<unique>: if D != 0
		asmFile << "@1\nD=A\nD=-D\n";	// they're equal
		asmFile << "@5\nM=D\n"; // store result in RAM[5]
		asmFile << "@" + next + "\n0;JMP\n";
		asmFile << '(' + notEqual + ')' + '\n';
		asmFile << "@0\nD=A\n";		// they're not equal
		asmFile << "@5\nM=D\n"; // store result in RAM[5]
		asmFile << '(' + next + ')' + '\n';
		asmFile << "@5\nD=M\n";	// get result again
		asmFile << "@SP\nA=M\nM=D\n";	// store new value at top of stack
		updateStack("push");	// update stack pointer
	}
	else if (command == "gt"){	// x > y?
		string notEqual;
		string next;
		// label creation
		notEqual = "eq" + to_string(unique);
		next = "eqNext" + to_string(unique);
		unique++; // get new unique number
		writeArithmetic("sub");
		updateStack("pop"); // get to subtraction result
		asmFile << "@SP\nA=M\nD=M\n";	// D = top of stack
		asmFile << "@" + notEqual + '\n';
		asmFile << "D;JLE\n";	// jump to eq<unique>: if D <= 0
		asmFile << "@1\nD=A\nD=-D\n";	// they're equal
		asmFile << "@5\nM=D\n"; // store result in RAM[5]
		asmFile << "@" + next + "\n0;JMP\n";
		asmFile << '(' + notEqual + ')' + '\n';
		asmFile << "@0\nD=A\n";		// they're not equal
		asmFile << "@5\nM=D\n"; // store result in RAM[5]
		asmFile << '(' + next + ')' + '\n';
		asmFile << "@5\nD=M\n";	// get result again
		asmFile << "@SP\nA=M\nM=D\n";	// store new value at top of stack
		updateStack("push");	// update stack pointer
	}
	else if (command == "lt") {	// x < y?
		string notEqual;
		string next;
		// label creation
		notEqual = "eq" + to_string(unique);
		next = "eqNext" + to_string(unique);
		unique++; // get new unique number
		writeArithmetic("sub");
		updateStack("pop"); // get to subtraction result
		asmFile << "@SP\nA=M\nD=M\n";	// D = top of stack
		asmFile << "@" + notEqual + '\n';
		asmFile << "D;JGE\n";	// jump to eq<unique>: if D >= 0
		asmFile << "@1\nD=A\nD=-D\n";	// they're equal
		asmFile << "@5\nM=D\n"; // store result in RAM[5]
		asmFile << "@" + next + "\n0;JMP\n";
		asmFile << '(' + notEqual + ')' + '\n';
		asmFile << "@0\nD=A\n";		// they're not equal
		asmFile << "@5\nM=D\n"; // store result in RAM[5]
		asmFile << '(' + next + ')' + '\n';
		asmFile << "@5\nD=M\n";	// get result again
		asmFile << "@SP\nA=M\nM=D\n";	// store new value at top of stack
		updateStack("push");	// update stack pointer
	}
	else if (command == "and") {
		getStackData(2);
		asmFile << "D=D&M\n"; // x&y
		asmFile << "@5\nM=D\n"; // store result in RAM[5]
		for (int i = 0; i < 2; i++)
			updateStack("pop");		// pop values from stack
		asmFile << "@5\nD=M\n";	// get result again
		asmFile << "@SP\nA=M\nM=D\n";	// store new value at top of stack
		updateStack("push");	// update stack pointer
	}
	else if (command == "or") {
		getStackData(2);
		asmFile << "D=D|M\n"; // x|y
		asmFile << "@5\nM=D\n"; // store result in RAM[5]
		for (int i = 0; i < 2; i++)
			updateStack("pop");		// pop values from stack
		asmFile << "@5\nD=M\n";	// get result again
		asmFile << "@SP\nA=M\nM=D\n";	// store new value at top of stack
		updateStack("push");	// update stack pointer
	}
	else if (command == "not") {	// command is not
		getStackData(1);
		asmFile << "M=!M\n";	// negate y
	}
}
void VMTranslator::CodeWriter::writePushPop(string command, string segment, int index) {
	string op;	// tell updateStack to push or pop
	if (command == "push") {
		op = command;
		if (segment == "constant") 
			asmFile << "@" + to_string(index) + "\nD=A\n";	// get constant
		else if (segment == "local")
			asmFile << "@" + to_string(index) + "\nD=A\n@LCL\nA=M+D\nD=M\n"; // grabbing data at base + index
		else if (segment == "argument")
			asmFile << "@" + to_string(index) + "\nD=A\n@ARG\nA=M+D\nD=M\n"; // grabbing data at base + index
		else if (segment == "this")
			asmFile << "@" + to_string(index) + "\nD=A\n@THIS\nA=M+D\nD=M\n"; // grabbing data at base + index
		else if (segment == "that")
			asmFile << "@" + to_string(index) + "\nD=A\n@THAT\nA=M+D\nD=M\n"; // grabbing data at base + index
		else if (segment == "pointer")
			asmFile << "@" + to_string(POINTER + index) + "\nD=M\n"; // access pointer + index
		else if (segment == "temp")
			asmFile << "@" + to_string(TEMP + index) + "\nD=M\n"; // access pointer + index
		else if (segment == "static")
			asmFile << "@" + name + '.' + to_string(index) + "\nD=M\n";
		asmFile << "@SP\nA=M\nM=D\n";	// store in location SP points to
		updateStack("push");	// update stack pointer
	}
	else if (command == "pop") {	// use temp to store base + index
		updateStack("pop");	// update stack pointer
		if (segment == "local") {
			asmFile << "@" + to_string(index) + "\nD=A\n@LCL\nD=M+D\n";	// calculate LCL + index
			asmFile << "@5\nM=D\n";	// store in temp
			asmFile << "@SP\nA=M\nD=M\n";	// get top of stack
			asmFile << "@5\nA=M\nM=D\n";	// store in lcl + index
		}
		else if (segment == "argument") {
			asmFile << "@" + to_string(index) + "\nD=A\n@ARG\nD=M+D\n";	// calculate ARG + index
			asmFile << "@5\nM=D\n";	// store in temp
			asmFile << "@SP\nA=M\nD=M\n";	// get top of stack
			asmFile << "@5\nA=M\nM=D\n";	// store in arg + index
		}
		else if (segment == "this") {
			asmFile << "@" + to_string(index) + "\nD=A\n@THIS\nD=M+D\n";	// calculate THIS + index
			asmFile << "@5\nM=D\n";	// store in temp
			asmFile << "@SP\nA=M\nD=M\n";	// get top of stack
			asmFile << "@5\nA=M\nM=D\n";	// store in this + index
		}
		else if (segment == "that") {
			asmFile << "@" + to_string(index) + "\nD=A\n@THAT\nD=M+D\n";	// calculate THAT + index
			asmFile << "@5\nM=D\n";	// store in temp
			asmFile << "@SP\nA=M\nD=M\n";	// get top of stack
			asmFile << "@5\nA=M\nM=D\n";	// store in that + index
		}
		else if (segment == "pointer") {
			asmFile << "@SP\nA=M\nD=M\n";	// get top of stack
			asmFile << "@" + to_string(POINTER + index) + "\nM=D\n";	// go to pointer + index & store stack data
		}
		else if (segment == "temp") {
			asmFile << "@SP\nA=M\nD=M\n";	// get top of stack
			asmFile << "@" + to_string(TEMP + index) + "\nM=D\n";	// go to temp + index & store stack data
		}
		else if (segment == "static") {
			asmFile << "@SP\nA=M\nD=M\n";	// get top of stack
			asmFile << "@" + name + '.' + to_string(index) + "\nM=D\n";
		}
	}
}