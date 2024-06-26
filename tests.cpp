#include "Base.cpp"
#include "PtrArray.cpp"

static void test()
{
    // Test default constructor
    PtrArray<Base> arr;
    assert(arr.empty());
    assert(arr.empty());

    // Test initializer list constructor
    PtrArray<Base> arr2;
    arr2.emplace_back(new Derived1(1));
    arr2.emplace_back(new Derived1(2));
    arr2.emplace_back(new Derived1(3));
    assert(arr2.size() == 3);
    assert(arr2[0]->getValue() == 1);
    assert(arr2[1]->getValue() == 2);
    assert(arr2[2]->getValue() == 3);

    // Test copy constructor
    PtrArray<Base> arr3 = arr2;
    assert(arr3.size() == 3);
    assert(arr3[0]->getValue() == 1);
    assert(arr3[1]->getValue() == 2);
    assert(arr3[2]->getValue() == 3);

    // Test move constructor
    PtrArray<Base> arr4 = std::move(arr3);
    assert(arr4.size() == 3);
    assert(arr4[0]->getValue() == 1);
    assert(arr4[1]->getValue() == 2);
    assert(arr4[2]->getValue() == 3);
    assert(arr3.size() == 0);

    // Test emplace
    arr.emplace(arr.begin(), new Derived1(4));
    assert(arr.size() == 1);
    assert(arr[0]->getValue() == 4);

    // Test emplace_back
    arr.emplace_back(new Derived1(5));
    assert(arr.size() == 2);
    assert(arr[1]->getValue() == 5);

    // Test erase
    arr.erase(arr.begin());
    assert(arr.size() == 1);
    assert(arr[0]->getValue() == 5);

    // Test clear
    arr.clear();
    assert(arr.size() == 0);
    assert(arr.empty());

    // Test at
    arr.emplace_back(new Derived1(6));
    try {
        arr.at(0).getValue();
    }
    catch (...) {
        assert(false); // Should not throw
    }

    try {
        arr.at(1).getValue();
        assert(false); // Should throw
    }
    catch (...) {
        // Expected to throw
    }

    // Test subscript operator
    assert(arr[0]->getValue() == 6);

    // Test assignment operator
    PtrArray<Base> arr5;
    arr5 = arr;
    assert(arr5.size() == 1);
    assert(arr5[0]->getValue() == 6);

    // Test move assignment operator
    PtrArray<Base> arr6;
    arr6 = std::move(arr5);
    assert(arr6.size() == 1);
    assert(arr6[0]->getValue() == 6);
    assert(arr5.size() == 0);

    // Test begin and end
    int sum = 0;
    for (auto it = arr6.begin(); it != arr6.end(); ++it) {
        sum += it->getValue();
    }
    assert(sum == 6);

    // More complex operations
    arr6.clear();
    for (int i = 0; i < 1000; ++i) {
        if (i % 2)
            arr6.emplace_back(new Derived1(i));
        else
            arr6.emplace_back(new Derived2(i));

    }
    assert(arr6.size() == 1000);
    assert(arr6[500]->getValue() == 500);

    // Test large number of erases
    for (int i = 0; i < 500; ++i) {
        arr6.erase(arr6.begin());
    }
    assert(arr6.size() == 500);
    assert(arr6[0]->getValue() == 500);

    // Test reallocation
    PtrArray<Base> arr7;
    for (int i = 0; i < 100; ++i) {
        arr7.emplace_back(new Derived2(i));
    }
    assert(arr7.size() == 100);
    assert(arr7[99]->getValue() == 99);

    std::cout << "All tests passed!" << std::endl;
}

