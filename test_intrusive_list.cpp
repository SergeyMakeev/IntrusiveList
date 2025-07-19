#include "intrusive_list/intrusive_list.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <memory>
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

// Test object with const member to test const correctness
struct ConstTestObject
{
    const int id;
    mutable dod::list_node link;

    explicit ConstTestObject(int i) : id(i) {}
};

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

// NEW COMPREHENSIVE TESTS FOR 100% COVERAGE

TEST_F(IntrusiveListTest, IteratorComparisons)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);

    list.push_back(obj1);
    list.push_back(obj2);

    auto it1 = list.begin();
    auto it2 = list.begin();
    auto it3 = it1;
    ++it3;

    // Test equality
    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 == it3);

    // Test inequality
    EXPECT_FALSE(it1 != it2);
    EXPECT_TRUE(it1 != it3);

    // Test with end iterator
    auto end_it = list.end();
    EXPECT_FALSE(it1 == end_it);
    EXPECT_TRUE(it1 != end_it);
}

TEST_F(IntrusiveListTest, ConstIteratorComparisons)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);

    list.push_back(obj1);
    list.push_back(obj2);

    auto cit1 = list.cbegin();
    auto cit2 = list.cbegin();
    auto cit3 = cit1;
    ++cit3;

    // Test const iterator equality
    EXPECT_TRUE(cit1 == cit2);
    EXPECT_FALSE(cit1 == cit3);

    // Test const iterator inequality
    EXPECT_FALSE(cit1 != cit2);
    EXPECT_TRUE(cit1 != cit3);
}

TEST_F(IntrusiveListTest, ConstIteratorFromIterator)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj(42);

    list.push_back(obj);

    auto it = list.begin();
    // Test conversion from iterator to const_iterator
    auto cit = TestList::const_iterator(it);

    EXPECT_EQ(cit->value, 42);
    EXPECT_EQ((*cit).value, 42);
}

TEST_F(IntrusiveListTest, InsertAtBeginAndEnd)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);
    TestObject obj3(3);

    // Insert at begin of empty list
    auto it = list.insert(list.begin(), obj1);
    EXPECT_EQ(it->value, 1);
    EXPECT_EQ(list.front().value, 1);

    // Insert at end 
    it = list.insert(list.end(), obj2);
    EXPECT_EQ(it->value, 2);
    EXPECT_EQ(list.back().value, 2);

    // Insert at begin of non-empty list
    it = list.insert(list.begin(), obj3);
    EXPECT_EQ(it->value, 3);
    EXPECT_EQ(list.front().value, 3);

    // Verify order: 3, 1, 2
    std::vector<int> values;
    for (const auto& obj : list)
    {
        values.push_back(obj.value);
    }
    EXPECT_EQ(values, (std::vector<int>{3, 1, 2}));
}

TEST_F(IntrusiveListTest, EraseFirstAndLastElements)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);
    TestObject obj3(3);

    list.push_back(obj1);
    list.push_back(obj2);
    list.push_back(obj3);

    // Erase first element
    auto it = list.erase(list.begin());
    EXPECT_EQ(it->value, 2);

    // Erase last element (obj3)
    auto last_it = list.begin();
    ++last_it; // Point to obj3
    it = list.erase(last_it);
    EXPECT_EQ(it, list.end());

    // Only obj2 should remain
    EXPECT_EQ(list.front().value, 2);
    EXPECT_EQ(list.back().value, 2);
}

TEST_F(IntrusiveListTest, SwapEmptyLists)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list1, list2;

    // Swap two empty lists
    list1.swap(list2);

    EXPECT_TRUE(list1.empty());
    EXPECT_TRUE(list2.empty());
}

TEST_F(IntrusiveListTest, SwapEmptyWithNonEmpty)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList empty_list, full_list;
    TestObject obj1(1);
    TestObject obj2(2);

    full_list.push_back(obj1);
    full_list.push_back(obj2);

    // Swap empty with full
    empty_list.swap(full_list);

    EXPECT_FALSE(empty_list.empty());
    EXPECT_TRUE(full_list.empty());

    std::vector<int> values;
    for (const auto& obj : empty_list)
    {
        values.push_back(obj.value);
    }
    EXPECT_EQ(values, (std::vector<int>{1, 2}));
}

