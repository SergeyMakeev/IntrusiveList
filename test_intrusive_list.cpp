#include "intrusive_list/intrusive_list.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <vector>

// Test objects for intrusive lists
struct TestObject
{
    int value;
    dod::list_node link;

    explicit TestObject(int v)
        : value(v)
    {
    }

    // Make objects comparable for testing
    bool operator==(const TestObject& other) const { return value == other.value; }
};

// Test object that can be in multiple lists
struct MultiListObject
{
    int id;
    dod::list_node list1_link;
    dod::list_node list2_link;

    explicit MultiListObject(int i)
        : id(i)
    {
    }
};

// Test object with custom destructor for testing automatic unlinking
struct DestructorTestObject
{
    int value;
    dod::list_node link;
    static int destructor_count;

    explicit DestructorTestObject(int v)
        : value(v)
    {
    }

    ~DestructorTestObject() { ++destructor_count; }
};

int DestructorTestObject::destructor_count = 0;

class IntrusiveListTest : public ::testing::Test
{
  protected:
    void SetUp() override { DestructorTestObject::destructor_count = 0; }
};

// Basic construction and destruction tests
TEST_F(IntrusiveListTest, DefaultConstruction)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.begin(), list.end());
}

TEST_F(IntrusiveListTest, PushBackSingleElement)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj(42);

    EXPECT_TRUE(TestList::can_insert(obj));

    list.push_back(obj);

    EXPECT_FALSE(list.empty());
    EXPECT_NE(list.begin(), list.end());
    EXPECT_EQ(list.front().value, 42);
    EXPECT_EQ(list.back().value, 42);
    EXPECT_FALSE(TestList::can_insert(obj));
}

TEST_F(IntrusiveListTest, PushFrontSingleElement)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj(84);

    list.push_front(obj);

    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.front().value, 84);
    EXPECT_EQ(list.back().value, 84);
}

TEST_F(IntrusiveListTest, PushMultipleElements)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);
    TestObject obj3(3);

    list.push_back(obj1);
    list.push_back(obj2);
    list.push_front(obj3);

    EXPECT_EQ(list.front().value, 3);
    EXPECT_EQ(list.back().value, 2);

    // Check order: 3, 1, 2
    auto it = list.begin();
    EXPECT_EQ(it->value, 3);
    ++it;
    EXPECT_EQ(it->value, 1);
    ++it;
    EXPECT_EQ(it->value, 2);
    ++it;
    EXPECT_EQ(it, list.end());
}

TEST_F(IntrusiveListTest, PopOperations)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);
    TestObject obj3(3);

    list.push_back(obj1);
    list.push_back(obj2);
    list.push_back(obj3);

    // List: 1, 2, 3
    EXPECT_EQ(list.front().value, 1);
    EXPECT_EQ(list.back().value, 3);

    list.pop_front();
    // List: 2, 3
    EXPECT_EQ(list.front().value, 2);
    EXPECT_EQ(list.back().value, 3);
    EXPECT_TRUE(TestList::can_insert(obj1));

    list.pop_back();
    // List: 2
    EXPECT_EQ(list.front().value, 2);
    EXPECT_EQ(list.back().value, 2);
    EXPECT_TRUE(TestList::can_insert(obj3));

    list.pop_front();
    // List: empty
    EXPECT_TRUE(list.empty());
    EXPECT_TRUE(TestList::can_insert(obj2));
}

TEST_F(IntrusiveListTest, IteratorOperations)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(10);
    TestObject obj2(20);
    TestObject obj3(30);

    list.push_back(obj1);
    list.push_back(obj2);
    list.push_back(obj3);

    // Forward iteration
    std::vector<int> values;
    for (const auto& obj : list)
    {
        values.push_back(obj.value);
    }
    EXPECT_EQ(values, (std::vector<int>{10, 20, 30}));

    // Manual iterator operations
    auto it = list.begin();
    EXPECT_EQ(it->value, 10);

    ++it;
    EXPECT_EQ(it->value, 20);

    --it;
    EXPECT_EQ(it->value, 10);

    // Post-increment/decrement
    auto it2 = it++;
    EXPECT_EQ(it2->value, 10);
    EXPECT_EQ(it->value, 20);

    auto it3 = it--;
    EXPECT_EQ(it3->value, 20);
    EXPECT_EQ(it->value, 10);
}

