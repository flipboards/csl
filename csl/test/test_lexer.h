#include "../lexer.h"

class LexerTest {
public:

	LexerTest() {

	}

	void run() {
		lexer.clear();
		std::string sample = "_1a +-3.5 4e-7 \"1+2\"\t '3' ;";
		StrReader reader(sample);
		lexer.load(&reader);

				
		
	}

private:

	Lexer lexer;
};