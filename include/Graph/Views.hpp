//
// Created by Dániel Molnár on 2019-08-25.
//

#ifndef CORE_VIEWS_HPP
#define CORE_VIEWS_HPP

// ----- std -----
#include <memory>

template <class T, class = void> struct TypeDescriptor {
    using graph_type_ref = T &;
    using node_type = typename std::remove_reference_t<T>::Node;
    using node_type_ref = node_type &;
};

template <class T>
struct TypeDescriptor<T, typename std::enable_if_t<std::is_const_v<T>>> {
    using graph_type_ref = const T &;
    using node_type = const typename T::Node;
    using node_type_ref = node_type &;
};

namespace Core::DFS {
template <class View> class ForwardIterator;

template <class View> class BackwardIterator {
  private:
    friend class ForwardIterator<View>;

    typename View::graph_type_ref _viewed_obj;
    typename View::node_type *_viewed_node;

  public:
    BackwardIterator(typename View::graph_type_ref graph,
                     typename View::node_type *node)
        : _viewed_obj(graph), _viewed_node(node) {}

    bool operator!=(const BackwardIterator &other) { return !(*this == other); }

    bool operator==(const BackwardIterator &other) {
        return std::addressof(_viewed_obj) ==
                   std::addressof(other._viewed_obj) &&
               _viewed_node == other._viewed_node;
    }

    BackwardIterator operator--(int) {
        BackwardIterator result{_viewed_obj, _viewed_node};

        --*(this);

        return result;
    }

    BackwardIterator &operator--() {
        if (!_viewed_node) { // at the end, we need the first forward it
            _viewed_node =
                _viewed_obj.empty() ? nullptr : &_viewed_obj.root_node();
        } else {
            ForwardIterator<View> forward_this{_viewed_obj, _viewed_node};

            forward_this++;

            _viewed_node = forward_this._viewed_node;
        }
        return *this;
    }

    BackwardIterator operator++(int) {
        BackwardIterator result{_viewed_obj, _viewed_node};

        ++*(this);

        return result;
    }

    BackwardIterator &operator++() {
        static auto try_get_left_childs_rightmosts_leftmost = [](auto node) {
            if (node->has_left_child()) {
                node = &node->left_child();

                while (node->has_left_child() || node->has_right_child()) {
                    node = node->has_right_child() ? &node->right_child()
                                                   : &node->left_child();
                }
            }
            return node;
        };

        if (_viewed_node->is_left_child() && _viewed_node->has_parent()) {
            _viewed_node = &_viewed_node->parent();
        } else if (_viewed_node->is_right_child() &&
                   _viewed_node->has_parent()) { // right child or root
            _viewed_node = try_get_left_childs_rightmosts_leftmost(
                &_viewed_node->parent());
        } else { // root node
            _viewed_node = nullptr;
        }

        return *this;
    }

    typename View::node_type_ref operator*() const { return *_viewed_node; }

    typename View::node_type *operator->() const { return _viewed_node; }
};

template <class View> class ForwardIterator {
  private:
    friend class BackwardIterator<View>;

    typename View::graph_type_ref _viewed_obj;
    typename View::node_type *_viewed_node;

  public:
    ForwardIterator(typename View::graph_type_ref graph,
                    typename View::node_type *node)
        : _viewed_obj(graph), _viewed_node(node) {}

    bool operator!=(const ForwardIterator &other) { return !(*this == other); }

    bool operator==(const ForwardIterator &other) {
        return std::addressof(_viewed_obj) ==
                   std::addressof(other._viewed_obj) &&
               _viewed_node == other._viewed_node;
    }

    ForwardIterator operator--(int) {
        ForwardIterator result{_viewed_obj, _viewed_node};

        --*(this);

        return result;
    }

    ForwardIterator &operator--() {
        if (!_viewed_node) {
            if (!_viewed_obj.empty()) {
                std::reference_wrapper<typename View::node_type> walker =
                    _viewed_obj.root_node();

                while (walker.get().has_right_child()) {
                    walker = walker.get().right_child();
                }
                while (walker.get().has_left_child()) {
                    walker = walker.get().left_child();
                }
                _viewed_node = &walker.get();
            } else {
                _viewed_node = nullptr;
            }
        } else {
            BackwardIterator<View> backward_this{_viewed_obj, _viewed_node};

            backward_this++;

            _viewed_node = backward_this._viewed_node;
        }
        return *this;
    }

    ForwardIterator operator++(int) {
        ForwardIterator result{_viewed_obj, _viewed_node};

        ++*(this);

        return result;
    }

    ForwardIterator &operator++() {
        static auto try_get_first_left_child_parent = [](auto node) {
            while (node->is_right_child() && node->has_parent()) {
                node = &node->parent();
            }
            return node;
        };
        static auto try_get_first_parent_right_child = [](auto node) {
            auto prev_node = node;
            do {
                prev_node = node;
                node = &node->parent();
            } while (node->has_parent() && (!node->has_right_child() ||
                                            node->right_child() == *prev_node));
            return !node->has_right_child() || node->right_child() == *prev_node
                       ? nullptr
                       : &node->right_child();
        };

        if (_viewed_node->has_left_child()) {
            _viewed_node = &_viewed_node->left_child();
        } else if (_viewed_node->has_right_child()) {
            _viewed_node = &_viewed_node->right_child();
        } else if (_viewed_node->is_left_child() &&
                   _viewed_node->has_parent()) {
            _viewed_node = try_get_first_parent_right_child(_viewed_node);
        } else { // right child or root
            _viewed_node = try_get_first_left_child_parent(_viewed_node);
            if (_viewed_node->has_parent()) {
                _viewed_node = try_get_first_parent_right_child(_viewed_node);
            } else
                _viewed_node = nullptr;
        }

        return *this;
    }

    typename View::node_type_ref operator*() const { return *_viewed_node; }

    typename View::node_type *operator->() const { return _viewed_node; }
};

template <class T> class View {
  private:
    typename TypeDescriptor<T>::graph_type_ref _graph;

  public:
    using graph_type_ref = typename TypeDescriptor<T>::graph_type_ref;
    using node_type = typename TypeDescriptor<T>::node_type;
    using node_type_ref = typename TypeDescriptor<T>::node_type_ref;

    explicit View(T &graph) : _graph(graph) {}

    ForwardIterator<View> begin() const {
        if (_graph.empty())
            return end();
        return {_graph, &_graph.root_node()};
    }

    ForwardIterator<View> end() const { return {_graph, nullptr}; }

    BackwardIterator<View> rbegin() const {
        if (!_graph.empty()) {
            std::reference_wrapper<node_type> walker = _graph.root_node();

            while (walker.get().has_right_child()) {
                walker = walker.get().right_child();
            }
            while (walker.get().has_left_child()) {
                walker = walker.get().left_child();
            }

            return {_graph, &walker.get()};
        }
        return rend();
    }

    BackwardIterator<View> rend() const { return {_graph, nullptr}; }
};

template <class T> View(T &)->View<T>;
template <class T> View(const T &)->View<const T>;

} // namespace Core::DFS

