//
// Created by Dániel Molnár on 2019-08-20.
//

#include <gtest/gtest.h>

#include <Graph/BinaryTree.hpp>
#include <Graph/Views.hpp>

using namespace Core;

TEST(BinaryTree, root_node) {
    BinaryTree<int> graph;

    ASSERT_THROW(graph.root_node(), std::out_of_range);
    graph.insert(0);
    ASSERT_EQ(graph.root_node().value(), 0);
}

TEST(BinaryTree, insertion) {
    BinaryTree<float> graph;

    ASSERT_TRUE(graph.empty());
    auto item = 125.90f;
    graph.insert(item);
    graph.insert(2.f);
    graph.insert(3.f);
    graph.insert(4.f);
    graph.insert(5.f);

    graph.insert(6.f);
    graph.remove_node(5);
    item = 6.f;
    graph.insert(item);

    ASSERT_EQ(graph.size(), 6);
    ASSERT_FALSE(graph.empty());

    graph.clear();

    ASSERT_TRUE(graph.empty());
}

TEST(BinaryTree, node_removal) {
    struct dummy {
        bool operator!=(const dummy& other) const {
            return !(*this == other);
        }
        bool operator==(const dummy&) const {
            return true;
        }
    };
    BinaryTree<dummy> graph;

    graph.insert({});
    graph.insert({});
    const auto &second = graph.insert({});
    graph.insert({});
    graph.insert({});
    graph.insert({});
    graph.insert({});

    ASSERT_EQ(graph.size(), 7);

    graph.remove_node(second);

    ASSERT_EQ(graph.size(), 4);

    const auto &node = graph.insert({});
    ASSERT_EQ(graph.size(), 5);

    ASSERT_EQ(node, graph.root_node().right_child());

    graph.remove_node(graph.root_node());

    ASSERT_TRUE(graph.empty());
}

TEST(BinaryTree, value_removal) {
    BinaryTree<int> tree;

    tree.insert(0);
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    tree.insert(4);
    tree.insert(5);
    tree.remove_value(2);
    tree.remove_value(4);

    ASSERT_EQ(tree.size(), 4);

    tree.remove_node(tree.root_node());

    for (int i = 0; i < 100; ++i) {
        tree.insert(i);
    }

    ASSERT_EQ(tree.size(), 100);
    for (int i = 49; i >= 0; --i) {
        tree.remove_value(i);
        ASSERT_EQ(tree.size(), i + 50);
    }
    for (int i = 50; i < 100; ++i) {
        tree.remove_value(i);
        ASSERT_EQ(tree.size(), 100-1-i );
    }
}

TEST(BinaryTree, relations) {
    BinaryTree<int> graph;

    const auto &root = graph.insert(-1);
    auto &root_noconst = graph.root_node();
    auto &left = graph.insert(2);
    auto &right = graph.insert(4);

    ASSERT_EQ(root.left_child(), left);
    ASSERT_EQ(root.right_child(), right);
    ASSERT_EQ(right.parent(), left.parent());
    ASSERT_EQ(root, right.parent());

    const auto &left_of_left = graph.insert(-2);
    const auto &right_of_left = graph.insert(-5);
    const auto &left_of_right = graph.insert(-7);
    const auto &right_of_right = graph.insert(-9);

    ASSERT_EQ(left_of_left, left.left_child());
    ASSERT_EQ(left_of_left.parent(), left);

    ASSERT_EQ(right_of_left, left.right_child());
    ASSERT_EQ(right_of_left.parent(), left);

    ASSERT_EQ(left_of_right, right.left_child());
    ASSERT_EQ(left_of_right.parent(), right);

    ASSERT_EQ(right_of_right, right.right_child());
    ASSERT_EQ(right_of_right.parent(), right);

    ASSERT_THROW(root.parent(), std::out_of_range);
    ASSERT_THROW(root_noconst.parent(), std::out_of_range);
}

