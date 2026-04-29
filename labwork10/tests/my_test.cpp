#include <lib/interpreter.h>
#include <gtest/gtest.h>


TEST(ListOpsSuite, PushPopInsertRemoveTest) {
    std::string code = R"(
        lst = [1, 2, 4]
        push(lst, 5)          // [1,2,4,5]
        insert(lst, 2, 3)     // [1,2,3,4,5]
        pop(lst)              // [1,2,3,4]
        remove(lst, 3)        // -> 4 ; [1,2,3]
        print(len(lst))       // 3
        print(lst[0])         // 1
        print(lst[1])         // 2
        print(lst[2])         // 3
    )";

    std::string expected = "3123";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(StringOpsSuite, LowerUpperSplitJoinReplace) {
    std::string code = R"(
        s = "Hello World"
        print(lower(s))           // "hello world"
        print(upper(s))           // "HELLO WORLD"

        x = "a-b-c"
        lst = split(x, "-")       // ["a","b","c"]
        print(len(lst))           // 3
        y = join(lst, "+")        // "a+b+c"
        z = replace(y, "+", "_")  // "a_b_c"
        print(y)
        print(z)
    )";

    std::string expected = "\"hello world\"\"HELLO WORLD\"3a+b+ca_b_c";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(SliceSuite, StringAndListSlices) {
    std::string code = R"(
        s = "abcdef"
        print(s[1:4])             // "bcd"
        print(s[-2])              // "e"

        lst = [1,2,3,4,5]
        print(len(lst[1:4]))      // 3
        print(lst[-1])            // 5
    )";

    std::string expected = "bcde35";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(AssignOpsSuite, PlusMinusMulDivAssign) {
    std::string code = R"(
        x = 5
        x += 3     // 8
        print(x)
        x *= 2     // 16
        print(x)
        x -= 4     // 12
        print(x)
        x /= 2     // 6
        print(x)
    )";

    std::string expected = "816126";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(LoopControlSuite, BreakAndContinueInFor) {
    std::string code = R"(
        for i in range(0,10,1)
            if i == 3 then
                continue          // пропускаем 3
            end if
            if i == 6 then
                break             // выходим на 6
            end if
            print(i)
        end for
    )";

    std::string expected = "01245";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(BuiltinNumericSuite, AbsCeilFloorRoundSqrt) {
    std::string code = R"(
        print(abs(-5))      // 5
        print(ceil(2.3))    // 3
        print(floor(2.9))   // 2
        print(round(2.6))   // 3
        print(sqrt(9))      // 3
    )";

    std::string expected = "53233";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}