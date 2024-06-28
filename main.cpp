#include "PtrArray.cpp"
#include "tests.cpp"

int main() 
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    {
        test();
        test_copy_constructor();
        test_move_constructor();
        test_copy_assignment_operator();
        test_move_assignment_operator();
        test_std_min_element();
        test_clear();
        test_erase();
        test_stl();
        test_std_sort();
    }
    _CrtDumpMemoryLeaks();
}
