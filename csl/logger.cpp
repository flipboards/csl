
#include "logger.h"

void csl::output_error(std::ostream& os, const CSLError & error, const StrReader * reader) {

    if (error.get_id() == CSLError::NONE) {
        os << "Unknown Error: " << error.what() << std::endl;
    }

    else if (error.get_id() == CSLError::SYNTAX) {
        os << "Syntax Error: " << error.what() << std::endl;
    }

    csl::output_context(os, reader);
}

void csl::output_context(std::ostream & os, const StrReader* reader) {

    size_t linepos = reader->iter() - reader->cur_line_begin();

    os << "At line " << reader->lineno() + 1 << ", pos " << linepos << std::endl;
    os << std::string(reader->cur_line_begin(), reader->cur_line_end()) << std::endl;
    os << std::string(linepos, ' ') << '^' << std::endl;
}