TEST_F(IntrusiveListTest, SwapNonEmptyWithEmpty)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList full_list, empty_list;
    TestObject obj1(1);
    TestObject obj2(2);

    full_list.push_back(obj1);
    full_list.push_back(obj2);

    // Swap full with empty (opposite order from previous test)
    full_list.swap(empty_list);

    EXPECT_TRUE(full_list.empty());
    EXPECT_FALSE(empty_list.empty());

    std::vector<int> values;
    for (const auto& obj : empty_list)
    {
        values.push_back(obj.value);
    }
    EXPECT_EQ(values, (std::vector<int>{1, 2}));
}

TEST_F(IntrusiveListTest, SelfSwap)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);

    list.push_back(obj1);
    list.push_back(obj2);

    // Self-swap should be no-op
    list.swap(list);

    std::vector<int> values;
    for (const auto& obj : list)
    {
        values.push_back(obj.value);
    }
    EXPECT_EQ(values, (std::vector<int>{1, 2}));
}

TEST_F(IntrusiveListTest, NodeSelfMoveAssignment)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestObject obj(42);
    TestList list;

    list.push_back(obj);

    // Self-move assignment should be no-op
    // NOLINTNEXTLINE(clang-diagnostic-self-move)
    obj.link = std::move(obj.link);

    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.front().value, 42);
    EXPECT_FALSE(TestList::can_insert(obj));
}

TEST_F(IntrusiveListTest, NodeMoveConstructor)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestObject obj1(1);
    TestObject obj2(2);
    TestList list;

    list.push_back(obj1);

    // Move construct obj2's link from obj1's link
    obj2.link = dod::list_node(std::move(obj1.link));

    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.front().value, 2);
    EXPECT_TRUE(TestList::can_insert(obj1));
    EXPECT_FALSE(TestList::can_insert(obj2));
}

TEST_F(IntrusiveListTest, UnlinkUnlinkedNode)
{
    TestObject obj(42);

    // Unlink a node that's not linked - should be safe
    EXPECT_FALSE(obj.link.is_linked());
    obj.link.unlink(); // Should be no-op
    EXPECT_FALSE(obj.link.is_linked());
}

TEST_F(IntrusiveListTest, MoveConstructorFromUnlinkedNode)
{
    TestObject obj1(1);
    TestObject obj2(2);

    EXPECT_FALSE(obj1.link.is_linked());

    // Move construct from unlinked node
    obj2.link = dod::list_node(std::move(obj1.link));

    EXPECT_FALSE(obj1.link.is_linked());
    EXPECT_FALSE(obj2.link.is_linked());
}

TEST_F(IntrusiveListTest, MoveAssignmentFromUnlinkedNode)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestObject obj1(1);
    TestObject obj2(2);
    TestList list;

    list.push_back(obj2);
    EXPECT_TRUE(obj2.link.is_linked());

    // Move assign from unlinked node to linked node
    obj2.link = std::move(obj1.link);

    EXPECT_FALSE(obj1.link.is_linked());
    EXPECT_FALSE(obj2.link.is_linked());
    EXPECT_TRUE(list.empty());
}

TEST_F(IntrusiveListTest, ConstCorrectness)
{
    using ConstList = dod::intrusive_list<&ConstTestObject::link>;
    ConstList list;
    ConstTestObject obj1(1);
    ConstTestObject obj2(2);

    list.push_back(obj1);
    list.push_back(obj2);

    const auto& const_list = list;

    // Test const front/back
    EXPECT_EQ(const_list.front().id, 1);
    EXPECT_EQ(const_list.back().id, 2);

    // Test const iteration
    std::vector<int> values;
    for (const auto& obj : const_list)
    {
        values.push_back(obj.id);
    }
    EXPECT_EQ(values, (std::vector<int>{1, 2}));
}

TEST_F(IntrusiveListTest, IteratorDereferenceOperators)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj(42);

    list.push_back(obj);

    auto it = list.begin();
    auto cit = list.cbegin();

    // Test operator->
    EXPECT_EQ(it->value, 42);
    EXPECT_EQ(cit->value, 42);

    // Test operator*
    EXPECT_EQ((*it).value, 42);
    EXPECT_EQ((*cit).value, 42);

    // Test that we can modify through non-const iterator
    (*it).value = 84;
    EXPECT_EQ(obj.value, 84);
}