TEST_F(IntrusiveListTest, ConstIterators)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(100);
    TestObject obj2(200);

    list.push_back(obj1);
    list.push_back(obj2);

    const auto& const_list = list;

    std::vector<int> values;
    for (auto it = const_list.cbegin(); it != const_list.cend(); ++it)
    {
        values.push_back(it->value);
    }
    EXPECT_EQ(values, (std::vector<int>{100, 200}));
}

TEST_F(IntrusiveListTest, EraseOperations)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);
    TestObject obj3(3);
    TestObject obj4(4);

    list.push_back(obj1);
    list.push_back(obj2);
    list.push_back(obj3);
    list.push_back(obj4);

    // Erase by iterator
    auto it = list.begin();
    ++it; // Point to obj2
    auto next_it = list.erase(it);
    EXPECT_EQ(next_it->value, 3); // Should point to obj3
    EXPECT_TRUE(TestList::can_insert(obj2));

    // Erase by object reference
    list.erase(obj4);
    EXPECT_TRUE(TestList::can_insert(obj4));

    // Verify remaining elements: 1, 3
    std::vector<int> values;
    for (const auto& obj : list)
    {
        values.push_back(obj.value);
    }
    EXPECT_EQ(values, (std::vector<int>{1, 3}));
}

TEST_F(IntrusiveListTest, InsertOperation)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(10);
    TestObject obj2(20);
    TestObject obj3(15);

    list.push_back(obj1);
    list.push_back(obj2);

    // Insert obj3 between obj1 and obj2
    auto it = list.begin();
    ++it; // Point to obj2
    auto inserted_it = list.insert(it, obj3);

    EXPECT_EQ(inserted_it->value, 15);

    // Verify order: 10, 15, 20
    std::vector<int> values;
    for (const auto& obj : list)
    {
        values.push_back(obj.value);
    }
    EXPECT_EQ(values, (std::vector<int>{10, 15, 20}));
}

TEST_F(IntrusiveListTest, ClearOperation)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);
    TestObject obj3(3);

    list.push_back(obj1);
    list.push_back(obj2);
    list.push_back(obj3);

    EXPECT_FALSE(list.empty());

    list.clear();

    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.begin(), list.end());

    // All objects should be unlinked
    EXPECT_TRUE(TestList::can_insert(obj1));
    EXPECT_TRUE(TestList::can_insert(obj2));
    EXPECT_TRUE(TestList::can_insert(obj3));
}

TEST_F(IntrusiveListTest, MoveConstructor)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list1;
    TestObject obj1(42);
    TestObject obj2(84);

    list1.push_back(obj1);
    list1.push_back(obj2);

    auto list2 = std::move(list1);

    // list1 should be empty
    EXPECT_TRUE(list1.empty());

    // list2 should contain the objects
    EXPECT_FALSE(list2.empty());
    std::vector<int> values;
    for (const auto& obj : list2)
    {
        values.push_back(obj.value);
    }
    EXPECT_EQ(values, (std::vector<int>{42, 84}));
}

TEST_F(IntrusiveListTest, MoveAssignment)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list1, list2;
    TestObject obj1(1);
    TestObject obj2(2);
    TestObject obj3(3);

    list1.push_back(obj1);
    list1.push_back(obj2);
    list2.push_back(obj3);

    list2 = std::move(list1);

    // list1 should be empty
    EXPECT_TRUE(list1.empty());

    // list2 should contain obj1 and obj2 (obj3 should be unlinked)
    std::vector<int> values;
    for (const auto& obj : list2)
    {
        values.push_back(obj.value);
    }
    EXPECT_EQ(values, (std::vector<int>{1, 2}));
    EXPECT_TRUE(TestList::can_insert(obj3));
}

TEST_F(IntrusiveListTest, SwapOperation)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list1, list2;
    TestObject obj1(1);
    TestObject obj2(2);
    TestObject obj3(3);
    TestObject obj4(4);

    list1.push_back(obj1);
    list1.push_back(obj2);
    list2.push_back(obj3);
    list2.push_back(obj4);

    list1.swap(list2);

    // list1 should now contain obj3, obj4
    std::vector<int> values1;
    for (const auto& obj : list1)
    {
        values1.push_back(obj.value);
    }
    EXPECT_EQ(values1, (std::vector<int>{3, 4}));

    // list2 should now contain obj1, obj2
    std::vector<int> values2;
    for (const auto& obj : list2)
    {
        values2.push_back(obj.value);
    }
    EXPECT_EQ(values2, (std::vector<int>{1, 2}));
}

