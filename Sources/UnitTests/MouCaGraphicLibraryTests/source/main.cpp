#include <Dependancies.h>

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    MouCaEnvironment* environment = new MouCaEnvironment;
    environment->initialize(argc, argv);
    testing::AddGlobalTestEnvironment(environment);

    return RUN_ALL_TESTS();
}