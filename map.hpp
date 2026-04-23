
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

#include "utility.hpp"
#include <functional>
#include <cstddef>

namespace sjtu {

template <class Key, class T, class Compare = std::less<Key>>
class map {
public:
    typedef Key key_type;
    typedef T mapped_type;
    typedef pair<const Key, T> value_type;
    typedef Compare key_compare;
    
private:
    struct Node {
        value_type data;
        Node *left, *right, *parent;
        int height;
        
        Node(const value_type &val) : data(val), left(nullptr), right(nullptr), parent(nullptr), height(1) {}
    };
    
    Node *root;
    size_t node_count;
    key_compare comp;
    
    int height(Node *node) const {
        return node ? node->height : 0;
    }
    
    int balance_factor(Node *node) const {
        return node ? height(node->left) - height(node->right) : 0;
    }
    
    void update_height(Node *node) {
        if (node) {
            node->height = 1 + (height(node->left) > height(node->right) ? height(node->left) : height(node->right));
        }
    }
    
    Node* rotate_right(Node *y) {
        Node *x = y->left;
        Node *T2 = x->right;
        
        x->right = y;
        y->left = T2;
        
        if (T2) T2->parent = y;
        x->parent = y->parent;
        y->parent = x;
        
        update_height(y);
        update_height(x);
        
        return x;
    }
    
    Node* rotate_left(Node *x) {
        Node *y = x->right;
        Node *T2 = y->left;
        
        y->left = x;
        x->right = T2;
        
        if (T2) T2->parent = x;
        y->parent = x->parent;
        x->parent = y;
        
        update_height(x);
        update_height(y);
        
        return y;
    }
    
    Node* balance(Node *node) {
        if (!node) return nullptr;
        
        update_height(node);
        
        int bf = balance_factor(node);
        
        if (bf > 1) {
            if (balance_factor(node->left) < 0) {
                node->left = rotate_left(node->left);
            }
            return rotate_right(node);
        }
        
        if (bf < -1) {
            if (balance_factor(node->right) > 0) {
                node->right = rotate_right(node->right);
            }
            return rotate_left(node);
        }
        
        return node;
    }
    
    Node* insert_node(Node *node, Node *parent, const value_type &value) {
        if (!node) {
            node_count++;
            return new Node(value);
        }
        
        if (comp(value.first, node->data.first)) {
            node->left = insert_node(node->left, node, value);
            if (node->left) node->left->parent = node;
        } else if (comp(node->data.first, value.first)) {
            node->right = insert_node(node->right, node, value);
            if (node->right) node->right->parent = node;
        } else {
            return node;
        }
        
        return balance(node);
    }
    
    Node* find_min(Node *node) const {
        while (node && node->left) {
            node = node->left;
        }
        return node;
    }
    
    Node* find_max(Node *node) const {
        while (node && node->right) {
            node = node->right;
        }
        return node;
    }
    
    Node* erase_node(Node *node, const Key &key) {
        if (!node) return nullptr;
        
        if (comp(key, node->data.first)) {
            node->left = erase_node(node->left, key);
            if (node->left) node->left->parent = node;
        } else if (comp(node->data.first, key)) {
            node->right = erase_node(node->right, key);
            if (node->right) node->right->parent = node;
        } else {
            if (!node->left || !node->right) {
                Node *temp = node->left ? node->left : node->right;
                if (!temp) {
                    temp = node;
                    node = nullptr;
                } else {
                    Node *old_parent = node->parent;
                    Node *old_left = node->left;
                    Node *old_right = node->right;
                    
                    const_cast<Key&>(node->data.first) = temp->data.first;
                    node->data.second = temp->data.second;
                    node->left = temp->left;
                    node->right = temp->right;
                    node->height = temp->height;
                    
                    if (node->left) node->left->parent = node;
                    if (node->right) node->right->parent = node;
                    
                    temp->left = old_left;
                    temp->right = old_right;
                    temp->parent = old_parent;
                    
                    delete temp;
                }
                node_count--;
            } else {
                Node *temp = find_min(node->right);
                value_type temp_data = temp->data;
                
                node->right = erase_node(node->right, temp->data.first);
                if (node->right) node->right->parent = node;
                
                const_cast<Key&>(node->data.first) = temp_data.first;
                node->data.second = temp_data.second;
            }
        }
        
        return balance(node);
    }
    
