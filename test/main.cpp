#include <catch2/catch_session.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);

    int result = Catch::Session().run(argc, argv);
    return result;
}
