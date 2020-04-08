/*PROGRESS NOTES
4/3/2020 - 5:22PM
Got file searching and file input working. Parser able to parse arithmetic, push, pop & ignores comments/whitespaces.
Ready for CodeWriter to be implemented to support VM arithmetic command translation to Hack assembly language.

4/7/2020 - 11:18PM
Revamped entire Ch. 7 code to not be dependent on 256 as the stack pointer value.
*/

#include "vm.h"

int main() {
	// Chapter 7 Tests
	// VMTranslator V("D:\\nand2tetris\\nand2tetris\\projects\\07\\StackArithmetic\\SimpleAdd\\");
	// VMTranslator V("D:\\nand2tetris\\nand2tetris\\projects\\07\\StackArithmetic\\StackTest\\");
	// VMTranslator V("D:\\nand2tetris\\nand2tetris\\projects\\07\\MemoryAccess\\BasicTest\\");
	// VMTranslator V("D:\\nand2tetris\\nand2tetris\\projects\\07\\MemoryAccess\\PointerTest\\");
	// VMTranslator V("D:\\nand2tetris\\nand2tetris\\projects\\07\\MemoryAccess\\StaticTest\\");

	// Chapter 8 Tests
	// VMTranslator V("D:\\nand2tetris\\nand2tetris\\projects\\08\\ProgramFlow\\BasicLoop\\");
	// VMTranslator V("D:\\nand2tetris\\nand2tetris\\projects\\08\\ProgramFlow\\FibonacciSeries\\");
	V.startOutput();
	return 0;
}