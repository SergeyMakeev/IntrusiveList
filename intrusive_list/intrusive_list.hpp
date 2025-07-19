#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace dod
{

// Forward declarations
template <auto Member> class intrusive_list;

/**
 * @brief Node type for intrusive linked lists
 *
 * Objects that want to be stored in an intrusive_list must contain
 * a member of this type.
 */
class list_node
{
    template <auto Member> friend class intrusive_list;

    template <auto Member> friend class intrusive_list_iterator;

    template <auto Member> friend class intrusive_list_const_iterator;

  private:
    list_node* next_ = nullptr;
    list_node* prev_ = nullptr;

  public:
    list_node() = default;

    // Automatically unlinks from any list to prevent dangling pointers
    ~list_node() { unlink(); }

    list_node(list_node&& other) noexcept
    {
        if (other.is_linked())
        {
            // Take over the other node's position in the list
            next_ = other.next_;
            prev_ = other.prev_;

            // Update neighbors to point to this node instead of the moved-from node
            if (next_)
                next_->prev_ = this;
            if (prev_)
                prev_->next_ = this;

            // Clear the other node to leave it in an unlinked state
            other.next_ = nullptr;
            other.prev_ = nullptr;
        }
    }

    list_node& operator=(list_node&& other) noexcept
    {
        if (this != &other)
        {
            unlink(); // Remove this node from any current list

            if (other.is_linked())
            {
                // Take over the other node's position
                next_ = other.next_;
                prev_ = other.prev_;

                // Update neighbors
                if (next_)
                    next_->prev_ = this;
                if (prev_)
                    prev_->next_ = this;

                // Clear the other node
                other.next_ = nullptr;
                other.prev_ = nullptr;
            }
        }
        return *this;
    }

    // Disable copy operations - intrusive nodes should have unique ownership semantics
    list_node(const list_node&) = delete;
    list_node& operator=(const list_node&) = delete;

    bool is_linked() const noexcept { return next_ != nullptr || prev_ != nullptr; }

    void unlink() noexcept
    {
        if (is_linked())
        {
            if (next_)
                next_->prev_ = prev_;
            if (prev_)
                prev_->next_ = next_;
            next_ = nullptr;
            prev_ = nullptr;
        }
    }
};

/**
 * @brief Iterator for intrusive_list
 */
template <auto Member> class intrusive_list_iterator
{
    template <auto V> friend class intrusive_list_const_iterator;

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
    using pointer = T*;
    using reference = T&;

  private:
    list_node* current_;

    // Convert from list_node back to containing object using pointer arithmetic
    // We calculate the offset of the Member within T and subtract it from the node address
    static T* node_to_object(list_node* node) noexcept
    {
        if (!node)
            return nullptr;

        // Calculate offset of the member within T - this lambda captures the offset at compile time
        // We use nullptr cast to avoid actually dereferencing null, just getting the address offset
        static const auto offset = [] { return reinterpret_cast<std::uintptr_t>(&(static_cast<T*>(nullptr)->*Member)); }();

        // Subtract offset to get object pointer - this works because the node is embedded within the object
        auto obj_ptr = reinterpret_cast<std::uintptr_t>(node) - offset;
        return reinterpret_cast<T*>(obj_ptr);
    }

  public:
    explicit intrusive_list_iterator(list_node* node = nullptr) noexcept
        : current_(node)
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

    intrusive_list_iterator& operator++() noexcept
    {
        assert(current_ != nullptr);
        current_ = current_->next_;
        return *this;
    }

    intrusive_list_iterator operator++(int) noexcept
    {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    intrusive_list_iterator& operator--() noexcept
    {
        assert(current_ != nullptr);
        current_ = current_->prev_;
        return *this;
    }

    intrusive_list_iterator operator--(int) noexcept
    {
        auto temp = *this;
        --(*this);
        return temp;
    }

    bool operator==(const intrusive_list_iterator& other) const noexcept { return current_ == other.current_; }

    bool operator!=(const intrusive_list_iterator& other) const noexcept { return current_ != other.current_; }

    friend class intrusive_list<Member>;
};

/**
 * @brief Const iterator for intrusive_list
 */
template <auto Member> class intrusive_list_const_iterator
{
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
    using pointer = const T*;
    using reference = const T&;

  private:
    const list_node* current_;

    static const T* node_to_object(const list_node* node) noexcept
    {
        if (!node)
            return nullptr;

        static const auto offset = [] { return reinterpret_cast<std::uintptr_t>(&(static_cast<T*>(nullptr)->*Member)); }();
        auto obj_ptr = reinterpret_cast<std::uintptr_t>(node) - offset;
        return reinterpret_cast<const T*>(obj_ptr);
    }

  public:
    explicit intrusive_list_const_iterator(const list_node* node = nullptr) noexcept
        : current_(node)
    {
    }

    // Allow conversion from non-const iterator
    intrusive_list_const_iterator(const intrusive_list_iterator<Member>& other) noexcept
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

    intrusive_list_const_iterator& operator++() noexcept
    {
        assert(current_ != nullptr);
        current_ = current_->next_;
        return *this;
    }

    intrusive_list_const_iterator operator++(int) noexcept
    {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    intrusive_list_const_iterator& operator--() noexcept
    {
        assert(current_ != nullptr);
        current_ = current_->prev_;
        return *this;
    }

    intrusive_list_const_iterator operator--(int) noexcept
    {
        auto temp = *this;
        --(*this);
        return temp;
    }

    bool operator==(const intrusive_list_const_iterator& other) const noexcept { return current_ == other.current_; }

    bool operator!=(const intrusive_list_const_iterator& other) const noexcept { return current_ != other.current_; }

    friend class intrusive_list<Member>;
};

/**
 * @brief Intrusive doubly-linked list
 *
 * @tparam Member Pointer to the list_node member within the object type
 *
 * Usage example:
 * @code
 * struct MyObject {
 *     int value;
 *     intrusive::list_node link;
 * };
 *
 * intrusive::intrusive_list<&MyObject::link> my_list;
 * @endcode
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

  private:
    // Sentinel node creates a circular list - simplifies edge case handling by eliminating null checks
    // Empty list has sentinel pointing to itself; non-empty list has sentinel as dummy node between head/tail
    // Another key benefit: nodes can unlink themselves without needing a pointer to the containing list
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
        if (this != &other)
        {
            clear();
            swap(other);
        }
        return *this;
    }

    // Disable copy operations - intrusive containers should not be copied as objects may be in multiple lists
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
        return *begin();
    }

    const_reference front() const noexcept
    {
        assert(!empty());
        return *begin();
    }

    reference back() noexcept
    {
        assert(!empty());
        return *(--end());
    }

    const_reference back() const noexcept
    {
        assert(!empty());
        return *(--end());
    }

    void push_front(T& obj)
    {
        auto& node = object_to_node(obj);
        // Objects can only be in one intrusive list at a time to prevent corruption
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

        // Handle empty lists specially because sentinel nodes need different treatment
        bool this_empty = empty();
        bool other_empty = other.empty();

        if (this_empty && other_empty)
        {
            return;
        }

        if (this_empty)
        {
            // Transfer other's list to this - update all nodes to point to this sentinel
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
            // Both non-empty - swap the sentinel pointers and update all nodes
            std::swap(sentinel_.next_, other.sentinel_.next_);
            std::swap(sentinel_.prev_, other.sentinel_.prev_);

            // Update head/tail nodes to point to correct sentinels
            sentinel_.next_->prev_ = &sentinel_;
            sentinel_.prev_->next_ = &sentinel_;
            other.sentinel_.next_->prev_ = &other.sentinel_;
            other.sentinel_.prev_->next_ = &other.sentinel_;
        }
    }

    // Check if an object can be safely added - prevents double-insertion bugs
    static bool can_insert(const T& obj) noexcept { return !object_to_node(obj).is_linked(); }
};

template <auto Member> void swap(intrusive_list<Member>& a, intrusive_list<Member>& b) noexcept { a.swap(b); }

} // namespace dod