
#include <cassert>
#include "../util/strmap.h"

class StrMapTest {
public:

    void test_strtmpref() {

        std::string a = "Hello!";
        const char* b = "Hello!";
        const char* c = "aaaaaaaa";

        StringTmpRef ra(a);

        assert(ra == a);
        assert(ra == b);
        assert(ra == StringTmpRef(b));
        assert(ra < StringTmpRef(c));

        assert(ra.copy() == a);
        assert(ra.substr(0, ra.length()) == a);
    }

    void test_strmap() {

        StrMap<int> map{ {"a", 1} };

        std::string b = "b";

        assert(map.find("a")->second == 1);
        assert(map.find(StringTmpRef(b)) == map.end());

        map.insert({ "b", 2 });

        assert(map.find(b)->second == 2);
    }
};