namespace Core::BFS {
template <class View> class ForwardIterator;

template <class View> class BackwardIterator {
  private:
    friend class ForwardIterator<View>;

    long long _index;
    typename View::graph_type_ref _viewed_obj;

  public:
    BackwardIterator(long long index, typename View::graph_type_ref graph)
        : _index(index), _viewed_obj(graph) {}

    bool operator!=(const BackwardIterator &other) { return !(*this == other); }

    bool operator==(const BackwardIterator &other) {
        return std::addressof(_viewed_obj) ==
                   std::addressof(other._viewed_obj) &&
               _index == other._index;
    }

    BackwardIterator operator--(int) {
        BackwardIterator result{_index, _viewed_obj};

        --*(this);

        return result;
    }

    BackwardIterator &operator--() {
        if (!_viewed_obj.empty()) {
            ForwardIterator<View> forward_this{_index, _viewed_obj};
            ++forward_this;

            _index = forward_this._index;
        }

        return *this;
    }

    BackwardIterator operator++(int) {
        BackwardIterator<View> result{_index, _viewed_obj};

        ++*(this);

        return result;
    }

    BackwardIterator &operator++() {
        do {
            _index--;
        } while (_index > 0 && _viewed_obj._nodes.at(_index) == nullptr);

        return *this;
    }

    typename View::node_type &operator*() const {
        return *_viewed_obj._nodes.at(_index);
    }

    typename View::node_type *operator->() const {
        return _viewed_obj._nodes.at(_index).get();
    }
};

template <class View> class ForwardIterator {
  private:
    friend class BackwardIterator<View>;

    long long _index;
    typename View::graph_type_ref _viewed_obj;

  public:
    ForwardIterator(long long index, typename View::graph_type_ref graph)
        : _index(index), _viewed_obj(graph) {}

    bool operator!=(const ForwardIterator &other) { return !(*this == other); }

    bool operator==(const ForwardIterator &other) {
        return std::addressof(_viewed_obj) ==
                   std::addressof(other._viewed_obj) &&
               _index == other._index;
    }

    ForwardIterator operator--(int) {
        ForwardIterator result{_index, _viewed_obj};

        --*(this);

        return result;
    }

    ForwardIterator &operator--() {
        if (!_viewed_obj.empty()) {
            BackwardIterator<View> backward_this = {_index, _viewed_obj};

            ++backward_this;

            _index = std::max(0ll, backward_this._index);
        }

        return *this;
    }

    ForwardIterator operator++(int) {
        ForwardIterator result{_index, _viewed_obj};

        ++*(this);

        return result;
    }

    ForwardIterator &operator++() {
        do {
            _index++;
        } while (_index < static_cast<long long>(_viewed_obj._nodes.size()) &&
                 _viewed_obj._nodes.at(_index) == nullptr);

        return *this;
    }

    typename View::node_type_ref operator*() const {
        return *_viewed_obj._nodes.at(_index);
    }

    typename View::node_type *operator->() const {
        return _viewed_obj._nodes.at(_index).get();
    }
};

template <class T> class View {
  private:
    typename TypeDescriptor<T>::graph_type_ref _graph;

  public:
    using graph_type_ref = typename TypeDescriptor<T>::graph_type_ref;
    using node_type = typename TypeDescriptor<T>::node_type;
    using node_type_ref = typename TypeDescriptor<T>::node_type_ref;

    explicit View(T &graph) : _graph(graph) {}

    ForwardIterator<View> begin() const {
        if (_graph.empty())
            return end();
        return {0, _graph};
    }

    ForwardIterator<View> end() const {
        return {static_cast<long long>(_graph._nodes.size()), _graph};
    }

    BackwardIterator<View> rbegin() const {
        if (!_graph.empty()) {
            for (long long i = _graph._nodes.size() - 1; i >= 0ll; --i) {
                if (_graph._nodes.at(i) != nullptr)
                    return {static_cast<long long>(i), _graph};
            }
        }
        return rend();
    }

    BackwardIterator<View> rend() const { return {-1l, _graph}; }
};

template <class T> View(T &)->View<T>;
template <class T> View(const T &)->View<const T>;

} // namespace Core::BFS