TEST(BinaryTree, value) {
    BinaryTree<double> graph;

    graph.insert(2.);

    auto &left = graph.insert(21.6);
    const auto &right = graph.insert(21.5 / 60.2);

    ASSERT_DOUBLE_EQ(graph.root_node().value(), 2.);
    ASSERT_DOUBLE_EQ(left.value(), 21.6);
    left.set_value(-1);
    ASSERT_DOUBLE_EQ(left.value(), -1);
    ASSERT_DOUBLE_EQ(graph.root_node().left_child().value(), -1);

    ASSERT_DOUBLE_EQ(right.value(), 21.5 / 60.2);
}

TEST(BinaryTree, equality) {
    BinaryTree<double> graph_one;
    BinaryTree<double> graph_two;

    ASSERT_EQ(graph_one, graph_two);
    graph_one.insert(-1);
    ASSERT_NE(graph_one, graph_two);
    graph_two.insert(2);
    ASSERT_NE(graph_one, graph_two);
    graph_one.root_node().set_value(2);
    ASSERT_EQ(graph_one, graph_two);

    auto &wrong_node = graph_two.insert(11);
    graph_one.insert(12);

    for (auto i = 0; i < 100; ++i) {
        graph_two.insert(i);
        graph_one.insert(i);
    }

    ASSERT_NE(graph_one, graph_two);
    wrong_node.set_value(12);
    ASSERT_EQ(graph_one, graph_two);

    BinaryTree<int> graph;
    for (auto i = 0; i < 1000; i++) {
        graph.insert(i);
    }

    auto copy = graph;
    ASSERT_EQ(copy.size(), graph.size());
    ASSERT_EQ(copy, graph);
    graph.remove_node(990);
    ASSERT_NE(copy, graph);
    copy.remove_node(990);
    ASSERT_EQ(copy, graph);
}

