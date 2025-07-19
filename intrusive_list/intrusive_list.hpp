#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace dod
{

template <auto Member> class intrusive_list;

/*

  **Node type for intrusive linked lists**

  Objects that want to be stored in an intrusive_list must contain a member of this type.

 */
class list_node
{
    template <auto Member> friend class intrusive_list;
    template <auto Member, bool IsConst> friend class intrusive_list_iterator_impl;

  private:
    list_node* next_ = nullptr;
    list_node* prev_ = nullptr;

  public:
    list_node() = default;

    // Prevent dangling pointers by auto-unlinking on destruction
    ~list_node() { unlink(); }

    list_node(list_node&& other) noexcept
    {
        if (!other.is_linked())
        {
            return;
        }

        // Take over the other node's position in the list
        next_ = other.next_;
        prev_ = other.prev_;

        // In a circular list, linked nodes are guaranteed to have valid pointers when the node is linked
        assert(next_ != nullptr && prev_ != nullptr);
        next_->prev_ = this;
        prev_->next_ = this;

        // Leave other node in unlinked state
        other.next_ = nullptr;
        other.prev_ = nullptr;
    }

    list_node& operator=(list_node&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        // Remove this node from any current list
        unlink();

        if (!other.is_linked())
        {
            return *this;
        }

        // Take over the other node's position
        next_ = other.next_;
        prev_ = other.prev_;

        // In a circular list, linked nodes are guaranteed to have valid pointers when the node is linked
        assert(next_ != nullptr && prev_ != nullptr);
        next_->prev_ = this;
        prev_->next_ = this;

        // Leave other node in unlinked state
        other.next_ = nullptr;
        other.prev_ = nullptr;

        return *this;
    }

    // Prevent accidental copies that would corrupt list structure
    list_node(const list_node&) = delete;
    list_node& operator=(const list_node&) = delete;

    bool is_linked() const noexcept
    {
        // Ensure that both pointers are either null (unlinked) or non-null (linked)
        assert((next_ != nullptr && prev_ != nullptr) || (next_ == nullptr && prev_ == nullptr));

        // Optimized: only check one pointer since both are always null or non-null together
        // This invariant is maintained by the circular list design
        return next_ != nullptr;
    }

    void unlink() noexcept
    {
        if (!is_linked())
        {
            return;
        }

        // In a circular list, linked nodes are guaranteed to have valid pointers when the node is linked
        assert(next_ != nullptr && prev_ != nullptr);
        next_->prev_ = prev_;
        prev_->next_ = next_;
        next_ = nullptr;
        prev_ = nullptr;
    }
};

/*

**Template iterator implementation for intrusive_list**

Single implementation handles both const and non-const iterators to eliminate code duplication.
Uses IsConst template parameter to control pointer and reference types.

*/
template <auto Member, bool IsConst> class intrusive_list_iterator_impl
{
    template <auto M, bool IC> friend class intrusive_list_iterator_impl;

    // Helper to extract class type from member pointer
    template <typename U> struct class_type_helper;
    template <typename U, typename V> struct class_type_helper<V U::*>
    {
        using type = U;
    };

    using T = typename class_type_helper<decltype(Member)>::type;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<IsConst, const T*, T*>;
    using reference = std::conditional_t<IsConst, const T&, T&>;

  private:
    using node_pointer = std::conditional_t<IsConst, const list_node*, list_node*>;
    node_pointer current_;

    static auto node_to_object(node_pointer node) noexcept { return intrusive_list<Member>::node_to_object(node); }

  public:
    explicit intrusive_list_iterator_impl(node_pointer node = nullptr) noexcept
        : current_(node)
    {
    }

    // Allow conversion from non-const to const iterator
    template <bool OtherIsConst, typename = std::enable_if_t<IsConst && !OtherIsConst>>
    intrusive_list_iterator_impl(const intrusive_list_iterator_impl<Member, OtherIsConst>& other) noexcept
        : current_(other.current_)
    {
    }

    reference operator*() const noexcept
    {
        assert(current_ != nullptr);
        return *node_to_object(current_);
    }

    pointer operator->() const noexcept
    {
        assert(current_ != nullptr);
        return node_to_object(current_);
    }

    intrusive_list_iterator_impl& operator++() noexcept
    {
        assert(current_ != nullptr);
        current_ = current_->next_;
        return *this;
    }

    intrusive_list_iterator_impl operator++(int) noexcept
    {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    intrusive_list_iterator_impl& operator--() noexcept
    {
        assert(current_ != nullptr);
        current_ = current_->prev_;
        return *this;
    }

    intrusive_list_iterator_impl operator--(int) noexcept
    {
        auto temp = *this;
        --(*this);
        return temp;
    }

    template <bool OtherIsConst> bool operator==(const intrusive_list_iterator_impl<Member, OtherIsConst>& other) const noexcept
    {
        return current_ == other.current_;
    }

    template <bool OtherIsConst> bool operator!=(const intrusive_list_iterator_impl<Member, OtherIsConst>& other) const noexcept
    {
        return current_ != other.current_;
    }

    friend class intrusive_list<Member>;
};

// Type aliases for the specific iterator types
template <auto Member> using intrusive_list_iterator = intrusive_list_iterator_impl<Member, false>;
template <auto Member> using intrusive_list_const_iterator = intrusive_list_iterator_impl<Member, true>;

/*

**Intrusive doubly-linked list**

This is a circular intrusive doubly-linked list using a sentinel node.

The sentinel acts as both head and tail, allowing for uniform insert/remove operations without special cases or null checks.
Since the list is circular, an empty list has sentinel->next and sentinel->prev pointing to itself.
Each node stores its own links, enabling features like auto-unlink where a node can safely detach itself without knowing the container.

This design avoids dynamic memory allocation and provides O(1) insertion and removal.

Ideal for performance-critical systems where memory layout and control matter.
The main limitation is that a node can only belong to one list at a time.

*/
template <auto Member> class intrusive_list
{
    static_assert(std::is_member_object_pointer_v<decltype(Member)>);

    // Helper to extract class and member types from member pointer
    template <typename U> struct member_type_helper;
    template <typename U, typename V> struct member_type_helper<V U::*>
    {
        using class_type = U;
        using member_type = V;
    };

    using T = typename member_type_helper<decltype(Member)>::class_type;
    using member_t = typename member_type_helper<decltype(Member)>::member_type;

    static_assert(std::is_same_v<std::remove_cv_t<member_t>, list_node>);

  public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = intrusive_list_iterator<Member>;
    using const_iterator = intrusive_list_const_iterator<Member>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

  public:
    // Convert list_node pointer to object pointer using pointer arithmetic
    // Single template handles both const/non-const cases to avoid code duplication
    template <typename NodePtr> static auto node_to_object(NodePtr node) noexcept
    {
        using ReturnType = std::conditional_t<std::is_const_v<std::remove_pointer_t<NodePtr>>, const T*, T*>;
        assert(node != nullptr);

        // Calculate offset of Member within T at compile time
        static const auto offset = [] { return reinterpret_cast<std::uintptr_t>(&(static_cast<T*>(nullptr)->*Member)); }();

        // Subtract offset from node address to get object address
        auto obj_ptr = reinterpret_cast<std::uintptr_t>(node) - offset;
        return reinterpret_cast<ReturnType>(obj_ptr);
    }

  private:
    // Sentinel creates circular list - eliminates null checks and enables self-unlinking
    //
    //   Empty: sentinel->sentinel
    //   Non-empty: head<->...nodes...<->tail<->sentinel
    //
    // Key insight: nodes can unlink themselves without needing a pointer to the containing list
    mutable list_node sentinel_;

    static list_node& object_to_node(T& obj) noexcept { return obj.*Member; }
    static const list_node& object_to_node(const T& obj) noexcept { return obj.*Member; }

  public:
    intrusive_list() noexcept
    {
        // Initialize as circular list with sentinel pointing to itself
        sentinel_.next_ = &sentinel_;
        sentinel_.prev_ = &sentinel_;
    }

    ~intrusive_list() { clear(); }

    intrusive_list(intrusive_list&& other) noexcept
        : intrusive_list()
    {
        swap(other);
    }

    intrusive_list& operator=(intrusive_list&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        clear();
        swap(other);
        return *this;
    }

    // Prevent list copies.
    // To allow an object to be in multiple lists, each list needs to have its own `list_node` embedded in the object.
    intrusive_list(const intrusive_list&) = delete;
    intrusive_list& operator=(const intrusive_list&) = delete;

    bool empty() const noexcept { return sentinel_.next_ == &sentinel_; }

    iterator begin() noexcept { return iterator(sentinel_.next_); }
    const_iterator begin() const noexcept { return const_iterator(sentinel_.next_); }
    const_iterator cbegin() const noexcept { return const_iterator(sentinel_.next_); }
    iterator end() noexcept { return iterator(&sentinel_); }
    const_iterator end() const noexcept { return const_iterator(&sentinel_); }
    const_iterator cend() const noexcept { return const_iterator(&sentinel_); }

    reference front() noexcept
    {
        assert(!empty());
        return *node_to_object(sentinel_.next_);
    }

    const_reference front() const noexcept
    {
        assert(!empty());
        return *node_to_object(sentinel_.next_);
    }

    reference back() noexcept
    {
        assert(!empty());
        return *node_to_object(sentinel_.prev_);
    }

    const_reference back() const noexcept
    {
        assert(!empty());
        return *node_to_object(sentinel_.prev_);
    }

    void push_front(T& obj)
    {
        auto& node = object_to_node(obj);
        // Prevent corruption from double-insertion
        assert(!node.is_linked());

        node.next_ = sentinel_.next_;
        node.prev_ = &sentinel_;
        sentinel_.next_->prev_ = &node;
        sentinel_.next_ = &node;
    }

    void push_back(T& obj)
    {
        auto& node = object_to_node(obj);
        assert(!node.is_linked());

        node.next_ = &sentinel_;
        node.prev_ = sentinel_.prev_;
        sentinel_.prev_->next_ = &node;
        sentinel_.prev_ = &node;
    }

    void pop_front() noexcept
    {
        assert(!empty());
        sentinel_.next_->unlink();
    }

    void pop_back() noexcept
    {
        assert(!empty());
        sentinel_.prev_->unlink();
    }

    iterator insert(const_iterator pos, T& obj)
    {
        auto& node = object_to_node(obj);
        assert(!node.is_linked());

        auto* pos_node = const_cast<list_node*>(pos.current_);
        node.next_ = pos_node;
        node.prev_ = pos_node->prev_;
        pos_node->prev_->next_ = &node;
        pos_node->prev_ = &node;

        return iterator(&node);
    }

    iterator erase(const_iterator pos) noexcept
    {
        // Cannot erase end() iterator (sentinel node)
        assert(pos != end());

        auto* node = const_cast<list_node*>(pos.current_);
        auto* next_node = node->next_;
        node->unlink();
        return iterator(next_node);
    }

    void erase(T& obj) noexcept
    {
        auto& node = object_to_node(obj);
        node.unlink();
    }

    void clear() noexcept
    {
        while (!empty())
        {
            pop_front();
        }
    }

    void swap(intrusive_list& other) noexcept
    {
        if (this == &other)
            return;

        // Handle empty lists specially - sentinel nodes need different pointer updates
        bool this_empty = empty();
        bool other_empty = other.empty();

        if (this_empty && other_empty)
        {
            return;
        }

        if (this_empty)
        {
            // Transfer other's nodes and repoint them to this sentinel
            sentinel_.next_ = other.sentinel_.next_;
            sentinel_.prev_ = other.sentinel_.prev_;
            sentinel_.next_->prev_ = &sentinel_;
            sentinel_.prev_->next_ = &sentinel_;

            // Reset other to empty state
            other.sentinel_.next_ = &other.sentinel_;
            other.sentinel_.prev_ = &other.sentinel_;
        }
        else if (other_empty)
        {
            // Transfer this list to other
            other.sentinel_.next_ = sentinel_.next_;
            other.sentinel_.prev_ = sentinel_.prev_;
            other.sentinel_.next_->prev_ = &other.sentinel_;
            other.sentinel_.prev_->next_ = &other.sentinel_;

            sentinel_.next_ = &sentinel_;
            sentinel_.prev_ = &sentinel_;
        }
        else
        {
            // Both non-empty: swap pointers and update sentinel references
            std::swap(sentinel_.next_, other.sentinel_.next_);
            std::swap(sentinel_.prev_, other.sentinel_.prev_);

            // Update head/tail nodes to point to correct sentinels
            sentinel_.next_->prev_ = &sentinel_;
            sentinel_.prev_->next_ = &sentinel_;
            other.sentinel_.next_->prev_ = &other.sentinel_;
            other.sentinel_.prev_->next_ = &other.sentinel_;
        }
    }

    // Prevents double-insertion bugs by checking if object is already linked
    static bool can_insert(const T& obj) noexcept { return !object_to_node(obj).is_linked(); }
};

template <auto Member> void swap(intrusive_list<Member>& a, intrusive_list<Member>& b) noexcept { a.swap(b); }

} // namespace dod