TEST_F(IntrusiveListTest, MultipleLists)
{
    using List1Type = dod::intrusive_list<&MultiListObject::list1_link>;
    using List2Type = dod::intrusive_list<&MultiListObject::list2_link>;

    MultiListObject obj1(1);
    MultiListObject obj2(2);
    MultiListObject obj3(3);

    List1Type list1;
    List2Type list2;

    // Add all objects to list1
    list1.push_back(obj1);
    list1.push_back(obj2);
    list1.push_back(obj3);

    // Add some objects to list2
    list2.push_back(obj1);
    list2.push_back(obj3);

    // Verify list1 contains all objects
    std::vector<int> values1;
    for (const auto& obj : list1)
    {
        values1.push_back(obj.id);
    }
    EXPECT_EQ(values1, (std::vector<int>{1, 2, 3}));

    // Verify list2 contains obj1 and obj3
    std::vector<int> values2;
    for (const auto& obj : list2)
    {
        values2.push_back(obj.id);
    }
    EXPECT_EQ(values2, (std::vector<int>{1, 3}));
}

TEST_F(IntrusiveListTest, AutomaticUnlinkingOnDestruction)
{
    using DestructorList = dod::intrusive_list<&DestructorTestObject::link>;
    DestructorList list;

    {
        DestructorTestObject obj1(1);
        DestructorTestObject obj2(2);

        list.push_back(obj1);
        list.push_back(obj2);

        EXPECT_FALSE(list.empty());
        EXPECT_EQ(DestructorTestObject::destructor_count, 0);
    } // Objects go out of scope here

    // Objects should be automatically unlinked
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(DestructorTestObject::destructor_count, 2);
}

TEST_F(IntrusiveListTest, ManualUnlinking)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestObject obj(42);

    {
        TestList list;
        list.push_back(obj);
        EXPECT_FALSE(TestList::can_insert(obj));

        // Manually unlink
        obj.link.unlink();
        EXPECT_TRUE(TestList::can_insert(obj));
        EXPECT_TRUE(list.empty());
    }

    // Object should still be unlinked
    EXPECT_TRUE(TestList::can_insert(obj));
}

TEST_F(IntrusiveListTest, NodeMoveSemantics)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestObject obj1(1);
    TestObject obj2(2);

    TestList list;
    list.push_back(obj1);

    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.front().value, 1);

    // Move the link from obj1 to obj2
    obj2.link = std::move(obj1.link);

    // obj1 should no longer be in the list, obj2 should be
    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.front().value, 2);
    EXPECT_TRUE(TestList::can_insert(obj1));
    EXPECT_FALSE(TestList::can_insert(obj2));
}

TEST_F(IntrusiveListTest, EmptyListOperations)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;

    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.begin(), list.end());
    EXPECT_EQ(list.cbegin(), list.cend());

    // These operations should not crash but are undefined behavior
    // In our implementation, they have assertions, so we won't test them in release builds
}

TEST_F(IntrusiveListTest, NonMemberSwap)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list1, list2;
    TestObject obj1(1);
    TestObject obj2(2);

    list1.push_back(obj1);
    list2.push_back(obj2);

    // Use non-member swap
    swap(list1, list2);

    EXPECT_EQ(list1.front().value, 2);
    EXPECT_EQ(list2.front().value, 1);
}

// Performance/stress test
TEST_F(IntrusiveListTest, StressTest)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    constexpr int NUM_OBJECTS = 1000;
    std::vector<TestObject> objects;
    objects.reserve(NUM_OBJECTS);

    for (int i = 0; i < NUM_OBJECTS; ++i)
    {
        objects.emplace_back(i);
    }

    TestList list;

    // Add all objects
    for (auto& obj : objects)
    {
        list.push_back(obj);
    }

    // Verify order
    int expected = 0;
    for (const auto& obj : list)
    {
        EXPECT_EQ(obj.value, expected++);
    }

    // Remove every other object
    for (int i = 1; i < NUM_OBJECTS; i += 2)
    {
        list.erase(objects[i]);
    }

    // Verify remaining objects
    expected = 0;
    for (const auto& obj : list)
    {
        EXPECT_EQ(obj.value, expected);
        expected += 2;
    }
}