TEST_F(IntrusiveListTest, ListSelfMoveAssignment)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);

    list.push_back(obj1);
    list.push_back(obj2);

    // Self-move assignment should be no-op
    // NOLINTNEXTLINE(clang-diagnostic-self-move)
    list = std::move(list);

    std::vector<int> values;
    for (const auto& obj : list)
    {
        values.push_back(obj.value);
    }
    EXPECT_EQ(values, (std::vector<int>{1, 2}));
}

TEST_F(IntrusiveListTest, ComplexIteratorNavigation)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);
    TestObject obj3(3);

    list.push_back(obj1);
    list.push_back(obj2);
    list.push_back(obj3);

    auto it = list.end();
    --it; // Should point to obj3
    EXPECT_EQ(it->value, 3);

    --it; // Should point to obj2
    EXPECT_EQ(it->value, 2);

    --it; // Should point to obj1
    EXPECT_EQ(it->value, 1);

    // Navigate forward again
    ++it; // Should point to obj2
    EXPECT_EQ(it->value, 2);

    ++it; // Should point to obj3
    EXPECT_EQ(it->value, 3);

    ++it; // Should point to end
    EXPECT_EQ(it, list.end());
}

TEST_F(IntrusiveListTest, PostIncrementDecrementReturn)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);

    list.push_back(obj1);
    list.push_back(obj2);

    auto it = list.begin();

    // Test post-increment returns old value
    auto old_it = it++;
    EXPECT_EQ(old_it->value, 1);
    EXPECT_EQ(it->value, 2);

    // Test post-decrement returns old value
    old_it = it--;
    EXPECT_EQ(old_it->value, 2);
    EXPECT_EQ(it->value, 1);
}

TEST_F(IntrusiveListTest, ConstIteratorPostIncrementDecrement)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);

    list.push_back(obj1);
    list.push_back(obj2);

    auto cit = list.cbegin();

    // Test const iterator post-increment
    auto old_cit = cit++;
    EXPECT_EQ(old_cit->value, 1);
    EXPECT_EQ(cit->value, 2);

    // Test const iterator post-decrement
    old_cit = cit--;
    EXPECT_EQ(old_cit->value, 2);
    EXPECT_EQ(cit->value, 1);
}

TEST_F(IntrusiveListTest, NodeToObjectUtility)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestObject obj(42);

    // Test the public node_to_object utility
    auto* obj_ptr = TestList::node_to_object(&obj.link);
    EXPECT_EQ(obj_ptr, &obj);
    EXPECT_EQ(obj_ptr->value, 42);

    // Test with const node
    const auto* const_obj_ptr = TestList::node_to_object(static_cast<const dod::list_node*>(&obj.link));
    EXPECT_EQ(const_obj_ptr, &obj);
    EXPECT_EQ(const_obj_ptr->value, 42);
}

TEST_F(IntrusiveListTest, EmptyListBeginEnd)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;

    // Empty list: begin should equal end
    EXPECT_EQ(list.begin(), list.end());
    EXPECT_EQ(list.cbegin(), list.cend());

    const auto& const_list = list;
    EXPECT_EQ(const_list.begin(), const_list.end());
}

TEST_F(IntrusiveListTest, SingleElementListNavigation)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj(42);

    list.push_back(obj);

    auto it = list.begin();
    EXPECT_EQ(it->value, 42);

    ++it;
    EXPECT_EQ(it, list.end());

    --it;
    EXPECT_EQ(it->value, 42);
    EXPECT_EQ(it, list.begin());
}

TEST_F(IntrusiveListTest, ClearSingleElement)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj(42);

    list.push_back(obj);
    EXPECT_FALSE(list.empty());

    list.clear();
    EXPECT_TRUE(list.empty());
    EXPECT_TRUE(TestList::can_insert(obj));
}

TEST_F(IntrusiveListTest, MultipleEraseByObject)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);
    TestObject obj3(3);

    list.push_back(obj1);
    list.push_back(obj2);
    list.push_back(obj3);

    // Erase all objects by reference
    list.erase(obj2); // middle
    list.erase(obj1); // front
    list.erase(obj3); // back (now only element)

    EXPECT_TRUE(list.empty());
    EXPECT_TRUE(TestList::can_insert(obj1));
    EXPECT_TRUE(TestList::can_insert(obj2));
    EXPECT_TRUE(TestList::can_insert(obj3));
}