    Node* find_node(Node *node, const Key &key) const {
        while (node) {
            if (comp(key, node->data.first)) {
                node = node->left;
            } else if (comp(node->data.first, key)) {
                node = node->right;
            } else {
                return node;
            }
        }
        return nullptr;
    }
    
    void clear_tree(Node *node) {
        if (node) {
            clear_tree(node->left);
            clear_tree(node->right);
            delete node;
        }
    }
    
    Node* copy_tree(Node *other_node, Node *parent) {
        if (!other_node) return nullptr;
        
        Node *new_node = new Node(other_node->data);
        new_node->parent = parent;
        new_node->height = other_node->height;
        
        new_node->left = copy_tree(other_node->left, new_node);
        new_node->right = copy_tree(other_node->right, new_node);
        
        return new_node;
    }

public:
    class const_iterator;
    class iterator {
    private:
        Node *current;
        map *container;
        
    public:
        iterator(Node *node = nullptr, map *cont = nullptr) : current(node), container(cont) {}
        
        iterator(const iterator &other) : current(other.current), container(other.container) {}
        
        value_type& operator*() const {
            return current->data;
        }
        
        value_type* operator->() const {
            return &(current->data);
        }
        
        iterator& operator++() {
            if (!current) {
                return *this;
            }
            
            if (current->right) {
                current = current->right;
                while (current->left) {
                    current = current->left;
                }
            } else {
                Node *parent = current->parent;
                while (parent && current == parent->right) {
                    current = parent;
                    parent = parent->parent;
                }
                current = parent;
            }
            
            return *this;
        }
        
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        iterator& operator--() {
            if (!current) {
                if (container && container->root) {
                    current = container->root;
                    while (current->right) {
                        current = current->right;
                    }
                }
                return *this;
            }
            
            if (current->left) {
                current = current->left;
                while (current->right) {
                    current = current->right;
                }
            } else {
                Node *parent = current->parent;
                while (parent && current == parent->left) {
                    current = parent;
                    parent = parent->parent;
                }
                current = parent;
            }
            
            return *this;
        }
        
        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }
        
        bool operator==(const iterator &other) const {
            return current == other.current;
        }
        
        bool operator==(const const_iterator &other) const {
            return current == other.current;
        }
        
        bool operator!=(const iterator &other) const {
            return current != other.current;
        }
        
        bool operator!=(const const_iterator &other) const {
            return current != other.current;
        }
        
        friend class const_iterator;
        friend class map;
    };
    
    class const_iterator {
    private:
        const Node *current;
        const map *container;
        
    public:
        const_iterator(const Node *node = nullptr, const map *cont = nullptr) : current(node), container(cont) {}
        
        const_iterator(const const_iterator &other) : current(other.current), container(other.container) {}
        
        const_iterator(const iterator &other) : current(other.current), container(other.container) {}
        
        const value_type& operator*() const {
            return current->data;
        }
        
        const value_type* operator->() const {
            return &(current->data);
        }
        
        const_iterator& operator++() {
            if (!current) {
                return *this;
            }
            
            if (current->right) {
                current = current->right;
                while (current->left) {
                    current = current->left;
                }
            } else {
                const Node *parent = current->parent;
                while (parent && current == parent->right) {
                    current = parent;
                    parent = parent->parent;
                }
                current = parent;
            }
            
            return *this;
        }
        
        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        const_iterator& operator--() {
            if (!current) {
                if (container && container->root) {
                    current = container->root;
                    while (current->right) {
                        current = current->right;
                    }
                }
                return *this;
            }
            
            if (current->left) {
                current = current->left;
                while (current->right) {
                    current = current->right;
                }
            } else {
                const Node *parent = current->parent;
                while (parent && current == parent->left) {
                    current = parent;
                    parent = parent->parent;
                }
                current = parent;
            }
            
            return *this;
        }
        
        const_iterator operator--(int) {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }
        
        bool operator==(const const_iterator &other) const {
            return current == other.current;
        }
        
        bool operator==(const iterator &other) const {
            return current == other.current;
        }
        
        bool operator!=(const const_iterator &other) const {
            return current != other.current;
        }
        
        bool operator!=(const iterator &other) const {
            return current != other.current;
        }
        
        friend class iterator;
        friend class map;
    };
    