namespace Core::InOrder {
template <class View> class BackwardIterator;

template <class View> class ForwardIterator {
  private:
    friend class BackwardIterator<View>;

    typename View::graph_type_ref _viewed_obj;
    typename View::node_type *_viewed_node;

    static constexpr typename View::node_type *
    get_left_most_of(typename View::node_type *node) {
        while (node->has_left_child()) {
            node = &node->left_child();
        }
        return node;
    }

  public:
    ForwardIterator(typename View::graph_type_ref graph,
                    typename View::node_type *node)
        : _viewed_obj(graph), _viewed_node(node) {}

    bool operator!=(const ForwardIterator &other) const {
        return !(*this == other);
    }

    bool operator==(const ForwardIterator &other) const {
        return _viewed_node == other._viewed_node;
    }

    ForwardIterator operator--(int) {
        ForwardIterator result{_viewed_obj, _viewed_node};

        --*(this);

        return result;
    }

    ForwardIterator &operator--() {
        if (!_viewed_node) { // at the end, we need the first backward it
            if (!_viewed_obj.empty()) {

                std::reference_wrapper<typename View::node_type> walker =
                    _viewed_obj.root_node();

                while (walker.get().has_right_child()) {
                    walker = walker.get().right_child();
                }

                _viewed_node = &walker.get();
            } else {
                _viewed_node = nullptr;
            }
        } else {
            BackwardIterator<View> backward_this{_viewed_obj, _viewed_node};

            backward_this++;

            _viewed_node = backward_this._viewed_node;
        }
        return *this;
    }

    ForwardIterator operator++(int) {
        ForwardIterator result{_viewed_obj, _viewed_node};

        ++*(this);

        return result;
    }

    ForwardIterator &operator++() {
        static auto try_get_first_left_child_parent = [](auto node) {
            while (node->is_right_child() && node->has_parent()) {
                node = &node->parent();
            }
            return node;
        };

        if (!_viewed_node) {
            return *this;
        }

        auto &node = *_viewed_node;

        if (node.has_right_child()) {
            _viewed_node = get_left_most_of(&node.right_child());
        } else {
            if (node.is_left_child()) {
                if (node.has_parent()) {
                    _viewed_node = &node.parent();
                }
            } else if (node.is_right_child()) {
                _viewed_node = try_get_first_left_child_parent(_viewed_node);
                if (_viewed_node->has_parent()) {
                    _viewed_node = &_viewed_node->parent();
                } else {
                    _viewed_node = nullptr;
                }
            } else { // root node
                _viewed_node = nullptr;
            }
        }

        return *this;
    }

    typename View::node_type_ref operator*() const { return *_viewed_node; }
    typename View::node_type *operator->() const { return _viewed_node; }
};

template <class View> class BackwardIterator {
  private:
    friend class ForwardIterator<View>;

    typename View::graph_type_ref _viewed_obj;
    typename View::node_type *_viewed_node;

    static constexpr typename View::node_type *
    get_right_most_of(typename View::node_type *node) {
        while (node->has_right_child()) {
            node = &node->right_child();
        }
        return node;
    }

  public:
    BackwardIterator(typename View::graph_type_ref graph,
                     typename View::node_type *node)
        : _viewed_obj(graph), _viewed_node(node) {}

    bool operator!=(const BackwardIterator &other) const {
        return !(*this == other);
    }

    bool operator==(const BackwardIterator &other) const {
        return _viewed_node == other._viewed_node;
    }

    BackwardIterator operator--(int) {
        BackwardIterator result{_viewed_node};

        --*(this);

        return result;
    }

    BackwardIterator &operator--() {
        if (!_viewed_node) { // at the end, we need the first forward it
            if (!_viewed_obj.empty()) {
                std::reference_wrapper<typename View::node_type> walker =
                    _viewed_obj.root_node();

                while (walker.get().has_left_child()) {
                    walker = walker.get().left_child();
                }

                _viewed_node = &walker.get();
            } else {
                _viewed_node = nullptr;
            }
        } else {
            ForwardIterator<View> forward_this{_viewed_obj, _viewed_node};

            forward_this++;

            _viewed_node = forward_this._viewed_node;
        }
        return *this;
    }

    BackwardIterator operator++(int) {
        BackwardIterator result{_viewed_obj, _viewed_node};

        ++*(this);

        return result;
    }

    BackwardIterator &operator++() {
        static auto try_get_first_right_child_parent = [](auto node) {
            while (node->is_left_child() && node->has_parent()) {
                node = &node->parent();
            }
            return node;
        };

        if (!_viewed_node) {
            return *this;
        }

        auto &node = *_viewed_node;

        if (node.has_left_child()) {
            _viewed_node = get_right_most_of(&node.left_child());

        } else {
            if (node.is_right_child()) {
                if (node.has_parent()) {
                    _viewed_node = &node.parent();
                }
            } else if (node.is_left_child()) {
                _viewed_node = try_get_first_right_child_parent(_viewed_node);
                if (_viewed_node->has_parent()) {
                    _viewed_node = &_viewed_node->parent();
                } else {
                    _viewed_node = nullptr;
                }
            } else { // root node
                _viewed_node = nullptr;
            }
        }

        return *this;
    }

    typename View::node_type_ref operator*() const { return *_viewed_node; }
    typename View::node_type *operator->() const { return _viewed_node; }
};

template <class T> class View {
  private:
    typename TypeDescriptor<T>::graph_type_ref _graph;
    friend class ForwardIterator<View>;
    friend class BackwardIterator<View>;

  public:
    using graph_type_ref = typename TypeDescriptor<T>::graph_type_ref;
    using node_type = typename TypeDescriptor<T>::node_type;
    using node_type_ref = typename TypeDescriptor<T>::node_type_ref;

    explicit View(graph_type_ref graph) : _graph(graph) {}

    ForwardIterator<View> begin() const {
        if (_graph.empty())
            return end();

        std::reference_wrapper<node_type> walker = _graph.root_node();

        while (walker.get().has_left_child()) {
            walker = walker.get().left_child();
        }

        return {_graph, &walker.get()};
    }

    ForwardIterator<View> end() const { return {_graph, nullptr}; }

    BackwardIterator<View> rbegin() const {
        if (_graph.empty())
            return rend();

        std::reference_wrapper<node_type> walker = _graph.root_node();

        while (walker.get().has_right_child()) {
            walker = walker.get().right_child();
        }

        return {_graph, &walker.get()};
    }

    BackwardIterator<View> rend() const { return {_graph, nullptr}; }
};

template <class T> View(T &)->View<T>;
template <class T> View(const T &)->View<const T>;

} // namespace Core::InOrder

#endif // CORE_VIEWS_HPP
