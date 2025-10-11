#include <iostream>
#include <gtest/gtest.h>

void BasicTest();

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