TEST_F(IntrusiveListTest, MoveNodeBetweenPositions)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);
    TestObject obj3(3);
    TestObject extra(999);

    list.push_back(obj1);
    list.push_back(obj2);
    list.push_back(obj3);

    // Move obj2's position to extra
    extra.link = std::move(obj2.link);

    // List should now contain: 1, 999, 3
    std::vector<int> values;
    for (const auto& obj : list)
    {
        values.push_back(obj.value);
    }
    EXPECT_EQ(values, (std::vector<int>{1, 999, 3}));

    EXPECT_TRUE(TestList::can_insert(obj2));
    EXPECT_FALSE(TestList::can_insert(extra));
}

TEST_F(IntrusiveListTest, IteratorTypesAndCategory)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    using Iterator = TestList::iterator;
    using ConstIterator = TestList::const_iterator;

    // Verify iterator category
    static_assert(std::is_same_v<Iterator::iterator_category, std::bidirectional_iterator_tag>);
    static_assert(std::is_same_v<ConstIterator::iterator_category, std::bidirectional_iterator_tag>);

    // Verify value types
    static_assert(std::is_same_v<Iterator::value_type, TestObject>);
    static_assert(std::is_same_v<ConstIterator::value_type, TestObject>);

    // Verify pointer types
    static_assert(std::is_same_v<Iterator::pointer, TestObject*>);
    static_assert(std::is_same_v<ConstIterator::pointer, const TestObject*>);

    // Verify reference types
    static_assert(std::is_same_v<Iterator::reference, TestObject&>);
    static_assert(std::is_same_v<ConstIterator::reference, const TestObject&>);
}

TEST_F(IntrusiveListTest, ListTypeAliases)
{
    using TestList = dod::intrusive_list<&TestObject::link>;

    // Verify type aliases
    static_assert(std::is_same_v<TestList::value_type, TestObject>);
    static_assert(std::is_same_v<TestList::reference, TestObject&>);
    static_assert(std::is_same_v<TestList::const_reference, const TestObject&>);
    static_assert(std::is_same_v<TestList::pointer, TestObject*>);
    static_assert(std::is_same_v<TestList::const_pointer, const TestObject*>);
    static_assert(std::is_same_v<TestList::size_type, std::size_t>);
    static_assert(std::is_same_v<TestList::difference_type, std::ptrdiff_t>);
}

// Additional edge case tests for complete coverage
TEST_F(IntrusiveListTest, IteratorInvalidOperations)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    
    // Test default constructed iterators
    TestList::iterator default_it;
    TestList::const_iterator default_cit;
    
    // These should be valid comparisons with default-constructed iterators
    EXPECT_TRUE(default_it == TestList::iterator{});
    EXPECT_TRUE(default_cit == TestList::const_iterator{});
    EXPECT_FALSE(default_it != TestList::iterator{});
    EXPECT_FALSE(default_cit != TestList::const_iterator{});
}

TEST_F(IntrusiveListTest, MoveFromLinkedToLinkedNode)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list1, list2;
    TestObject obj1(1);
    TestObject obj2(2);
    
    list1.push_back(obj1);
    list2.push_back(obj2);
    
    // Move from linked node to another linked node
    obj2.link = std::move(obj1.link);
    
    // obj1 should be unlinked, obj2 should be in list1, list2 should be empty
    EXPECT_TRUE(TestList::can_insert(obj1));
    EXPECT_FALSE(TestList::can_insert(obj2));
    EXPECT_TRUE(list2.empty());
    EXPECT_FALSE(list1.empty());
    EXPECT_EQ(list1.front().value, 2);
}

TEST_F(IntrusiveListTest, ConstListBeginEnd)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj(42);
    
    list.push_back(obj);
    
    const TestList& const_list = list;
    
    // Test const version of begin/end
    auto const_begin = const_list.begin();
    auto const_end = const_list.end();
    
    EXPECT_NE(const_begin, const_end);
    EXPECT_EQ(const_begin->value, 42);
    
    ++const_begin;
    EXPECT_EQ(const_begin, const_end);
}

TEST_F(IntrusiveListTest, EraseSingleElementList)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj(42);
    
    list.push_back(obj);
    
    // Erase the only element
    auto it = list.erase(list.begin());
    
    EXPECT_EQ(it, list.end());
    EXPECT_TRUE(list.empty());
    EXPECT_TRUE(TestList::can_insert(obj));
}

