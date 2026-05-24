#pragma once

#include <memory>
#include <limits>
#include <iterator>
#include <initializer_list>
#include <type_traits>

template<typename T, size_t NodeMaxSize = 10, typename Allocator = std::allocator<T>>
class unrolled_list {
private:
    class node {
    public:
        node* next;
        node* prev;
        size_t cur_size;

        using t_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
        t_allocator_type t_alloc;

        alignas(T) char buffer[sizeof(T) * NodeMaxSize];

        node(const t_allocator_type& alloc = t_allocator_type())
            : next(nullptr), prev(nullptr), cur_size(0), t_alloc(alloc) {
        }

        T* elements() {
            return reinterpret_cast<T*>(buffer);
        }
        const T* elements() const {
            return reinterpret_cast<const T*>(buffer);
        }

        node(const node& other)
            : next(nullptr), prev(nullptr), cur_size(other.cur_size), t_alloc(other.t_alloc) {
            const T* old_data = other.elements();
            T* new_data = elements();
            for (size_t i = 0; i < cur_size; ++i) {
                new (new_data + i) T(old_data[i]);
            }
        }

        ~node() {
            T* elems = elements();
            for (size_t i = 0; i < cur_size; ++i) {
                elems[i].~T();
            }
        }
    };

    using node_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<node>;

    node* head_ptr;
    node* tail;
    node_allocator node_alloc;
    Allocator alloc;

public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;

    class iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        friend class unrolled_list;
        friend class const_iterator;

        iterator() : cur_node(nullptr), idx(0), tail_ptr(nullptr) {}
        iterator(node* n, size_t i, node* tail_node) : cur_node(n), idx(i), tail_ptr(tail_node) {}

        reference operator*() const {
            return cur_node->elements()[idx];
        }
        pointer operator->() const {
            return &(cur_node->elements()[idx]);
        }

