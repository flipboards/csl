
#include "../util/memory.h"
#include <cassert>

class MempoolTest {
public:

    void test_pool() {
        MemoryPool pool;

        int* a = new int;
        *a = 100;
        int b = 200;

        auto ptra = pool.collect(a);
        auto ptrb = pool.assign(b);

        assert(*ptra == 100);
        assert(*ptrb == 200);

        auto ptrc = ptra.to_const();
        auto ptrd = ptra;

        assert(*ptrc == 100);
        assert(ptrc.exists());

        assert(ptrd.use_count() == 3);
    }

    void test_strpool() {
        ConstStringPool pool;

        std::string a = "hello!";

        auto ptra = pool.assign(a);

        const char* p = ptra.get();

        assert(ptra == a);
        assert(ptra.length() == a.length());

        auto ptrb = ptra;
        assert(ptrb.use_count() == 2);
        assert(ptrb.to_string() == a);
    }
};