    map() : root(nullptr), node_count(0), comp() {}
    
    map(const map &other) : root(nullptr), node_count(0), comp(other.comp) {
        root = copy_tree(other.root, nullptr);
        node_count = other.node_count;
    }
    
    map& operator=(const map &other) {
        if (this != &other) {
            clear();
            comp = other.comp;
            root = copy_tree(other.root, nullptr);
            node_count = other.node_count;
        }
        return *this;
    }
    
    ~map() {
        clear();
    }
    
    T& operator[](const Key &key) {
        Node *node = find_node(root, key);
        if (!node) {
            root = insert_node(root, nullptr, value_type(key, T()));
            node = find_node(root, key);
        }
        return node->data.second;
    }
    
    T& at(const Key &key) {
        Node *node = find_node(root, key);
        if (!node) {
            throw std::out_of_range("Key not found");
        }
        return node->data.second;
    }
    
    const T& at(const Key &key) const {
        Node *node = find_node(root, key);
        if (!node) {
            throw std::out_of_range("Key not found");
        }
        return node->data.second;
    }
    
    iterator begin() {
        return iterator(find_min(root), this);
    }
    
    const_iterator begin() const {
        return const_iterator(find_min(root), this);
    }
    
    const_iterator cbegin() const {
        return const_iterator(find_min(root), this);
    }
    
    iterator end() {
        return iterator(nullptr, this);
    }
    
    const_iterator end() const {
        return const_iterator(nullptr, this);
    }
    
    const_iterator cend() const {
        return const_iterator(nullptr, this);
    }
    
    bool empty() const {
        return node_count == 0;
    }
    
    size_t size() const {
        return node_count;
    }
    
    void clear() {
        clear_tree(root);
        root = nullptr;
        node_count = 0;
    }
    
    pair<iterator, bool> insert(const value_type &value) {
        Node *existing = find_node(root, value.first);
        if (existing) {
            return pair<iterator, bool>(iterator(existing, this), false);
        }
        
        root = insert_node(root, nullptr, value);
        Node *new_node = find_node(root, value.first);
        return pair<iterator, bool>(iterator(new_node, this), true);
    }
    
    void erase(iterator pos) {
        if (pos.current) {
            root = erase_node(root, pos.current->data.first);
        }
    }
    
    size_t erase(const Key &key) {
        Node *existing = find_node(root, key);
        if (existing) {
            root = erase_node(root, key);
            return 1;
        }
        return 0;
    }
    
    iterator find(const Key &key) {
        Node *node = find_node(root, key);
        return iterator(node, this);
    }
    
    const_iterator find(const Key &key) const {
        Node *node = find_node(root, key);
        return const_iterator(node, this);
    }
    
    size_t count(const Key &key) const {
        return find_node(root, key) ? 1 : 0;
    }
    
    iterator lower_bound(const Key &key) {
        Node *current = root;
        Node *result = nullptr;
        
        while (current) {
            if (!comp(current->data.first, key)) {
                result = current;
                current = current->left;
            } else {
                current = current->right;
            }
        }
        
        return iterator(result, this);
    }
    
    const_iterator lower_bound(const Key &key) const {
        Node *current = root;
        Node *result = nullptr;
        
        while (current) {
            if (!comp(current->data.first, key)) {
                result = current;
                current = current->left;
            } else {
                current = current->right;
            }
        }
        
        return const_iterator(result, this);
    }
    
    iterator upper_bound(const Key &key) {
        Node *current = root;
        Node *result = nullptr;
        
        while (current) {
            if (comp(key, current->data.first)) {
                result = current;
                current = current->left;
            } else {
                current = current->right;
            }
        }
        
        return iterator(result, this);
    }
    
    const_iterator upper_bound(const Key &key) const {
        Node *current = root;
        Node *result = nullptr;
        
        while (current) {
            if (comp(key, current->data.first)) {
                result = current;
                current = current->left;
            } else {
                current = current->right;
            }
        }
        
        return const_iterator(result, this);
    }
    
    pair<iterator, iterator> equal_range(const Key &key) {
        return pair<iterator, iterator>(lower_bound(key), upper_bound(key));
    }
    
    pair<const_iterator, const_iterator> equal_range(const Key &key) const {
        return pair<const_iterator, const_iterator>(lower_bound(key), upper_bound(key));
    }
};

}

#endif
