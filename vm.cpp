#include "vm.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

using namespace std;

/*VM TRANSLATOR FUNCTIONS*/
VMTranslator::VMTranslator(std::string path) { // initialize parsers here
	int vmCount = 0;
	if (fs::path(path).extension() == ".vm") {
		// open vm file
		vmFiles = new string[1];
		*(vmFiles) = fs::path(path).string();
		asmFileName = vmFiles[0].substr(0, vmFiles[0].length() - 3) + ".asm";
		vmCount = 1;
	}
	else {
		asmFileName = fs::path(path).string() + ".asm";
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
	VMTranslator::Parser p(vmFiles[0]);
	// move this somewhere else
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
	}
	return;
}

VMTranslator::~VMTranslator() {
	delete[] vmFiles;
	delete CW;
	return;
}

/*PARSER FUNCTIONS*/
VMTranslator::Parser::Parser(string filename) {
	currentCmd = "";
	argument1 = "";
	argument2 = 0;
	type = C_COMMENT;
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
	else if (command == "label")
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
		type = C_RETURN;
	else 	// expand on the other types later
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
	asmFile.open(fileName);
	name = fileName;
	size_t x = name.find_last_of("\\");
	name = name.erase(0, x + 1);
	stackSize = 0;
	unique = 0;
}

VMTranslator::CodeWriter::~CodeWriter() {
	close();
}

// change input file (in situations where theres more than one .vm) ? filename will be made outside of this function
void VMTranslator::CodeWriter::setFileName(string fileName) {

}

