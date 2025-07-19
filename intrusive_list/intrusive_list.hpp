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
    /**
     * @brief Default constructor - creates an unlinked node
     */
    list_node() = default;

    /**
     * @brief Destructor - automatically unlinks from any list
     */
    ~list_node() { unlink(); }

    /**
     * @brief Move constructor
     */
    list_node(list_node&& other) noexcept
    {
        if (other.is_linked())
        {
            // Take over the other node's position in the list
            next_ = other.next_;
            prev_ = other.prev_;

            // Update neighbors to point to this node
            if (next_)
                next_->prev_ = this;
            if (prev_)
                prev_->next_ = this;

            // Clear the other node
            other.next_ = nullptr;
            other.prev_ = nullptr;
        }
    }

    /**
     * @brief Move assignment operator
     */
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

    // Disable copy operations
    list_node(const list_node&) = delete;
    list_node& operator=(const list_node&) = delete;

    /**
     * @brief Check if this node is currently linked in a list
     */
    bool is_linked() const noexcept { return next_ != nullptr || prev_ != nullptr; }

    /**
     * @brief Remove this node from its current list (if any)
     */
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

    // Helper to convert from node to object
    static T* node_to_object(list_node* node) noexcept
    {
        if (!node)
            return nullptr;

        // Calculate offset of the member within T using offsetof-like calculation
        static const auto offset = [] { return reinterpret_cast<std::uintptr_t>(&(static_cast<T*>(nullptr)->*Member)); }();

        // Subtract offset to get object pointer
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
    mutable list_node sentinel_; // Sentinel node for circular list

    // Helper to get the list_node from an object
    static list_node& object_to_node(T& obj) noexcept { return obj.*Member; }

    static const list_node& object_to_node(const T& obj) noexcept { return obj.*Member; }

  public:
    /**
     * @brief Default constructor - creates an empty list
     */
    intrusive_list() noexcept
    {
        sentinel_.next_ = &sentinel_;
        sentinel_.prev_ = &sentinel_;
    }

    /**
     * @brief Destructor - clears the list
     */
    ~intrusive_list() { clear(); }

    /**
     * @brief Move constructor
     */
    intrusive_list(intrusive_list&& other) noexcept
        : intrusive_list()
    {
        swap(other);
    }

    /**
     * @brief Move assignment operator
     */
    intrusive_list& operator=(intrusive_list&& other) noexcept
    {
        if (this != &other)
        {
            clear();
            swap(other);
        }
        return *this;
    }

    // Disable copy operations
    intrusive_list(const intrusive_list&) = delete;
    intrusive_list& operator=(const intrusive_list&) = delete;

    /**
     * @brief Check if the list is empty
     */
    bool empty() const noexcept { return sentinel_.next_ == &sentinel_; }

    /**
     * @brief Get iterator to the beginning
     */
    iterator begin() noexcept { return iterator(sentinel_.next_); }

    const_iterator begin() const noexcept { return const_iterator(sentinel_.next_); }

    const_iterator cbegin() const noexcept { return const_iterator(sentinel_.next_); }

    /**
     * @brief Get iterator to the end
     */
    iterator end() noexcept { return iterator(&sentinel_); }

    const_iterator end() const noexcept { return const_iterator(&sentinel_); }

    const_iterator cend() const noexcept { return const_iterator(&sentinel_); }

    /**
     * @brief Get reference to the first element
     */
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

    /**
     * @brief Get reference to the last element
     */
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

    /**
     * @brief Add element to the front of the list
     */
    void push_front(T& obj)
    {
        auto& node = object_to_node(obj);
        assert(!node.is_linked()); // Object must not already be in a list

        node.next_ = sentinel_.next_;
        node.prev_ = &sentinel_;

        sentinel_.next_->prev_ = &node;
        sentinel_.next_ = &node;
    }

    /**
     * @brief Add element to the back of the list
     */
    void push_back(T& obj)
    {
        auto& node = object_to_node(obj);
        assert(!node.is_linked()); // Object must not already be in a list

        node.next_ = &sentinel_;
        node.prev_ = sentinel_.prev_;

        sentinel_.prev_->next_ = &node;
        sentinel_.prev_ = &node;
    }

    /**
     * @brief Remove the first element
     */
    void pop_front() noexcept
    {
        assert(!empty());
        sentinel_.next_->unlink();
    }

    /**
     * @brief Remove the last element
     */
    void pop_back() noexcept
    {
        assert(!empty());
        sentinel_.prev_->unlink();
    }

    /**
     * @brief Insert element before the given position
     */
    iterator insert(const_iterator pos, T& obj)
    {
        auto& node = object_to_node(obj);
        assert(!node.is_linked()); // Object must not already be in a list

        auto* pos_node = const_cast<list_node*>(pos.current_);

        node.next_ = pos_node;
        node.prev_ = pos_node->prev_;

        pos_node->prev_->next_ = &node;
        pos_node->prev_ = &node;

        return iterator(&node);
    }

    /**
     * @brief Remove element at the given position
     */
    iterator erase(const_iterator pos) noexcept
    {
        assert(pos != end());

        auto* node = const_cast<list_node*>(pos.current_);
        auto* next_node = node->next_;

        node->unlink();

        return iterator(next_node);
    }

    /**
     * @brief Remove the given object from the list
     */
    void erase(T& obj) noexcept
    {
        auto& node = object_to_node(obj);
        node.unlink();
    }

    /**
     * @brief Remove all elements from the list
     */
    void clear() noexcept
    {
        while (!empty())
        {
            pop_front();
        }
    }

    /**
     * @brief Swap contents with another list
     */
    void swap(intrusive_list& other) noexcept
    {
        if (this == &other)
            return;

        // Handle empty lists
        bool this_empty = empty();
        bool other_empty = other.empty();

        if (this_empty && other_empty)
        {
            return; // Both empty, nothing to do
        }

        if (this_empty)
        {
            // This is empty, other is not
            sentinel_.next_ = other.sentinel_.next_;
            sentinel_.prev_ = other.sentinel_.prev_;
            sentinel_.next_->prev_ = &sentinel_;
            sentinel_.prev_->next_ = &sentinel_;

            other.sentinel_.next_ = &other.sentinel_;
            other.sentinel_.prev_ = &other.sentinel_;
        }
        else if (other_empty)
        {
            // Other is empty, this is not
            other.sentinel_.next_ = sentinel_.next_;
            other.sentinel_.prev_ = sentinel_.prev_;
            other.sentinel_.next_->prev_ = &other.sentinel_;
            other.sentinel_.prev_->next_ = &other.sentinel_;

            sentinel_.next_ = &sentinel_;
            sentinel_.prev_ = &sentinel_;
        }
        else
        {
            // Both non-empty
            std::swap(sentinel_.next_, other.sentinel_.next_);
            std::swap(sentinel_.prev_, other.sentinel_.prev_);

            sentinel_.next_->prev_ = &sentinel_;
            sentinel_.prev_->next_ = &sentinel_;
            other.sentinel_.next_->prev_ = &other.sentinel_;
            other.sentinel_.prev_->next_ = &other.sentinel_;
        }
    }

    /**
     * @brief Check if an object can be safely added to the list
     */
    static bool can_insert(const T& obj) noexcept { return !object_to_node(obj).is_linked(); }
};

/**
 * @brief Non-member swap function
 */
template <auto Member> void swap(intrusive_list<Member>& a, intrusive_list<Member>& b) noexcept { a.swap(b); }

} // namespace intrusive