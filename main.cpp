/*PROGRESS NOTES
4/3/2020 - 5:22PM
Got file searching and file input working. Parser able to parse arithmetic, push, pop & ignores comments/whitespaces.
Ready for CodeWriter to be implemented to support VM arithmetic command translation to Hack assembly language.
*/

#include "vm.h"

int main() {
	// VMTranslator V("D:\\nand2tetris\\nand2tetris\\projects\\07\\StackArithmetic\\SimpleAdd");
	// VMTranslator V("D:\\nand2tetris\\nand2tetris\\projects\\07\\StackArithmetic\\StackTest\\StackTest.vm");
	// VMTranslator V("D:\\nand2tetris\\nand2tetris\\projects\\07\\MemoryAccess\\BasicTest\\BasicTest.vm");
	// VMTranslator V("D:\\nand2tetris\\nand2tetris\\projects\\07\\MemoryAccess\\PointerTest\\PointerTest.vm");
	VMTranslator V("D:\\nand2tetris\\nand2tetris\\projects\\07\\MemoryAccess\\StaticTest\\StaticTest.vm");
	return 0;
}