        iterator& operator++() {
            if (cur_node) {
                if (idx + 1 < cur_node->cur_size) {
                    ++idx;
                }
                else {
                    cur_node = cur_node->next;
                    idx = 0;
                }
            }
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        iterator& operator--() {
            if (!cur_node) {
                cur_node = tail_ptr;
                if (cur_node)
                    idx = cur_node->cur_size - 1;
            }
            else if (idx > 0) {
                --idx;
            }
            else {
                cur_node = cur_node->prev;
                if (cur_node)
                    idx = cur_node->cur_size - 1;
            }
            return *this;
        }
        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }
        bool operator==(const iterator& other) const {
            return cur_node == other.cur_node && idx == other.idx;
        }
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        node* cur_node;
        size_t idx;
        node* tail_ptr;
    };

    class const_iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        friend class unrolled_list;
        friend class iterator;

        const_iterator() : cur_node(nullptr), idx(0), tail_ptr(nullptr) {}
        const_iterator(node* n, size_t i, node* tail_node) : cur_node(n), idx(i), tail_ptr(tail_node) {}
        const_iterator(const iterator& it) : cur_node(it.cur_node), idx(it.idx), tail_ptr(it.tail_ptr) {}

        reference operator*() const {
            return cur_node->elements()[idx];
        }
        pointer operator->() const {
            return &(cur_node->elements()[idx]);
        }
        const_iterator& operator++() {
            if (cur_node) {
                if (idx + 1 < cur_node->cur_size) {
                    ++idx;
                }
                else {
                    cur_node = cur_node->next;
                    idx = 0;
                }
            }
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        const_iterator& operator--() {
            if (!cur_node) {
                cur_node = tail_ptr;
                if (cur_node)
                    idx = cur_node->cur_size - 1;
            }
            else if (idx > 0) {
                --idx;
            }
            else {
                cur_node = cur_node->prev;
                if (cur_node)
                    idx = cur_node->cur_size - 1;
            }
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator tmp = *this;
            --(*this);
            return tmp;
        }
        bool operator==(const const_iterator& other) const {
            return cur_node == other.cur_node && idx == other.idx;
        }
        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }

    private:
        node* cur_node;
        size_t idx;
        node* tail_ptr;
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    unrolled_list()
        : head_ptr(nullptr), tail(nullptr),
        node_alloc(node_allocator()), alloc(Allocator()) {
    }

    unrolled_list(const Allocator& alloc_)
        : head_ptr(nullptr), tail(nullptr),
        node_alloc(node_allocator(alloc_)), alloc(alloc_) {
    }

    unrolled_list(const unrolled_list& other)
        : head_ptr(nullptr), tail(nullptr),
        node_alloc(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.alloc)),
        alloc(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.alloc)) {
        if (other.head_ptr) {
            head_ptr = std::allocator_traits<node_allocator>::allocate(node_alloc, 1);
            std::allocator_traits<node_allocator>::construct(node_alloc, head_ptr, *other.head_ptr);
            node* cur_this = head_ptr;
            node* cur_other = other.head_ptr->next;
            while (cur_other) {
                cur_this->next = std::allocator_traits<node_allocator>::allocate(node_alloc, 1);
                std::allocator_traits<node_allocator>::construct(node_alloc, cur_this->next, *cur_other);
                cur_this->next->prev = cur_this;
                cur_this = cur_this->next;
                cur_other = cur_other->next;
            }
            tail = cur_this;
        }
    }

    unrolled_list(const unrolled_list& other, const Allocator& alloc_)
        : head_ptr(nullptr), tail(nullptr),
        node_alloc(node_allocator(alloc_)), alloc(alloc_) {
        if (other.head_ptr) {
            head_ptr = std::allocator_traits<node_allocator>::allocate(node_alloc, 1);
            std::allocator_traits<node_allocator>::construct(node_alloc, head_ptr, *other.head_ptr);
            node* cur_this = head_ptr;
            node* cur_other = other.head_ptr->next;
            while (cur_other) {
                cur_this->next = std::allocator_traits<node_allocator>::allocate(node_alloc, 1);
                std::allocator_traits<node_allocator>::construct(node_alloc, cur_this->next, *cur_other);
                cur_this->next->prev = cur_this;
                cur_this = cur_this->next;
                cur_other = cur_other->next;
            }
            tail = cur_this;
        }
    }

    unrolled_list(size_type n, const T& t = T(), const Allocator& alloc_ = Allocator())
        : head_ptr(nullptr), tail(nullptr),
        node_alloc(node_allocator(alloc_)), alloc(alloc_) {
        try {
            for (size_type i = 0; i < n; ++i) {
                push_back(t);
            }
        }
        catch (...) {
            clear();
            throw;
        }
    }

    template<typename InputIterator>
    unrolled_list(InputIterator first, InputIterator last, const Allocator& alloc_ = Allocator())
        : head_ptr(nullptr), tail(nullptr),
        node_alloc(node_allocator(alloc_)), alloc(alloc_) {
        try {
            for (; first != last; ++first) {
                push_back(*first);
            }
        }
        catch (...) {
            clear();
            throw;
        }
    }

    unrolled_list(std::initializer_list<T> il, const Allocator& a = Allocator())
        : unrolled_list(il.begin(), il.end(), a) {
    }

    ~unrolled_list() {
        clear();
    }

    const_iterator head() const {
        return begin();
    }

    iterator head() {
        return begin();
    }

    unrolled_list& operator=(const unrolled_list& other) {
        if (this != &other) {
            unrolled_list copy(other);
            if (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value) {
                node_alloc = other.node_alloc;
                alloc = other.alloc;
            }
            swap(copy);
        }
        return *this;
    }

    unrolled_list& operator=(std::initializer_list<T> il) {
        unrolled_list tmp(il, alloc);
        swap(tmp);
        return *this;
    }

    bool operator==(const unrolled_list& other) const {
        if (size() != other.size()) {
            return false;
        }
        const_iterator it1 = begin();
        const_iterator it2 = other.begin();
        while (it1 != end()) {
            if (!(*it1 == *it2)) {
                return false;
            }
            ++it1;
            ++it2;
        }
        return true;
    }

    bool empty() const {
        return head_ptr == nullptr;
    }

    size_type size() const {
        size_type total = 0;
        for (node* cur = head_ptr; cur != nullptr; cur = cur->next) {
            total += cur->cur_size;
        }
        return total;
    }

    size_type max_size() const {
        return std::numeric_limits<size_type>::max();
    }

    void clear() noexcept {
        node* cur = head_ptr;
        while (cur) {
            node* nxt = cur->next;
            std::allocator_traits<node_allocator>::destroy(node_alloc, cur);
            std::allocator_traits<node_allocator>::deallocate(node_alloc, cur, 1);
            cur = nxt;
        }
        head_ptr = nullptr;
        tail = nullptr;
    }

    reference front() {
        if (empty()) {
            throw;
        }
        return head_ptr->elements()[0];
    }

    const_reference front() const {
        if (empty()) {
            throw;
        }
        return head_ptr->elements()[0];
    }

    reference back() {
        if (empty()) {
            throw;
        }
        return tail->elements()[tail->cur_size - 1];
    }

    const_reference back() const {
        if (empty()) {
            throw;
        }
        return tail->elements()[tail->cur_size - 1];
    }

    iterator begin() {
        return iterator(head_ptr, 0, tail);
    }

    iterator end() {
        return iterator(nullptr, 0, tail);
    }

    const_iterator begin() const {
        return const_iterator(head_ptr, 0, tail);
    }

    const_iterator end() const {
        return const_iterator(nullptr, 0, tail);
    }

    const_iterator cbegin() const {
        return const_iterator(head_ptr, 0, tail);
    }

    const_iterator cend() const {
        return const_iterator(nullptr, 0, tail);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }

    void push_back(const T& t) {
        using node_allocator_traits = std::allocator_traits<node_allocator>;
        if (!tail || tail->cur_size == NodeMaxSize) {
            node* new_node = node_allocator_traits::allocate(node_alloc, 1);
            node_allocator_traits::construct(node_alloc, new_node, typename node::t_allocator_type(alloc));
            if (!head_ptr) {
                head_ptr = new_node;
            }
            if (tail) {
                tail->next = new_node;
                new_node->prev = tail;
            }
            tail = new_node;
        }
        T* place = tail->elements() + tail->cur_size;
        new (place) T(t);
        tail->cur_size++;
    }

    void push_front(const T& t) {
        if (!head_ptr || head_ptr->cur_size == NodeMaxSize) {
            node* new_node = std::allocator_traits<node_allocator>::allocate(node_alloc, 1);
            std::allocator_traits<node_allocator>::construct(node_alloc, new_node,
                typename node::t_allocator_type(alloc));
            if (!head_ptr) {
                head_ptr = tail = new_node;
            }
            else {
                new_node->next = head_ptr;
                head_ptr->prev = new_node;
                head_ptr = new_node;
            }
            new (head_ptr->elements()) T(t);
            head_ptr->cur_size = 1;
        }
        else {
            node* cur = head_ptr;
            T* arr = cur->elements();
            for (size_t i = cur->cur_size; i > 0; --i) {
                new (arr + i) T(arr[i - 1]);
                arr[i - 1].~T();
            }
            new (arr) T(t);
            cur->cur_size++;
        }
    }

    void pop_back() noexcept {
        if (tail) {
            T* last_elem = tail->elements() + (tail->cur_size - 1);
            last_elem->~T();
            tail->cur_size--;
            if (tail->cur_size == 0 && (tail->prev || tail->next)) {
                node* to_delete = tail;
                if (tail->prev) {
                    tail = tail->prev;
                    tail->next = nullptr;
                }
                else {
                    tail = tail->next;
                    if (tail) tail->prev = nullptr;
                }
                std::allocator_traits<node_allocator>::destroy(node_alloc, to_delete);
                std::allocator_traits<node_allocator>::deallocate(node_alloc, to_delete, 1);
            }
            else if (tail->cur_size == 0 && !tail->prev && !tail->next) {
                std::allocator_traits<node_allocator>::destroy(node_alloc, tail);
                std::allocator_traits<node_allocator>::deallocate(node_alloc, tail, 1);
                head_ptr = tail = nullptr;
            }
        }
    }

    void pop_front() noexcept {
        if (head_ptr) {
            T* first_elem = head_ptr->elements();
            first_elem->~T();
            for (size_t i = 1; i < head_ptr->cur_size; ++i) {
                new (head_ptr->elements() + i - 1) T(*(head_ptr->elements() + i));
                (head_ptr->elements() + i)->~T();
            }
            head_ptr->cur_size--;
            if (head_ptr->cur_size == 0 && (head_ptr->next || head_ptr->prev)) {
                node* to_delete = head_ptr;
                if (head_ptr->next) {
                    head_ptr = head_ptr->next;
                    head_ptr->prev = nullptr;
                }
                else {
                    head_ptr = head_ptr->prev;
                    if (head_ptr) head_ptr->next = nullptr;
                }
                std::allocator_traits<node_allocator>::destroy(node_alloc, to_delete);
                std::allocator_traits<node_allocator>::deallocate(node_alloc, to_delete, 1);
            }
            else if (head_ptr->cur_size == 0 && !head_ptr->next && !head_ptr->prev) {
                std::allocator_traits<node_allocator>::destroy(node_alloc, head_ptr);
                std::allocator_traits<node_allocator>::deallocate(node_alloc, head_ptr, 1);
                head_ptr = tail = nullptr;
            }
        }
    }

    iterator insert(const_iterator pos, const T& value) {
        if (!pos.cur_node) {
            push_back(value);
            return iterator(tail, tail->cur_size - 1, tail);
        }
        node* cur = pos.cur_node;
        size_t idx = pos.idx;

        if (cur->cur_size < NodeMaxSize) {
            T* arr = cur->elements();
            for (size_t i = cur->cur_size; i > idx; --i) {
                new (arr + i) T(arr[i - 1]);
                arr[i - 1].~T();
            }
            new (arr + idx) T(value);
            cur->cur_size++;
            return iterator(cur, idx, tail);
        }
        else {
            node* new_node = std::allocator_traits<node_allocator>::allocate(node_alloc, 1);
            std::allocator_traits<node_allocator>::construct(node_alloc, new_node,
                typename node::t_allocator_type(alloc));

            size_t mid = NodeMaxSize / 2;
            T* arr_cur = cur->elements();
            T* arr_new = new_node->elements();

            for (size_t i = mid; i < NodeMaxSize; ++i) {
                new (arr_new + (i - mid)) T(arr_cur[i]);
                arr_cur[i].~T();
                new_node->cur_size++;
            }
            cur->cur_size = mid;

            new_node->next = cur->next;
            if (new_node->next) {
                new_node->next->prev = new_node;
            }
            else {
                tail = new_node;
            }
            cur->next = new_node;
            new_node->prev = cur;

            if (idx <= mid) {
                T* arr = cur->elements();
                for (size_t i = cur->cur_size; i > idx; --i) {
                    new (arr + i) T(arr[i - 1]);
                    arr[i - 1].~T();
                }
                new (arr + idx) T(value);
                cur->cur_size++;
                return iterator(cur, idx, tail);
            }
            else {
                size_t new_idx = idx - mid;
                T* arr = new_node->elements();
                for (size_t i = new_node->cur_size; i > new_idx; --i) {
                    new (arr + i) T(arr[i - 1]);
                    arr[i - 1].~T();
                }
                new (arr + new_idx) T(value);
                new_node->cur_size++;
                return iterator(new_node, new_idx, tail);
            }
        }
    }

    iterator insert(const_iterator pos, size_type count, const T& value) {
        iterator it = iterator(pos.cur_node, pos.idx, tail);
        for (size_type i = 0; i < count; ++i) {
            it = insert(pos, value);
            pos = const_iterator(it.cur_node, it.idx + 1, it.tail_ptr);
        }
        return it;
    }

    template<typename InputIterator>
    iterator insert(const_iterator pos, InputIterator first, InputIterator last) {
        iterator it(pos.cur_node, pos.idx, tail);
        while (first != last) {
            it = insert(pos, *first);
            ++first;
            pos = const_iterator(it.cur_node, it.idx + 1, it.tail_ptr);
        }
        return it;
    }

    iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
        return insert(pos, ilist.begin(), ilist.end());
    }

    template<typename Range>
    iterator insert_range(const_iterator pos, const Range& range) {
        return insert(pos, std::begin(range), std::end(range));
    }

    iterator erase(const_iterator pos) noexcept {
        node* cur = pos.cur_node;
        size_t idx = pos.idx;
        if (!cur) {
            return end();
        }
        T* arr = cur->elements();
        arr[idx].~T();
        for (size_t i = idx; i < cur->cur_size - 1; ++i) {
            new (arr + i) T(arr[i + 1]);
            arr[i + 1].~T();
        }
        cur->cur_size--;

        if (cur->cur_size == 0 && (cur->prev || cur->next)) {
            node* next_node = cur->next;
            if (cur->prev) {
                cur->prev->next = cur->next;
            }
            else {
                head_ptr = cur->next;
            }
            if (cur->next) {
                cur->next->prev = cur->prev;
            }
            else {
                tail = cur->prev;
            }
            std::allocator_traits<node_allocator>::destroy(node_alloc, cur);
            std::allocator_traits<node_allocator>::deallocate(node_alloc, cur, 1);
            return iterator(next_node, 0, tail);
        }
        else if (cur->cur_size == 0 && !cur->prev && !cur->next) {
            std::allocator_traits<node_allocator>::destroy(node_alloc, cur);
            std::allocator_traits<node_allocator>::deallocate(node_alloc, cur, 1);
            head_ptr = tail = nullptr;
            return iterator(nullptr, 0, nullptr);
        }
        else {
            if (idx >= cur->cur_size && cur->next) {
                return iterator(cur->next, 0, tail);
            }
            else {
                return iterator(cur, idx, tail);
            }
        }
    }

    iterator erase(const_iterator first, const_iterator last) noexcept {
        while (first != last) {
            first = erase(first);
        }
        return iterator(first.cur_node, first.idx, tail);
    }

    template<typename InputIterator>
    void assign(InputIterator first, InputIterator last) {
        unrolled_list tmp(first, last, alloc);
        swap(tmp);
    }

    void assign(size_type count, const T& value) {
        unrolled_list tmp(count, value, alloc);
        swap(tmp);
    }

    void assign(std::initializer_list<T> il) {
        unrolled_list tmp(il, alloc);
        swap(tmp);
    }

    void swap(unrolled_list& other) {
        if (node_alloc == other.node_alloc) {
            std::swap(head_ptr, other.head_ptr);
            std::swap(tail, other.tail);
        }
        else {
            unrolled_list temp(other);
            other = *this;
            *this = temp;
        }
        std::swap(node_alloc, other.node_alloc);
        std::swap(alloc, other.alloc);
    }

    allocator_type get_allocator() const {
        return alloc;
    }
};
