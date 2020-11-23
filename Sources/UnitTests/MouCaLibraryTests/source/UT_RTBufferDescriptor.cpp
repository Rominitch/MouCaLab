#include "Dependancies.h"

#include <LibRT/include/RTBufferDescriptor.h>

namespace RT
{

TEST(RTBufferDescriptor, checkAPI)
{
    BufferDescriptor descriptor;

    EXPECT_EQ(0, descriptor.getByteSize());
    EXPECT_EQ(0, descriptor.getNbDescriptors());

    // Add new component
    {
        ComponentDescriptor component(3, RT::Type::Float, RT::ComponentUsage::Vertex);
        ASSERT_NO_THROW(descriptor.addDescriptor(component));

        EXPECT_EQ(3*sizeof(float), descriptor.getByteSize());
        EXPECT_EQ(1, descriptor.getNbDescriptors());

        EXPECT_TRUE(descriptor.hasComponentUsage(RT::ComponentUsage::Vertex));
        EXPECT_FALSE(descriptor.hasComponentUsage(RT::ComponentUsage::Color));
        EXPECT_FALSE(descriptor.hasComponentUsage(RT::ComponentUsage::BonesIds));
    }

    // Add another component
    {
        ComponentDescriptor component(2, RT::Type::Int, RT::ComponentUsage::BonesIds);
        ASSERT_NO_THROW(descriptor.addDescriptor(component));

        EXPECT_EQ(3 * sizeof(float) + 2 * sizeof(int), descriptor.getByteSize());
        EXPECT_EQ(2, descriptor.getNbDescriptors());

        EXPECT_TRUE( descriptor.hasComponentUsage(RT::ComponentUsage::Vertex));
        EXPECT_FALSE(descriptor.hasComponentUsage(RT::ComponentUsage::Color));
        EXPECT_TRUE( descriptor.hasComponentUsage(RT::ComponentUsage::BonesIds));
    }
}

}