void VMTranslator::CodeWriter::writeArithmetic(string command) {
	string assemblyCode;
	// remember to decrement pseudo stack pointer after completing binary operations
	if (command == "add") {
		assemblyCode = "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x
		assemblyCode += "D=M\n";	// storing x in D
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 1) + '\n'; // get y
		assemblyCode += "D=D+M\n"; // x + y
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x again, to store new val
		assemblyCode += "M=D\n";	// store new value at x's stack address
		assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		stackSize--;
	}
	else if (command == "sub") {
		assemblyCode = "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x
		assemblyCode += "D=M\n";	// storing x in D
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 1) + '\n'; // get y
		assemblyCode += "D=D-M\n"; // x - y
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x again, to store new val
		assemblyCode += "M=D\n";	// store new value at x's stack address
		assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		stackSize--;
	}
	else if (command == "neg") {
		assemblyCode = "@" + to_string((STACK_ADDRESS + stackSize) - 1) + '\n'; // get y
		assemblyCode += "M=-M\n";	// negate y
	}
	else if (command == "eq") {	// x = y?
		string notEqual;
		string next;
		assemblyCode = "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x
		assemblyCode += "D=M\n";	// storing x in D
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 1) + '\n'; // get y
		assemblyCode += "D=D-M\n"; // x - y
		// label creation
		notEqual = "eq" + to_string(unique);
		next = "eqNext" + to_string(unique);
		assemblyCode += "@" + notEqual + '\n';
		unique++; // get new unique number
		assemblyCode += "D;JNE\n";	// jump to eq<unique>: if D != 0
		assemblyCode += "@1\nD=A\nD=-D\n";	// they're equal
		assemblyCode += "@" + next + "\n0;JMP\n";
		assemblyCode += '(' + notEqual + ')' + '\n';
		assemblyCode += "@0\nD=A\n";		// they're not equal
		assemblyCode += '(' + next + ')' + '\n';
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x again
		assemblyCode += "M=D\n";
		assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		stackSize--;
	}
	else if (command == "gt"){	// x > y?
		string notGt;
		string next;
		assemblyCode = "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x
		assemblyCode += "D=M\n";	// storing x in D
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 1) + '\n'; // get y
		assemblyCode += "D=D-M\n"; // x - y
		// label creation
		notGt = "gt" + to_string(unique);
		next = "gtNext" + to_string(unique);
		assemblyCode += "@" + notGt + '\n';
		unique++; // get new unique number
		assemblyCode += "D;JLE\n";	// jump to gt<unique>: if D <= 0
		assemblyCode += "@1\nD=A\nD=-D\n";	// x > y
		assemblyCode += "@" + next + "\n0;JMP\n";
		assemblyCode += '(' + notGt + ')' + '\n';
		assemblyCode += "@0\nD=A\n";		// x < y
		assemblyCode += '(' + next + ')' + '\n';
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x again
		assemblyCode += "M=D\n";
		assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		stackSize--;
	}
	else if (command == "lt") {	// x < y?
		string notLt;
		string next;
		assemblyCode = "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x
		assemblyCode += "D=M\n";	// storing x in D
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 1) + '\n'; // get y
		assemblyCode += "D=D-M\n"; // x - y
		// label creation
		notLt = "lt" + to_string(unique);
		next = "ltNext" + to_string(unique);
		assemblyCode += "@" + notLt + '\n';
		unique++; // get new unique number
		assemblyCode += "D;JGE\n";	// jump to gt<unique>: if D >= 0
		assemblyCode += "@1\nD=A\nD=-D\n";	// x < y
		assemblyCode += "@" + next + "\n0;JMP\n";
		assemblyCode += '(' + notLt + ')' + '\n';
		assemblyCode += "@0\nD=A\n";		// x > y
		assemblyCode += '(' + next + ')' + '\n';
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x again
		assemblyCode += "M=D\n";
		assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		stackSize--;
	}
	else if (command == "and") {
		assemblyCode = "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x
		assemblyCode += "D=M\n";	// storing x in D
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 1) + '\n'; // get y
		assemblyCode += "D=D&M\n"; // x&y
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x again, to store new val
		assemblyCode += "M=D\n";	// store new value at x's stack address
		assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		stackSize--;
	}
	else if (command == "or") {
		assemblyCode = "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x
		assemblyCode += "D=M\n";	// storing x in D
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 1) + '\n'; // get y
		assemblyCode += "D=D|M\n"; // x|y
		assemblyCode += "@" + to_string((STACK_ADDRESS + stackSize) - 2) + '\n'; // get x again, to store new val
		assemblyCode += "M=D\n";	// store new value at x's stack address
		assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		stackSize--;
	}
	else if (command == "not") {	// command is not
		assemblyCode = "@" + to_string((STACK_ADDRESS + stackSize) - 1) + '\n'; // get y
		assemblyCode += "M=!M\n";	// negate y
	}
	// write to output file
	asmFile << assemblyCode;
}
void VMTranslator::CodeWriter::writePushPop(string command, string segment, int index) {
	string assemblyCode;
	int address;
	if (command == "push") {
		address = STACK_ADDRESS + stackSize;
		if (segment == "constant") {
			assemblyCode = "@" + to_string(index) + '\n';	// get constant
			assemblyCode += "D=A\n";
			assemblyCode += "@" + to_string(address) + "\n";
			assemblyCode += "M=D\n";
			assemblyCode += "@1\nD=A\n@SP\nM=M+D\n";	// update stack pointer
		}
		else if (segment == "local") {
			assemblyCode = "@" + to_string(index) + "\nD=A\n@LCL\nA=M+D\nD=M\n"; // grabbing data at base + index
			assemblyCode += "@" + to_string(address) + "\nM=D\n";	// pushing to stack
			assemblyCode += "@1\nD=A\n@SP\nM=M+D\n";	// update stack pointer
		}
		else if (segment == "argument") {
			assemblyCode = "@" + to_string(index) + "\nD=A\n@ARG\nA=M+D\nD=M\n"; // grabbing data at base + index
			assemblyCode += "@" + to_string(address) + "\nM=D\n";	// pushing to stack
			assemblyCode += "@1\nD=A\n@SP\nM=M+D\n";	// update stack pointer
		}
		else if (segment == "this") {
			assemblyCode = "@" + to_string(index) + "\nD=A\n@THIS\nA=M+D\nD=M\n"; // grabbing data at base + index
			assemblyCode += "@" + to_string(address) + "\nM=D\n";	// pushing to stack
			assemblyCode += "@1\nD=A\n@SP\nM=M+D\n";	// update stack pointer
		}
		else if (segment == "that") {
			assemblyCode = "@" + to_string(index) + "\nD=A\n@THAT\nA=M+D\nD=M\n"; // grabbing data at base + index
			assemblyCode += "@" + to_string(address) + "\nM=D\n";	// pushing to stack
			assemblyCode += "@1\nD=A\n@SP\nM=M+D\n";	// update stack pointer
		}
		else if (segment == "pointer") {
			assemblyCode = "@" + to_string(POINTER + index) + "\nD=M\n"; // access pointer + index
			assemblyCode += "@" + to_string(address) + "\nM=D\n";	// pushing to stack
			assemblyCode += "@1\nD=A\n@SP\nM=M+D\n";	// update stack pointer
		}
		else if (segment == "temp") {
			assemblyCode = "@" + to_string(TEMP + index) + "\nD=M\n"; // access pointer + index
			assemblyCode += "@" + to_string(address) + "\nM=D\n";	// pushing to stack
			assemblyCode += "@1\nD=A\n@SP\nM=M+D\n";	// update stack pointer
		}
		else if (segment == "static") {
			assemblyCode = "@" + name + '.' + to_string(index) + "\nD=M\n";
			assemblyCode += "@" + to_string(address) + "\nM=D\n";	// pushing to stack
			assemblyCode += "@1\nD=A\n@SP\nM=M+D\n";	// update stack pointer
		}
		stackSize++;
	}
	else if (command == "pop") {	// use temp to store base + index
		// stack to data segment
		stackSize--;
		address = STACK_ADDRESS + stackSize;
		if (segment == "local") {
			assemblyCode = "@" + to_string(index) + "\nD=A\n@LCL\nD=M+D\n";	// calculate LCL + index
			assemblyCode += "@5\nM=D\n";	// store in temp
			assemblyCode += "@" + to_string(address) + "\nD=M\n";	// get stack data of interest
			assemblyCode += "@5\nA=M\nM=D\n";	// store in lcl + index
			assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		}
		else if (segment == "argument") {
			assemblyCode = "@" + to_string(index) + "\nD=A\n@ARG\nD=M+D\n";	// calculate ARG + index
			assemblyCode += "@5\nM=D\n";	// store in temp
			assemblyCode += "@" + to_string(address) + "\nD=M\n";	// get stack data of interest
			assemblyCode += "@5\nA=M\nM=D\n";	// store in lcl + index
			assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		}
		else if (segment == "this") {
			assemblyCode = "@" + to_string(index) + "\nD=A\n@THIS\nD=M+D\n";	// calculate THIS + index
			assemblyCode += "@5\nM=D\n";	// store in temp
			assemblyCode += "@" + to_string(address) + "\nD=M\n";	// get stack data of interest
			assemblyCode += "@5\nA=M\nM=D\n";	// store in lcl + index
			assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		}
		else if (segment == "that") {
			assemblyCode = "@" + to_string(index) + "\nD=A\n@THAT\nD=M+D\n";	// calculate THAT + index
			assemblyCode += "@5\nM=D\n";	// store in temp
			assemblyCode += "@" + to_string(address) + "\nD=M\n";	// get stack data of interest
			assemblyCode += "@5\nA=M\nM=D\n";	// store in lcl + index
			assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		}
		else if (segment == "pointer") {
			assemblyCode = "@" + to_string(address) + "\nD=M\n";	// get stack data of interest
			assemblyCode += "@" + to_string(POINTER + index) + "\nM=D\n";	// go to pointer + index & store stack data
			assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		}
		else if (segment == "temp") {
			assemblyCode = "@" + to_string(address) + "\nD=M\n";	// get stack data of interest
			assemblyCode += "@" + to_string(TEMP + index) + "\nM=D\n";	// go to temp + index & store stack data
			assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		}
		else if (segment == "static") {
			assemblyCode = "@" + to_string(address) + "\nD=M\n";	// get stack data of interest
			assemblyCode += "@" + name + '.' + to_string(index) + "\nM=D\n";
			assemblyCode += "@1\nD=A\n@SP\nM=M-D\n";	// update stack pointer
		}
	}
	// write to output file
	asmFile << assemblyCode;
}