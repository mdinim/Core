//
// Created by Dániel Molnár on 2019-09-02.
//

#pragma once
#ifndef CORE_BINARYSEARCHTREE_HPP
#define CORE_BINARYSEARCHTREE_HPP

// ----- std -----
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <vector>

// ----- libraries -----

// ----- in-project dependencies -----
#include <Graph/BinaryTree.hpp>

// ----- forward-decl -----

namespace Core {

/// \brief Binary search tree, with fast search of values
template <class T> class BinarySearchTree {
  public:
    /// \brief Internal representation of a node.
    class Node {
        friend class BinarySearchTree;

      private:
        /// \brief reference to the holder of the node
        BinarySearchTree &_owner;

        /// \brief raw-ptr to the parent
        Node *_parent;

        /// \brief Value of the node
        T _value;

        /// \brief Unique ownership of the left child
        std::unique_ptr<Node> _left;

        /// \brief Unique ownership of the right child
        std::unique_ptr<Node> _right;

        /// \brief Internal use only to retrieve a reference to the unique ptr
        /// to this node.
        constexpr std::unique_ptr<Node> &get_as_child_ref() const {
            if (!has_parent())
                return _owner._root_node;
            return is_left_child() ? _parent->_left : _parent->_right;
        }

      public:
        /// \brief Construct a node.
        Node(BinarySearchTree &owner, Node *parent, const T &value)
            : _owner(owner), _parent(parent), _value(value) {}

        /// \brief Move construction of a node.
        Node(BinarySearchTree &owner, Node *parent, T &&value)
            : _owner(owner), _parent(parent), _value(value) {}

        Node(Node &&) = delete;
        Node &operator=(Node &&) = delete;
        Node(const Node &) = delete;
        Node &operator=(const Node &) = delete;

        /// \brief Check if two nodes are different.
        bool operator!=(const Node &other) const { return !(*this == other); }

        /// \brief Check if two nodes are the same.
        bool operator==(const Node &other) const {
            return std::addressof(_owner) == std::addressof(other._owner) &&
                   get_as_child_ref() == other.get_as_child_ref();
        }

        /// \brief Access the underlying value. Note that a value of a node can
        /// not be modified afterwards, since it would break the binary search
        /// tree invariant.
        const T &value() const { return _value; }

        /// \brief Check if node holds a specific value
        bool operator!=(const T &value) const { return !(*this == value); }

        /// \brief Check if node holds a specific value
        bool operator==(const T &value) const { return _value == value; }

        /// \brief Access the parent of the node.
        const Node &parent() const { return *_parent; }

        /// \copydoc parent()
        Node &parent() { return *_parent; }

        /// \brief Check if a node is a left child.
        [[nodiscard]] bool is_left_child() const {
            return _parent && _parent->_left.get() == this;
        }

        /// \brief Check if a node is a right child.
        [[nodiscard]] bool is_right_child() const {
            return _parent && _parent->_right.get() == this;
        }

        /// \brief Check if a node has parent. Note that in a binary tree only
        /// the root node has no parent.
        [[nodiscard]] bool has_parent() const { return _parent != nullptr; }

        /// \brief Check if a node has left child.
        [[nodiscard]] bool has_left_child() const { return _left != nullptr; }

        /// \brief Check if a node has right child.
        [[nodiscard]] bool has_right_child() const { return _right != nullptr; }

        /// \brief Access to the left child node.
        const Node &left_child() const { return *_left; }

        /// \brief Access to the right child node.
        const Node &right_child() const { return *_right; }

        /// \copydoc left_child()
        Node &left_child() { return *_left; }

        /// \copydoc right_child()
        Node &right_child() { return *_right; }
    };

  protected:
    /// \brief Unique ownership of the root node.
    std::unique_ptr<Node> _root_node;

  public:
    BinarySearchTree() = default;

    BinarySearchTree(const BinarySearchTree &) = delete; // TODO
    BinarySearchTree(BinarySearchTree &&) = delete;      // TODO

    using value_type = T;

    /// \brief Access the root node.
    const Node &root_node() const { return *_root_node; }

    /// \copydoc root_node()
    Node &root_node() { return *_root_node; }

    /// \brief Check if the graph is empty.
    bool empty() const { return _root_node == nullptr; }

    /// \brief Find a value in the graph.
    /// \returns A reference to the node.
    std::optional<std::reference_wrapper<const Node>>
    find(const T &value) const {
        if (empty())
            return {};

        const Node *walker(&root_node());
        while (walker->value() != value) {
            if (walker->value() >= value) {
                if (walker->has_left_child())
                    walker = &walker->left_child();
                else
                    return {};
            } else {
                if (walker->has_right_child())
                    walker = &walker->right_child();
                else
                    return {};
            }
        }

        return *walker;
    }

    /// \brief Clear the graph.
    void clear() { _root_node.reset(); }

    /// \brief Remove the subgraph that has the root that holds the value.
    void remove_node(const T &value) {
        if (auto node = find(value)) {
            remove_node(*node);
        }
    }

    /// \copydoc remove_node
    void remove_node(Node &node) { node.get_as_child_ref().reset(); }

    /// \brief Remove the node that holds the given value.
    void remove_value(const T &value) {
        Node *walker = _root_node.get();
        while (walker != nullptr) {
            if (walker->value() > value)
                walker = walker->_left.get();
            else if (walker->value() < value)
                walker = walker->_right.get();
            else { // found what we are looking for
                if (!walker->has_left_child() && !walker->has_right_child())
                    walker->get_as_child_ref().reset();
                else if (!walker->has_left_child()) {
                    auto &child_ref = walker->get_as_child_ref();
                    std::swap(child_ref, walker->_right);
                    child_ref->_parent = walker->_right->_parent;
                    walker->_right.reset();
                } else if (!walker->has_right_child()) {
                    auto &child_ref = walker->get_as_child_ref();
                    std::swap(child_ref, walker->_left);
                    child_ref->_parent = walker->_left->_parent;
                    walker->_left.reset();
                } else {
                    InOrder::ForwardIterator<InOrder::View<BinarySearchTree<T>>>
                        iterator(*this, walker);
                    ++iterator;

                    walker->_value = std::move(iterator->_value);
                    if (iterator->has_right_child()) {
                        iterator->_right->_parent = iterator->_parent;
                        iterator->get_as_child_ref() =
                            std::move(iterator->_right);
                    } else {
                        iterator->get_as_child_ref().reset();
                    }
                }
                walker = nullptr;
            }
        }
    }

    /// \brief Check if the graph contains a specific value.
    bool contains(const T &value) const { return find(value).has_value(); }

    /// \brief Insert a value to the graph.
    /// \returns Reference to the node that was created.
    const Node &insert(const T &value) {
        std::reference_wrapper<std::unique_ptr<Node>> walker(_root_node);
        Node *parent = nullptr;
        while (walker.get() != nullptr) {
            parent = walker.get().get();
            if (value > walker.get()->value()) {
                walker = walker.get()->_right;
            } else {
                walker = walker.get()->_left;
            }
        }

        walker.get() = std::make_unique<Node>(*this, parent, value);

        return *walker.get();
    }
}; // namespace Core
} // namespace Core

#endif // CORE_BINARYSEARCHTREE_HPP