static void test_stl()
{
    PtrArray<Base> arr;
    arr.emplace_back(new Derived1(1));
    arr.emplace_back(new Derived1(3));
    arr.emplace_back(new Derived1(2));
    arr.emplace_back(new Derived1(5));
    arr.emplace_back(new Derived1(4));

    // Test std::accumulate
    auto sum = std::accumulate(arr.begin(), arr.end(), 0, [](int acc, Base const* obj) {
        return acc + obj->getValue();
        });
    assert(sum == 15);

    // Test std::count_if
    auto count_gt_2 = std::count_if(arr.begin(), arr.end(), [](Base const* obj) {
        return obj->getValue() > 2;
        });
    assert(count_gt_2 == 3);

    // Test std::find
    auto find_it = std::find_if(arr.begin(), arr.end(), [](Base const* obj) {
        return obj->getValue() == 3;
        });
    assert(find_it != arr.end());

    // Test std::find_if_not
    auto find_if_not_it = std::find_if_not(arr.begin(), arr.end(), [](Base const* obj) {
        return obj->getValue() < 3;
        });
    assert(find_if_not_it != arr.end());

    // Test std::transform
    std::vector<int> transformed(arr.size());
    std::transform(arr.begin(), arr.end(), transformed.begin(), [](Base const* obj) {
        return obj->getValue() * 2;
        });

    assert(transformed[0] == 2 && transformed[1] == 6 && transformed[2] == 4 && transformed[3] == 10 && transformed[4] == 8);

    // Test std::generate
    int value = 1;
    std::generate(arr.begin(), arr.end(), [&value]() -> Base* {
        if (value % 2 == 0) {
            return new Derived1(value++);
        }
        else {
            return new Derived2(value++);
        }
        });
    assert(arr[0]->getValue() == 1 && arr[1]->getValue() == 2 && arr[2]->getValue() == 3 && arr[3]->getValue() == 4 && arr[4]->getValue() == 5);

    // Test std::shuffle
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(arr.begin(), arr.end(), g);
    // Cannot assert specific order due to shuffling

    //Test std::all_of
    bool all_gt_0 = std::all_of(arr.begin(), arr.end(), [](Base const* obj) {
        return obj->getValue() > 0;
        });
    assert(all_gt_0);

    // Test std::any_of
    bool any_eq_2 = std::any_of(arr.begin(), arr.end(), [](Base const* obj) {
        return obj->getValue() == 2;
        });
    assert(any_eq_2);

    // Test std::none_of
    bool none_eq_0 = std::none_of(arr.begin(), arr.end(), [](Base const* obj) {
        return obj->getValue() == 0;
        });
    assert(none_eq_0);

    // Test std::replace_if
    auto it = std::find_if(arr.begin(), arr.end(), [](Base* obj) {return obj->getValue() == 5; });
    std::replace_if(arr.begin(), arr.end(), [](Base* obj) {
        return obj->getValue() == 5;
        }, new Derived1(6));

    assert(it->getValue() == 6);

    //// Test std::sort
    //std::sort(arr.begin(), arr.end(), [](Base* a, Base* b) {
    //    return a->getValue() < b->getValue();
    //    });
    //assert(arr[0].getValue() == 1);
    //assert(arr[1].getValue() == 2);
    //assert(arr[2].getValue() == 3);
    //assert(arr[3].getValue() == 4);
    //assert(arr[4].getValue() == 6);

    // Test std::max_element
    auto max_it = std::max_element(arr.begin(), arr.end(), [](Base const* a, Base const* b) {
        return a->getValue() < b->getValue();
        });
    assert(max_it != arr.end() && max_it->getValue() == 6);

    // Test std::min_element
    auto min_it = std::min_element(arr.begin(), arr.end(), [](Base const* a, Base const* b) {
        return a->getValue() < b->getValue();
        });
    assert(min_it != arr.end() && min_it->getValue() == 1);

    // Test std::reverse
    std::reverse(arr.begin(), arr.end());

    std::cout << "All tests passed!" << std::endl;
}

static void test_copy_constructor() {
    PtrArray<Base> arr;
    arr.emplace_back(new Derived1(3));
    arr.emplace_back(new Derived2(1));

    PtrArray<Base> copied_arr = arr;  // Use copy constructor

    assert(copied_arr.size() == arr.size());
    assert(copied_arr[0]->getValue() == arr[0]->getValue());
    assert(copied_arr[1]->getValue() == arr[1]->getValue());
}

static void test_move_constructor() {
    PtrArray<Base> arr;
    arr.emplace_back(new Derived1(3));
    arr.emplace_back(new Derived2(1));

    PtrArray<Base> moved_arr = std::move(arr);  // Use move constructor

    assert(moved_arr.size() == 2);
    assert(moved_arr[0]->getValue() == 3);
    assert(moved_arr[1]->getValue() == 1);
    assert(arr.size() == 0);  // Original array should be empty
}

static void test_copy_assignment_operator() {
    PtrArray<Base> arr;
    arr.emplace_back(new Derived1(3));
    arr.emplace_back(new Derived2(1));

    PtrArray<Base> copied_arr;
    copied_arr = arr;  // Use copy assignment operator

    assert(copied_arr.size() == arr.size());
    assert(copied_arr[0]->getValue() == arr[0]->getValue());
    assert(copied_arr[1]->getValue() == arr[1]->getValue());
}

static void test_move_assignment_operator() {
    PtrArray<Base> arr;
    arr.emplace_back(new Derived1(3));
    arr.emplace_back(new Derived2(1));

    PtrArray<Base> moved_arr;
    moved_arr = std::move(arr);  // Use move assignment operator

    assert(moved_arr.size() == 2);
    assert(moved_arr[0]->getValue() == 3);
    assert(moved_arr[1]->getValue() == 1);
    assert(arr.size() == 0);  // Original array should be empty
}

static void test_std_sort() {
    PtrArray<Base> arr;
    arr.emplace_back(new Derived1(3));
    arr.emplace_back(new Derived2(1));
    arr.emplace_back(new Derived1(2));

    std::sort(arr.begin(), arr.end(), [](Base const* a, Base const* b) {
        return a->getValue() < b->getValue();
        });

    assert(arr[0]->getValue() == 1);
    assert(arr[1]->getValue() == 2);
    assert(arr[2]->getValue() == 3);
}

static void test_std_min_element() {
    PtrArray<Base> arr;
    arr.emplace_back(new Derived1(3));
    arr.emplace_back(new Derived2(1));
    arr.emplace_back(new Derived1(2));

    auto min_it = std::min_element(arr.begin(), arr.end(), [](Base const* a, Base const* b) {
        return a->getValue() < b->getValue();
        });

    assert(min_it != arr.end());
    assert(min_it->getValue() == 1);
}

static void test_clear() {
    PtrArray<Base> arr;
    arr.emplace_back(new Derived1(3));
    arr.emplace_back(new Derived2(1));
    arr.emplace_back(new Derived1(2));

    arr.clear();

    assert(arr.size() == 0);
    assert(arr.empty());
}

static void test_erase() {
    PtrArray<Base> arr;
    arr.emplace_back(new Derived1(3));
    arr.emplace_back(new Derived2(1));
    arr.emplace_back(new Derived1(2));

    arr.erase(arr.begin() + 1);  // Erase second element

    assert(arr.size() == 2);
    assert(arr[0]->getValue() == 3);
    assert(arr[1]->getValue() == 2);

    for (auto& it : arr)
        std::cout << *it;
}