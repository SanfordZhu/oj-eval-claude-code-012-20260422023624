/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    /**
     * In linked_hashmap, iteration ordering is differ from map,
     * which is the order in which keys were inserted into the map.
     * You should maintain a doubly-linked list running through all
     * of its entries to keep the correct iteration order.
     *
     * Note that insertion order is not affected if a key is re-inserted
     * into the map.
     */

template<
    class Key,
    class T,
    class Hash = std::hash<Key>,
    class Equal = std::equal_to<Key>
> class linked_hashmap {
public:
    /**
     * the internal type of data.
     * it should have a default constructor, a copy constructor.
     * You can use sjtu::linked_hashmap as value_type by typedef.
     */
    typedef pair<const Key, T> value_type;

private:
    struct Node {
        value_type data;
        Node* prev;
        Node* next;
        Node* hash_prev;
        Node* hash_next;

        Node(const value_type& d) : data(d), prev(nullptr), next(nullptr), hash_prev(nullptr), hash_next(nullptr) {}
    };

    Node* head;
    Node* tail;
    Node** hash_table;
    size_t table_size;
    size_t element_count;

    static constexpr double LOAD_FACTOR = 0.75;
    static constexpr size_t INITIAL_SIZE = 16;

    Hash hash_func;
    Equal equal_func;

    friend class iterator;
    friend class const_iterator;

    size_t get_hash(const Key& key) const {
        return hash_func(key) % table_size;
    }

    void rehash() {
        size_t new_size = table_size * 2;
        Node** new_table = new Node*[new_size]();

        for (Node* curr = head; curr != nullptr; curr = curr->next) {
            size_t hash = hash_func(curr->data.first) % new_size;
            curr->hash_next = new_table[hash];
            curr->hash_prev = nullptr;
            if (new_table[hash] != nullptr) {
                new_table[hash]->hash_prev = curr;
            }
            new_table[hash] = curr;
        }

        delete[] hash_table;
        hash_table = new_table;
        table_size = new_size;
    }

    void initialize() {
        table_size = INITIAL_SIZE;
        hash_table = new Node*[table_size]();
        head = nullptr;
        tail = nullptr;
        element_count = 0;
    }

    void clear_all() {
        Node* curr = head;
        while (curr != nullptr) {
            Node* next = curr->next;
            delete curr;
            curr = next;
        }
        delete[] hash_table;
    }

    void copy_from(const linked_hashmap& other) {
        initialize();
        for (Node* curr = other.head; curr != nullptr; curr = curr->next) {
            insert(curr->data);
        }
    }

public:
    /**
     * see BidirectionalIterator at CppReference for help.
     *
     * if there is anything wrong throw invalid_iterator.
     *     like it = linked_hashmap.begin(); --it;
     *       or it = linked_hashmap.end(); ++end();
     */
    class const_iterator;
    class iterator {
    private:
        Node* node;
        const linked_hashmap* map;

        friend class linked_hashmap;
        friend class const_iterator;

    public:
        // The following code is written for the C++ type_traits library.
        // Type traits is a C++ feature for describing certain properties of a type.
        // For instance, for an iterator, iterator::value_type is the type that the
        // iterator points to.
        // STL algorithms and containers may use these type_traits (e.g. the following
        // typedef) to work properly.
        // See these websites for more information:
        // https://en.cppreference.com/w/cpp/header/type_traits
        // About value_type: https://blog.csdn.net/u014299153/article/details/72419713
        // About iterator_category: https://en.cppreference.com/w/cpp/iterator
        using difference_type = std::ptrdiff_t;
        using value_type = typename linked_hashmap::value_type;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::output_iterator_tag;


        iterator() : node(nullptr), map(nullptr) {}
        iterator(Node* n, const linked_hashmap* m) : node(n), map(m) {}
        iterator(const iterator &other) : node(other.node), map(other.map) {}
        /**
         * TODO iter++
         */
        iterator operator++(int) {
            iterator temp = *this;
            if (node == nullptr) throw invalid_iterator();
            node = node->next;
            return temp;
        }
        /**
         * TODO ++iter
         */
        iterator & operator++() {
            if (node == nullptr) throw invalid_iterator();
            node = node->next;
            return *this;
        }
        /**
         * TODO iter--
         */
        iterator operator--(int) {
            iterator temp = *this;
            if (node == nullptr) {
                node = map->tail;
            } else {
                node = node->prev;
            }
            if (node == nullptr) throw invalid_iterator();
            return temp;
        }
        /**
         * TODO --iter
         */
        iterator & operator--() {
            if (node == nullptr) {
                node = map->tail;
            } else {
                node = node->prev;
            }
            if (node == nullptr) throw invalid_iterator();
            return *this;
        }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory).
         */
        value_type & operator*() const {
            if (node == nullptr) throw invalid_iterator();
            return node->data;
        }
        bool operator==(const iterator &rhs) const {
            return node == rhs.node;
        }
        bool operator==(const const_iterator &rhs) const {
            return node == rhs.node;
        }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const {
            return node != rhs.node;
        }
        bool operator!=(const const_iterator &rhs) const {
            return node != rhs.node;
        }

        /**
         * for the support of it->first.
         * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
         */
        value_type* operator->() const noexcept {
            return &(node->data);
        }
    };

    class const_iterator {
    private:
        const Node* node;
        const linked_hashmap* map;

        friend class iterator;
        friend class linked_hashmap;

    public:
        const_iterator() : node(nullptr), map(nullptr) {}
        const_iterator(const Node* n, const linked_hashmap* m) : node(n), map(m) {}
        const_iterator(const const_iterator &other) : node(other.node), map(other.map) {}
        const_iterator(const iterator &other) : node(other.node), map(other.map) {}

        const_iterator operator++(int) {
            const_iterator temp = *this;
            if (node == nullptr) throw invalid_iterator();
            node = node->next;
            return temp;
        }
        const_iterator & operator++() {
            if (node == nullptr) throw invalid_iterator();
            node = node->next;
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator temp = *this;
            if (node == nullptr) {
                node = map->tail;
            } else {
                node = node->prev;
            }
            if (node == nullptr) throw invalid_iterator();
            return temp;
        }
        const_iterator & operator--() {
            if (node == nullptr) {
                node = map->tail;
            } else {
                node = node->prev;
            }
            if (node == nullptr) throw invalid_iterator();
            return *this;
        }
        const value_type & operator*() const {
            if (node == nullptr) throw invalid_iterator();
            return node->data;
        }
        bool operator==(const const_iterator &rhs) const {
            return node == rhs.node;
        }
        bool operator==(const iterator &rhs) const {
            return node == rhs.node;
        }
        bool operator!=(const const_iterator &rhs) const {
            return node != rhs.node;
        }
        bool operator!=(const iterator &rhs) const {
            return node != rhs.node;
        }
        const value_type* operator->() const noexcept {
            return &(node->data);
        }
    };

    /**
     * TODO two constructors
     */
    linked_hashmap() {
        initialize();
    }
    linked_hashmap(const linked_hashmap &other) {
        copy_from(other);
    }

    /**
     * TODO assignment operator
     */
    linked_hashmap & operator=(const linked_hashmap &other) {
        if (this != &other) {
            clear_all();
            copy_from(other);
        }
        return *this;
    }

    /**
     * TODO Destructors
     */
    ~linked_hashmap() {
        clear_all();
    }

    /**
     * TODO
     * access specified element with bounds checking
     * Returns a reference to the mapped value of the element with key equivalent to key.
     * If no such element exists, an exception of type `index_out_of_bound'
     */
    T & at(const Key &key) {
        size_t hash = get_hash(key);
        Node* curr = hash_table[hash];
        while (curr != nullptr) {
            if (equal_func(curr->data.first, key)) {
                return curr->data.second;
            }
            curr = curr->hash_next;
        }
        throw index_out_of_bound();
    }
    const T & at(const Key &key) const {
        size_t hash = get_hash(key);
        Node* curr = hash_table[hash];
        while (curr != nullptr) {
            if (equal_func(curr->data.first, key)) {
                return curr->data.second;
            }
            curr = curr->hash_next;
        }
        throw index_out_of_bound();
    }

    /**
     * TODO
     * access specified element
     * Returns a reference to the value that is mapped to a key equivalent to key,
     *   performing an insertion if such key does not already exist.
     */
    T & operator[](const Key &key) {
        size_t hash = get_hash(key);
        Node* curr = hash_table[hash];
        while (curr != nullptr) {
            if (equal_func(curr->data.first, key)) {
                return curr->data.second;
            }
            curr = curr->hash_next;
        }

        value_type new_value(key, T());
        insert(new_value);
        return at(key);
    }

    /**
     * behave like at() throw index_out_of_bound if such key does not exist.
     */
    const T & operator[](const Key &key) const {
        return at(key);
    }

    /**
     * return a iterator to the beginning
     */
    iterator begin() {
        return iterator(head, this);
    }
    const_iterator cbegin() const {
        return const_iterator(head, this);
    }

    /**
     * return a iterator to the end
     * in fact, it returns past-the-end.
     */
    iterator end() {
        return iterator(nullptr, this);
    }
    const_iterator cend() const {
        return const_iterator(nullptr, this);
    }

    /**
     * checks whether the container is empty
     * return true if empty, otherwise false.
     */
    bool empty() const {
        return element_count == 0;
    }

    /**
     * returns the number of elements.
     */
    size_t size() const {
        return element_count;
    }

    /**
     * clears the contents
     */
    void clear() {
        clear_all();
        initialize();
    }

    /**
     * insert an element.
     * return a pair, the first of the pair is
     *   the iterator to the new element (or the element that prevented the insertion),
     *   the second one is true if insert successfully, or false.
     */
    pair<iterator, bool> insert(const value_type &value) {
        size_t hash = get_hash(value.first);
        Node* curr = hash_table[hash];

        while (curr != nullptr) {
            if (equal_func(curr->data.first, value.first)) {
                return pair<iterator, bool>(iterator(curr, this), false);
            }
            curr = curr->hash_next;
        }

        if (element_count >= table_size * LOAD_FACTOR) {
            rehash();
            hash = get_hash(value.first);
        }

        Node* new_node = new Node(value);

        new_node->hash_next = hash_table[hash];
        if (hash_table[hash] != nullptr) {
            hash_table[hash]->hash_prev = new_node;
        }
        hash_table[hash] = new_node;

        if (tail == nullptr) {
            head = tail = new_node;
        } else {
            tail->next = new_node;
            new_node->prev = tail;
            tail = new_node;
        }

        element_count++;
        return pair<iterator, bool>(iterator(new_node, this), true);
    }

    /**
     * erase the element at pos.
     *
     * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
     */
    void erase(iterator pos) {
        if (pos.node == nullptr) throw invalid_iterator();

        Node* target = pos.node;
        size_t hash = get_hash(target->data.first);

        if (target->hash_prev != nullptr) {
            target->hash_prev->hash_next = target->hash_next;
        } else {
            hash_table[hash] = target->hash_next;
        }

        if (target->hash_next != nullptr) {
            target->hash_next->hash_prev = target->hash_prev;
        }

        if (target->prev != nullptr) {
            target->prev->next = target->next;
        } else {
            head = target->next;
        }

        if (target->next != nullptr) {
            target->next->prev = target->prev;
        } else {
            tail = target->prev;
        }

        delete target;
        element_count--;
    }

    /**
     * Returns the number of elements with key
     *   that compares equivalent to the specified argument,
     *   which is either 1 or 0
     *     since this container does not allow duplicates.
     */
    size_t count(const Key &key) const {
        size_t hash = get_hash(key);
        Node* curr = hash_table[hash];
        while (curr != nullptr) {
            if (equal_func(curr->data.first, key)) {
                return 1;
            }
            curr = curr->hash_next;
        }
        return 0;
    }

    /**
     * Finds an element with key equivalent to key.
     * key value of the element to search for.
     * Iterator to an element with key equivalent to key.
     *   If no such element is found, past-the-end (see end()) iterator is returned.
     */
    iterator find(const Key &key) {
        size_t hash = get_hash(key);
        Node* curr = hash_table[hash];
        while (curr != nullptr) {
            if (equal_func(curr->data.first, key)) {
                return iterator(curr, this);
            }
            curr = curr->hash_next;
        }
        return end();
    }
    const_iterator find(const Key &key) const {
        size_t hash = get_hash(key);
        Node* curr = hash_table[hash];
        while (curr != nullptr) {
            if (equal_func(curr->data.first, key)) {
                return const_iterator(curr, this);
            }
            curr = curr->hash_next;
        }
        return cend();
    }
};

}

#endif