TEST_F(IntrusiveListTest, InsertAndEraseAtSamePosition)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj1(1);
    TestObject obj2(2);
    TestObject obj3(3);
    
    list.push_back(obj1);
    list.push_back(obj3);
    
    // Insert obj2 between obj1 and obj3
    auto pos = list.begin();
    ++pos; // Point to obj3
    auto inserted_it = list.insert(pos, obj2);
    
    // Now erase obj2
    auto next_it = list.erase(inserted_it);
    
    EXPECT_EQ(next_it->value, 3); // Should point to obj3
    EXPECT_TRUE(TestList::can_insert(obj2));
    
    // List should contain only obj1 and obj3
    std::vector<int> values;
    for (const auto& obj : list)
    {
        values.push_back(obj.value);
    }
    EXPECT_EQ(values, (std::vector<int>{1, 3}));
}

TEST_F(IntrusiveListTest, NodeIsLinkedAfterOperations)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj(42);
    
    // Initially unlinked
    EXPECT_FALSE(obj.link.is_linked());
    
    // After push_back
    list.push_back(obj);
    EXPECT_TRUE(obj.link.is_linked());
    
    // After pop_back
    list.pop_back();
    EXPECT_FALSE(obj.link.is_linked());
    
    // After push_front
    list.push_front(obj);
    EXPECT_TRUE(obj.link.is_linked());
    
    // After pop_front
    list.pop_front();
    EXPECT_FALSE(obj.link.is_linked());
}

TEST_F(IntrusiveListTest, MultipleSwapsInSequence)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list1, list2;
    TestObject obj1(1);
    TestObject obj2(2);
    
    list1.push_back(obj1);
    list2.push_back(obj2);
    
    // Multiple swaps should work correctly
    list1.swap(list2); // list1 has obj2, list2 has obj1
    EXPECT_EQ(list1.front().value, 2);
    EXPECT_EQ(list2.front().value, 1);
    
    list1.swap(list2); // Back to original
    EXPECT_EQ(list1.front().value, 1);
    EXPECT_EQ(list2.front().value, 2);
    
    list1.swap(list2); // Swap again
    EXPECT_EQ(list1.front().value, 2);
    EXPECT_EQ(list2.front().value, 1);
}

TEST_F(IntrusiveListTest, ClearNonEmptyList)
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
    
    // Clear should unlink all objects
    list.clear();
    
    EXPECT_TRUE(list.empty());
    EXPECT_TRUE(TestList::can_insert(obj1));
    EXPECT_TRUE(TestList::can_insert(obj2));
    EXPECT_TRUE(TestList::can_insert(obj3));
    EXPECT_FALSE(obj1.link.is_linked());
    EXPECT_FALSE(obj2.link.is_linked());
    EXPECT_FALSE(obj3.link.is_linked());
}

TEST_F(IntrusiveListTest, ConstIteratorComparisonEdgeCases)
{
    using TestList = dod::intrusive_list<&TestObject::link>;
    TestList list;
    TestObject obj(42);
    
    list.push_back(obj);
    
    auto cit1 = list.cbegin();
    auto cit2 = list.cend();
    auto cit3 = list.cbegin();
    
    // Test all comparison combinations
    EXPECT_TRUE(cit1 == cit3);
    EXPECT_FALSE(cit1 == cit2);
    EXPECT_FALSE(cit1 != cit3);
    EXPECT_TRUE(cit1 != cit2);
    EXPECT_TRUE(cit2 == list.cend());
    EXPECT_FALSE(cit2 != list.cend());
}

TEST_F(IntrusiveListTest, DestructorOrderWithLinkedNodes)
{
    using DestructorList = dod::intrusive_list<&DestructorTestObject::link>;
    
    {
        DestructorList list;
        auto obj1 = std::make_unique<DestructorTestObject>(1);
        auto obj2 = std::make_unique<DestructorTestObject>(2);
        
        list.push_back(*obj1);
        list.push_back(*obj2);
        
        EXPECT_FALSE(list.empty());
        EXPECT_EQ(DestructorTestObject::destructor_count, 0);
        
        // Destroy objects while still in list
        obj1.reset();
        EXPECT_EQ(DestructorTestObject::destructor_count, 1);
        EXPECT_FALSE(list.empty()); // obj2 still in list
        
        obj2.reset();
        EXPECT_EQ(DestructorTestObject::destructor_count, 2);
        EXPECT_TRUE(list.empty()); // Now empty
    }
}
