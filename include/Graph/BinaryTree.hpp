//
// Created by Dániel Molnár on 2019-08-20.
//

#pragma once
#ifndef CORE_BINARYTREE_HPP
#define CORE_BINARYTREE_HPP

// ----- std -----
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <vector>

// ----- libraries -----

// ----- in-project dependencies -----
#include <Graph/Views.hpp>

// ----- forward-decl -----

namespace Core {
/// \brief Holds a binary tree graph with access of its nodes and other
/// utilities
template <class T> class BinaryTree {
  public:
    template <class U> friend class BFS::View;
    template <class U> friend class BFS::ForwardIterator;
    template <class U> friend class BFS::BackwardIterator;
    template <class U> friend class InOrder::View;
    template <class U> friend class InOrder::ForwardIterator;
    template <class U> friend class InOrder::BackwardIterator;
    template <class U> friend class DFS::View;
    template <class U> friend class DFS::ForwardIterator;
    template <class U> friend class DFS::BackwardIterator;

    /// \brief Internal representation of a node. Non-copyable, since it holds
    /// its owners as well
    class Node {
      protected:
        friend class BinaryTree;

        /// \brief Reference to the holder of the node
        BinaryTree &_owner;

        /// \brief Index of the node in the owner's array-representation
        unsigned int _index;

        /// \brief Value of the node
        T _value;

        /// \brief Get the index of the left child
        /// \param index Index of the node
        static constexpr unsigned int index_of_left(unsigned int index) {
            return index * 2 + 1;
        }

        /// \brief Get the index of the left child
        /// \param index Index of the node
        static constexpr unsigned int index_of_left(const Node &node) {
            return index_of_left(node._index);
        }

        /// \brief Get the index of the right child
        /// \param index Index of the node
        static constexpr unsigned int index_of_right(unsigned int index) {
            return index * 2 + 2;
        }

        /// \brief Get the index of the left child
        /// \param index Index of the node
        static constexpr unsigned int index_of_right(const Node &node) {
            return index_of_right(node._index);
        }

        /// \brief Get the index of the parent
        /// \param index Index of the node
        static constexpr unsigned int index_of_parent(unsigned int index) {
            // special case, parent is the root
            if (index == 1u || index == 2u)
                return 0u;

            return std::floor((index - (index % 2 == 0 ? 2u : 1u)) / 2.0);
        }

      public:
        /// \brief Construct a node
        /// \param owner The graph containing the node
        /// \param index Index of the node in the graph's array [internal use
        /// only]
        /// \param value The value it should store
        Node(BinaryTree &owner, unsigned int index, const T &value)
            : _owner(owner), _index(index), _value(value) {}

        /// \brief Construct a node. Enables move-construction
        /// \param owner The graph containing the node
        /// \param index Index of the node in the graph's array [internal use
        /// only]
        /// \param value The value it should store
        Node(BinaryTree &owner, unsigned int index, T &&value)
            : _owner(owner), _index(index), _value(value) {}

        Node(const Node &) = delete;
        Node &operator=(const Node &) = delete;
        Node(Node &&) = delete;
        Node &operator=(Node &&) = delete;

        /// \brief Destructs the node and its children
        ~Node() {
            _owner.remove_at(2 * _index + 1);
            _owner.remove_at(2 * _index + 2);
        }

        /// \brief Check if two nodes are different.
        bool operator!=(const Node &other) const { return !(*this == other); }

        /// \brief Check if two nodes are the same.
        bool operator==(const Node &other) const {
            return std::addressof(_owner) == std::addressof(other._owner) &&
                   _index == other._index;
        }

        /// \brief Check if node holds a specific value
        bool operator!=(const T &value) const { return !(*this == value); }

        /// \brief Check if node holds a specific value
        bool operator==(const T &value) const { return _value == value; }

        /// \brief Get the parent node.
        /// \throws std::out_of_range if no parent exists (root node only)
        Node &parent() {
            if (_index == 0u)
                throw std::out_of_range("Root node has no parent");

            return *_owner._nodes[index_of_parent(_index)];
        }

        const Node &parent() const {
            if (_index == 0u)
                throw std::out_of_range("Root node has no parent");

            return *_owner._nodes[index_of_parent(_index)];
        }

        /// \brief Check if node is the left child of its parent
        [[nodiscard]] bool is_left_child() const { return _index % 2 == 1; }

        /// \brief Check if node is the right child of its parent
        [[nodiscard]] bool is_right_child() const {
            return _index != 0 && _index % 2 == 0;
        }

        /// \brief Check if node has parent
        [[nodiscard]] bool has_parent() const {
            return _index != 0 &&
                   _owner._nodes[index_of_parent(_index)] != nullptr;
        }

        /// \brief Check if node has left child
        [[nodiscard]] bool has_left_child() const {
            return _owner._nodes.size() > index_of_left(_index) &&
                   _owner._nodes[index_of_left(_index)] != nullptr;
        }

        /// \brief Check if node has right child
        [[nodiscard]] bool has_right_child() const {
            return _owner._nodes.size() > index_of_right(_index) &&
                   _owner._nodes[index_of_right(_index)] != nullptr;
        }

        /// \brief Access left child.
        const Node &left_child() const {
            return *_owner._nodes.at(index_of_left(_index));
        }

        /// \brief Access right child.
        const Node &right_child() const {
            return *_owner._nodes.at(index_of_right(_index));
        }

        /// \copydoc Node::left_child
        Node &left_child() { return *_owner._nodes.at(index_of_left(_index)); }

        /// \copydoc Node::right_child
        Node &right_child() {
            return *_owner._nodes.at(index_of_right(_index));
        }

        /// \brief Access held value
        const T &value() const { return _value; }

        /// \copydoc Node::value
        T &value() { return _value; }

        /// \brief Set value
        void set_value(const T &value) { _value = value; }
    };

  protected:
    /// \brief Index of unoccupied nodes. Needed for fast-insert and find
    std::set<unsigned int> _unoccupied_nodes;

    /// \brief Array representation of the node
    std::vector<std::unique_ptr<Node>> _nodes;

    /// \brief Index of the left child of the node.
    static constexpr unsigned int index_of_left(const Node &node) {
        return Node::index_of_left(node);
    }

    /// \brief Index of the right child of the node.
    static constexpr unsigned int index_of_right(const Node &node) {
        return Node::index_of_right(node);
    }

    /// \brief Remove node with a specific index.
    /// Internal use only, since client code has no access to indices.
    void remove_at(unsigned int index) {
        if (_nodes.size() > index) {
            _nodes[index] = nullptr;
            _unoccupied_nodes.insert(index);
        }
    }

  public:
    using value_type = T;

    /// \brief Search policies to use in case of search.
    enum class SearchPolicy { DepthFirst, BreadthFirst, InOrder };

    /// \brief Default constructor.
    BinaryTree() = default;

    /// \brief Copy a graph from another.
    BinaryTree(const BinaryTree &other) {
        _nodes.clear();
        _unoccupied_nodes = other._unoccupied_nodes;
        _nodes.reserve(other._nodes.size());

        for (const auto &node : other._nodes) {
            _nodes.emplace_back(
                std::make_unique<Node>(*this, _nodes.size(), node->value()));
        }
    }

    /// \brief Copy-assignment operator.
    BinaryTree &operator=(const BinaryTree &other) {
        _nodes.clear();
        _unoccupied_nodes = other._unoccupied_nodes;
        _nodes.reserve(other._nodes.size());

        for (const auto &node : other._nodes) {
            _nodes.emplace_back(
                std::make_unique<Node>(*this, _nodes.size(), node->value()));
        }

        return *this;
    }

    /// \brief Move construction.
    BinaryTree(BinaryTree &&other) noexcept {
        _unoccupied_nodes = std::move(other._unoccupied_nodes);
        _nodes = std::move(other._nodes);
    }

    /// \brief Move assignment.
    BinaryTree &operator=(BinaryTree &&other) noexcept {
        _unoccupied_nodes = std::move(other._unoccupied_nodes);
        _nodes = std::move(other._nodes);
    }

    /// \brief Check if two graphs are different
    bool operator!=(const BinaryTree &other) const { return !(*this == other); }

    /// \brief Check if two graphs are the same.
    /// Empty nodes are not considered as inequality
    bool operator==(const BinaryTree &other) const {
        if (size() != other.size()) {
            return false;
        }

        for (unsigned int i = 0; i < _nodes.size(); ++i) {
            if (((_nodes[i] == nullptr) !=
                 (other._nodes[i] ==
                  nullptr)) || // one is nullptr other is not or
                (_nodes[i] &&
                 _nodes[i]->value() !=
                     other._nodes[i]->value())) // both not nullptr and the
                                                // value is not the same
            {
                return false;
            }
        }

        return true;
    }

    /// \brief Check if graph has any node filled
    [[nodiscard]] bool empty() const { return size() == 0; }

    /// \brief Get the amount of occupied nodes.
    [[nodiscard]] unsigned int size() const {
        return _nodes.size() - _unoccupied_nodes.size();
    }

    void reserve(unsigned int size) { _nodes.reserve(size); }

    /// \brief Clear the graph.
    void clear() {
        _nodes.clear();
        _unoccupied_nodes.clear();
    }

    /// \brief Access root node.
    const Node &root_node() const { return *_nodes.at(0); }

    /// \copydoc root_node
    Node &root_node() { return *_nodes.at(0); }

    /// \brief Insertion. If there is an unoccupied one, the inserted value
    /// takes its place.
    virtual Node &insert(const T &value) {
        // if there is an "empty" node, use it
        if (!_unoccupied_nodes.empty()) {
            auto place =
                _unoccupied_nodes.extract(_unoccupied_nodes.begin()).value();
            _nodes.at(place) = std::make_unique<Node>(*this, place, value);
            return *_nodes.at(place);
        } else {
            // add new node
            return *_nodes.emplace_back(
                std::make_unique<Node>(*this, _nodes.size(), value));
        }
    }

    /// \brief Insertion, enables move. If there is an unoccupied one, the
    /// inserted value takes its place.
    virtual Node &insert(T &&value) {
        // if there is an "empty" node, use it
        if (!_unoccupied_nodes.empty()) {
            auto place =
                _unoccupied_nodes.extract(_unoccupied_nodes.begin()).value();
            _nodes.at(place) = std::make_unique<Node>(*this, place, value);
            return *_nodes.at(place);
        } else {
            // add new node
            return *_nodes.emplace_back(
                std::make_unique<Node>(*this, _nodes.size(), value));
        }
    }

    /// \brief Find an element in the graph, if present.
    /// \param value The value to look for.
    /// \param policy Determines which order the nodes are encountered.
    /// \returns Optionally the node which contains the element
    virtual std::optional<std::reference_wrapper<Node>>
    find(const T &value, SearchPolicy policy = SearchPolicy::DepthFirst) {
        switch (policy) {
        case SearchPolicy::BreadthFirst: {
            for (auto &node : BFS::View(*this)) {
                if (node.value() == value) {
                    return node;
                }
            }
            return std::nullopt;
        }
        case SearchPolicy::DepthFirst: {
            for (auto &node : DFS::View(*this)) {
                if (node.value() == value) {
                    return node;
                }
            }
            return std::nullopt;
        }
        case SearchPolicy::InOrder: {
            for (auto &node : InOrder::View(*this)) {
                if (node.value() == value) {
                    return node;
                }
            }
            return std::nullopt;
        }
            // Intentionally no default path
        }
        // Can not happen, but compilers complain anyway so need a return
        return std::nullopt;
    }

    /// \brief Check if a value is in the graph.
    /// \param policy The order in which the nodes are encountered.
    /// \param value The value to look for.
    bool contains(const T &value,
                  SearchPolicy policy = SearchPolicy::DepthFirst) {
        return find(value, policy).has_value();
    }

    /// \brief Removes the sub-graph the node represents. Behaviour is undefined
    /// if the nodes owner is not this graph.
    void remove_node(const Node &node) { remove_at(node._index); }

    /// \brief Remove the sub-graph that the first found node that holds the
    /// value represents
    void remove_node(const T &value,
                     SearchPolicy policy = SearchPolicy::DepthFirst) {
        if (auto node = find(value, policy)) {
            remove_node(*node);
        }
    }

    /// \brief Remove a value from this graph. Moves other values forward (in
    /// breadth-first traversal)
    void remove_value(const T &value,
                      SearchPolicy policy = SearchPolicy::DepthFirst) {
        if (auto node = find(value, policy)) {
            remove_value(*node);
        }
    }

    /// \copydoc remove_value
    void remove_value(const Node &node) {
        for (auto i = node._index + 1; i < _nodes.size(); ++i) {
            if (_nodes[i] != nullptr) {
                _nodes[i]->_index = i - 1;
                _unoccupied_nodes.erase(i - 1);
            } else {
                _unoccupied_nodes.insert(i - 1);
            }

            // i - 1 can not be nullptr, since we are moving node towards the
            // end of the array
            _nodes[i - 1]->_index = i;
            _unoccupied_nodes.erase(i);

            std::swap(_nodes.at(i - 1), _nodes.at(i));
        }
        _nodes.back() = nullptr;
        _unoccupied_nodes.insert(_nodes.size() - 1);
    }
};

} // namespace Core

#endif // CORE_BINARYTREE_HPP