TEST(BinaryTree, traversal) {
    const auto do_dfs = [](auto &graph, auto &result) {
        result.clear();
        std::vector<
            typename std::remove_reference<decltype(graph)>::type::value_type>
            alt_result = {};
        for (auto &node : DFS::View(graph)) {
            result.push_back(node.value());
        }
        auto view = DFS::View(graph);
        for (auto it = --view.rend(); it != view.rbegin(); --it) {
            alt_result.push_back(it->value());
        }
        // begin() won't get pushed, pushing it manually if have to
        if (view.rbegin() != view.rend())
            alt_result.push_back(view.rbegin()->value());

        EXPECT_EQ(alt_result, result);
    };

    const auto do_const_dfs = [](const auto &graph, auto &result) {
        result.clear();
        for (auto &node : DFS::View(graph)) {
            result.push_back(node.value());
        }
    };

    // Makes sure that iterators do not depend on their respective views.
    const auto do_reverse_dfs = [](auto &graph, auto &result) {
        using CurrentView = DFS::View<decltype(graph)>;
        result.clear();
        std::vector<
            typename std::remove_reference_t<decltype(graph)>::value_type>
            alt_result = {};
        auto rbeg = CurrentView(graph).rbegin();
        auto rend = CurrentView(graph).rend();

        for (auto it = rbeg; it != rend; ++it) {
            result.push_back(it->value());
        }

        auto beg = CurrentView(graph).begin(); // temporary view!
        auto end = CurrentView(graph).end();
        for (auto it = --end; it != beg; --it) {
            alt_result.push_back(it->value());
        }
        // begin() won't get pushed, pushing it manually if have to
        if (beg != CurrentView(graph).end())
            alt_result.push_back(beg->value());

        EXPECT_EQ(alt_result, result);
    };

    const auto dfs = [&do_reverse_dfs, &do_const_dfs, &do_dfs](auto &graph,
                                                               auto &expected) {
        std::vector<
            typename std::remove_reference_t<decltype(graph)>::value_type>
            result;
        do_dfs(graph, result);
        ASSERT_EQ(expected, result);

        do_const_dfs(graph, result);
        ASSERT_EQ(expected, result);

        do_reverse_dfs(graph, result);
        std::reverse(expected.begin(), expected.end());
        ASSERT_EQ(expected, result);
    };

    const auto do_bfs = [](auto &graph, auto &result) {
        result.clear();
        std::vector<
            typename std::remove_reference_t<decltype(graph)>::value_type>
            alt_result = {};
        for (auto &node : BFS::View(graph)) {
            result.push_back(node.value());
        }
        auto view = BFS::View(graph);
        for (auto it = --view.rend(); it != view.rbegin(); --it) {
            alt_result.push_back(it->value());
        }
        // begin() won't get pushed, pushing it manually if have to
        if (view.rbegin() != view.rend())
            alt_result.push_back(view.rbegin()->value());

        EXPECT_EQ(alt_result, result);
    };

    const auto do_reverse_bfs = [](auto &graph, auto &result) {
        using CurrentView = BFS::View<decltype(graph)>;
        result.clear();
        std::vector<
            typename std::remove_reference_t<decltype(graph)>::value_type>
            alt_result = {};
        auto rbeg = CurrentView(graph).rbegin();
        auto rend = CurrentView(graph).rend();

        for (auto it = rbeg; it != rend; ++it) {
            result.push_back(it->value());
        }

        auto beg = CurrentView(graph).begin();
        auto end = CurrentView(graph).end();
        for (auto it = --end; it != beg; --it) {
            alt_result.push_back(it->value());
        }
        // begin() won't get pushed, pushing it manually if have to
        if (beg != CurrentView(graph).end())
            alt_result.push_back(beg->value());

        EXPECT_EQ(alt_result, result);
    };

    const auto do_const_bfs = [](const auto &graph, auto &result) {
        result.clear();
        for (auto &node : BFS::View(graph)) {
            result.push_back(node.value());
        }
    };

    const auto bfs = [&do_reverse_bfs, &do_const_bfs, &do_bfs](auto &graph,
                                                               auto &expected) {
        std::vector<
            typename std::remove_reference_t<decltype(graph)>::value_type>
            result;
        do_bfs(graph, result);
        ASSERT_EQ(expected, result);

        do_const_bfs(graph, result);
        ASSERT_EQ(expected, result);

        do_reverse_bfs(graph, result);
        std::reverse(expected.begin(), expected.end());
        ASSERT_EQ(expected, result);
    };

    const auto do_inorder = [](auto &graph, auto &result) {
        result.clear();
        std::vector<
            typename std::remove_reference_t<decltype(graph)>::value_type>
            alt_result = {};
        for (auto &node : InOrder::View(graph)) {
            result.push_back(node.value());
        }

        auto view = InOrder::View(graph);
        for (auto it = --view.rend(); it != view.rbegin(); --it) {
            alt_result.push_back(it->value());
        }
        // begin() won't get pushed, pushing it manually if have to
        if (view.rbegin() != view.rend())
            alt_result.push_back(view.rbegin()->value());

        EXPECT_EQ(alt_result, result);
    };

    const auto do_reverse_inorder = [](auto &graph, auto &result) {
        using CurrentView = InOrder::View<decltype(graph)>;
        result.clear();
        std::vector<
            typename std::remove_reference_t<decltype(graph)>::value_type>
            alt_result = {};
        auto rbeg = CurrentView(graph).rbegin();
        auto rend = CurrentView(graph).rend();

        for (auto it = rbeg; it != rend; ++it) {
            result.push_back(it->value());
        }

        auto beg = CurrentView(graph).begin();
        auto end = CurrentView(graph).end();
        for (auto it = --end; it != beg; --it) {
            alt_result.push_back(it->value());
        }
        // begin() won't get pushed, pushing it manually if have to
        if (beg != CurrentView(graph).end())
            alt_result.push_back(beg->value());

        EXPECT_EQ(alt_result, result);
    };

    const auto do_const_inorder = [](const auto &graph, auto &result) {
        result.clear();
        for (auto &node : InOrder::View(graph)) {
            result.push_back(node.value());
        }
    };

    const auto inorder = [&do_reverse_inorder, &do_const_inorder,
                          &do_inorder](auto &graph, auto &expected) {
        std::vector<
            typename std::remove_reference_t<decltype(graph)>::value_type>
            result;
        do_inorder(graph, result);
        ASSERT_EQ(expected, result);

        do_const_inorder(graph, result);
        ASSERT_EQ(expected, result);

        do_reverse_inorder(graph, result);
        std::reverse(expected.begin(), expected.end());
        ASSERT_EQ(expected, result);
    };

    BinaryTree<double> graph;
    BinaryTree<double> copy = graph;

    std::vector<double> expected;

    // InOrder
    inorder(graph, expected);

    // BFS
    bfs(graph, expected);

    // DFS
    dfs(graph, expected);

    graph.insert(0.);
    expected = {0.};

    // InOrder
    inorder(graph, expected);

    // BFS
    bfs(graph, expected);

    // DFS
    dfs(graph, expected);

    copy = graph;
    copy.remove_node(0);
    expected = {};

    // InOrder
    inorder(copy, expected);

    // BFS
    bfs(copy, expected);

    // DFS
    dfs(copy, expected);

    graph.insert(1.);
    graph.insert(2.);
    graph.insert(3.);
    graph.insert(4.);
    graph.insert(5.);
    graph.insert(6.);
    graph.insert(7.);
    graph.insert(8.);
    graph.insert(9.);
    graph.insert(10.);
    graph.insert(11.);
    graph.insert(12.);
    graph.insert(13.);
    graph.insert(14.);

    expected = {7.,  3., 8.,  1., 9.,  4., 10., 0.,
                11., 5., 12., 2., 13., 6., 14.};

    // InOrder
    inorder(graph, expected);

    expected = {0., 1., 2.,  3.,  4.,  5.,  6., 7.,
                8., 9., 10., 11., 12., 13., 14.};

    // BFS
    bfs(graph, expected);

    expected = {0., 1., 3.,  7.,  8., 4.,  9., 10.,
                2., 5., 11., 12., 6., 13., 14.};

    // DFS
    dfs(graph, expected);

    copy = graph;
    copy.remove_node(1);

    expected = {0., 11., 5., 12., 2., 13., 6., 14.};

    // InOrder
    inorder(copy, expected);

    expected = {0., 2., 5., 6., 11., 12., 13., 14.};

    // BFS
    bfs(copy, expected);

    expected = {0., 2., 5., 11., 12., 6., 13., 14.};

    // DFS
    dfs(copy, expected);

    copy = graph;
    copy.remove_node(14);

    expected = {7., 3., 8., 1., 9., 4., 10., 0., 11., 5., 12., 2., 13., 6.};

    // InOrder
    inorder(copy, expected);

    expected = {0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13.};

    // BFS
    bfs(copy, expected);

    expected = {0., 1., 3., 7., 8., 4., 9., 10., 2., 5., 11., 12., 6., 13.};

    // DFS
    dfs(copy, expected);

    copy = graph;
    copy.remove_node(10);

    expected = {7., 3., 8., 1., 9., 4., 0., 11., 5., 12., 2., 13., 6., 14.};

    // InOrder
    inorder(copy, expected);

    expected = {0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 11., 12., 13., 14.};

    // BFS
    bfs(copy, expected);

    expected = {0., 1., 3., 7., 8., 4., 9., 2., 5., 11., 12., 6., 13., 14.};

    // DFS
    dfs(copy, expected);

    copy = graph;
    copy.remove_node(8);
    copy.remove_node(4);
    copy.remove_node(2);

    expected = {7., 3., 1., 0.};

    // InOrder
    inorder(copy, expected);

    expected = {0., 1., 3., 7.};

    // BFS
    bfs(copy, expected);

    expected = {0., 1., 3., 7.};

    // DFS
    dfs(copy, expected);

    copy = graph;
    copy.remove_node(1);
    copy.remove_node(12);
    copy.remove_node(13);

    expected = {0., 11., 5., 2., 6., 14.};

    // InOrder
    inorder(copy, expected);

    expected = {0., 2., 5., 6., 11., 14.};

    // BFS
    bfs(copy, expected);

    expected = {0., 2., 5., 11., 6., 14.};

    // DFS
    dfs(copy, expected);

    copy = graph;
    copy.remove_node(1);
    copy.remove_node(5);
    copy.remove_node(13);

    expected = {0., 2., 6., 14.};

    // InOrder
    inorder(copy, expected);

    expected = {0., 2., 6., 14.};

    // BFS
    bfs(copy, expected);

    expected = {0., 2., 6., 14.};

    // DFS
    dfs(copy, expected);

    copy = graph;
    copy.insert(15);
    copy.insert(16);
    copy.insert(17);
    copy.insert(18);
    copy.insert(19);
    copy.insert(20);
    copy.remove_node(11);
    copy.remove_node(10);
    copy.remove_node(15);
    copy.remove_node(17);
    copy.remove_node(19);

    expected = {7., 16., 3., 8.,  18, 1.,  9., 20.,
                4., 0.,  5., 12., 2., 13., 6., 14.};

    // InOrder
    inorder(copy, expected);

    expected = {0., 1., 2.,  3.,  4.,  5.,  6.,  7.,
                8., 9., 12., 13., 14., 16., 18., 20.};

    // BFS
    bfs(copy, expected);

    expected = {0., 1.,  3., 7., 16., 8., 18., 4.,
                9., 20., 2., 5., 12., 6., 13., 14};

    // DFS
    dfs(copy, expected);

    copy.remove_node(2);
    copy.remove_node(3);

    expected = {1., 9., 20., 4., 0.};

    // InOrder
    inorder(copy, expected);

    expected = {0., 1., 4., 9., 20.};

    // BFS
    bfs(copy, expected);

    expected = {0., 1., 4., 9., 20.};

    // DFS
    bfs(copy, expected);

    copy = graph;
    copy.remove_node(12);
    copy.remove_node(10);
    copy.remove_node(14);

    expected = {7., 3., 8., 1., 9., 4., 0., 11., 5., 2., 13., 6.};

    // InOrder
    inorder(copy, expected);

    expected = {0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 11., 13.};

    // BFS
    bfs(copy, expected);

    expected = {0., 1., 3., 7., 8., 4., 9., 2., 5., 11., 6., 13.};

    // DFS
    dfs(copy, expected);

    copy = graph;
    copy.remove_node(0);

    expected = {};

    // InOrder
    inorder(copy, expected);

    expected = {};

    // BFS
    bfs(copy, expected);

    expected = {};

    // DFS
    dfs(copy, expected);

    ASSERT_EQ(++InOrder::View(graph).rend(), InOrder::View(graph).rend());
    ASSERT_EQ(++InOrder::View(graph).end(), InOrder::View(graph).end());
}

TEST(BinaryTree, search) {
    BinaryTree<double> graph;
    using SearchPolicy = BinaryTree<double>::SearchPolicy;

    for (double i = 0.0; i < 100.0; i += 0.2) {
        graph.insert(i);
    }

    for (double i = 0.0; i < 100.0; i += 0.2) {
        ASSERT_TRUE(graph.find(i, SearchPolicy::DepthFirst).has_value());
        ASSERT_EQ(graph.find(i, SearchPolicy::DepthFirst)->get(), i);
        ASSERT_TRUE(graph.find(i, SearchPolicy::InOrder).has_value());
        ASSERT_EQ(graph.find(i, SearchPolicy::InOrder)->get(), i);
        ASSERT_TRUE(graph.find(i, SearchPolicy::BreadthFirst).has_value());
        ASSERT_EQ(graph.find(i, SearchPolicy::BreadthFirst)->get(), i);

        ASSERT_FALSE(graph.find(i + 0.1, SearchPolicy::DepthFirst).has_value());
        ASSERT_FALSE(graph.find(i + 0.1, SearchPolicy::InOrder).has_value());
        ASSERT_FALSE(
            graph.find(i + 0.1, SearchPolicy::BreadthFirst).has_value());
    }
}