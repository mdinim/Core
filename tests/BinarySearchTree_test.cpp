//
// Created by Dániel Molnár on 2019-09-05.
//

#include <gtest/gtest.h>

#include <chrono>
#include <random>
#include <type_traits>

#include <Graph/BinarySearchTree.hpp>
#include <Utils/TestUtil.hpp>

using namespace Core;

class BinarySearchTreeTextFixture : public ::testing::Test {
  private:
    std::random_device _device;
    std::mt19937 _generator;
    std::uniform_int_distribution<> _distribution;

  protected:
    BinarySearchTree<int> search_tree;

    BinarySearchTreeTextFixture()
        : _generator(_device()), _distribution(-99999, 99999) {}

    void SetUp() { search_tree.clear(); }

    int get_random() { return _distribution(_generator); }
};

TEST_F(BinarySearchTreeTextFixture, insertion) {
    for (unsigned int i = 0; i < 100u; ++i) {
        std::vector<int> result;
        result.reserve(150);
        for (unsigned int j = 0; j < 1000u; ++j) {
            int random_variable = get_random();

            search_tree.insert(random_variable);
        }

        for (const auto &node : InOrder::View(search_tree)) {
            result.push_back(node.value());
        }

        // InOrder traversal should produce sorted sequence of values.
        ASSERT_TRUE(std::is_sorted(result.begin(), result.end()));
    }
}

TEST_F(BinarySearchTreeTextFixture, search) {
    using namespace std::chrono;

    BinaryTree<int> regular_graph;

    ASSERT_FALSE(search_tree.find(0));

    int needle = get_random();
    regular_graph.reserve(2 * 99999);
    for (int i = -99999; i < 99999; ++i) {
        int value = get_random();
        if (get_random() < 0) // There is a chance this never occurs, and
                              // needle is a completely random number that is
                              // not even in the graph. We can live with that.
            needle = value;

        regular_graph.insert(value);
        search_tree.insert(value);
    }

    ASSERT_TRUE(search_tree.contains(needle));
    ASSERT_TRUE(regular_graph.contains(needle));
    ASSERT_FALSE(search_tree.contains(100000));
    ASSERT_FALSE(search_tree.contains(-100000));

    auto time_at_start = high_resolution_clock::now();
    ASSERT_TRUE(regular_graph.find(needle));
    auto time_at_end = high_resolution_clock::now();
    auto elapsed_micros_regular =
        duration_cast<microseconds>(time_at_end - time_at_start).count();
    time_at_start = high_resolution_clock::now();
    ASSERT_TRUE(search_tree.find(needle));
    time_at_end = high_resolution_clock::now();
    auto elapsed_microsec_search =
        duration_cast<microseconds>(time_at_end - time_at_start).count();

    TEST_INFO << "Tree: " << elapsed_micros_regular << " micro s" << std::endl;
    TEST_INFO << "SearchTree: " << elapsed_microsec_search << " micro s"
              << std::endl;
    ASSERT_GT(elapsed_micros_regular, elapsed_microsec_search);
}

TEST_F(BinarySearchTreeTextFixture, value_removal) {
    const auto do_inorder = [](const auto &graph, const auto &expected) {
        std::vector<
            typename std::remove_reference_t<decltype(graph)>::value_type>
            result = {};
        for (const auto &node : InOrder::View(graph)) {
            result.push_back(node.value());
        }
        ASSERT_EQ(result, expected);
    };

    const auto do_dfs = [](const auto &graph, const auto &expected) {
        std::vector<
            typename std::remove_reference_t<decltype(graph)>::value_type>
            result = {};
        for (const auto &node : DFS::View(graph)) {
            result.push_back(node.value());
        }
        ASSERT_EQ(result, expected);
    };

    search_tree.insert(0);
    search_tree.insert(-5);
    search_tree.insert(-2);
    search_tree.insert(-10);
    search_tree.insert(-4);
    search_tree.insert(-1);
    search_tree.insert(-3);
    search_tree.insert(-7);

    std::vector<int> expected = {0, -5, -10, -7, -2, -4, -3, -1};

    do_dfs(search_tree, expected);

    expected = {-10, -7, -5, -4, -3, -2, -1, 0};

    do_inorder(search_tree, expected);

    search_tree.remove_value(-10);

    expected = {0, -5, -7, -2, -4, -3, -1};

    do_dfs(search_tree, expected);

    expected = {-7, -5, -4, -3, -2, -1, 0};

    do_inorder(search_tree, expected);

    search_tree.remove_value(-1);

    expected = {0, -5, -7, -2, -4, -3};

    do_dfs(search_tree, expected);

    expected = {-7, -5, -4, -3, -2, 0};

    do_inorder(search_tree, expected);

    search_tree.remove_value(-3);

    expected = {0, -5, -7, -2, -4};

    do_dfs(search_tree, expected);

    expected = {-7, -5, -4, -2, 0};

    do_inorder(search_tree, expected);

    search_tree.remove_value(-5);

    expected = {0, -4, -7, -2};

    do_dfs(search_tree, expected);

    expected = {-7, -4, -2, 0};

    do_inorder(search_tree, expected);

    search_tree.remove_value(-2);

    expected = {0, -4, -7};

    do_dfs(search_tree, expected);

    expected = {-7, -4, 0};

    do_inorder(search_tree, expected);

    search_tree.remove_value(0);

    expected = {-4, -7};

    do_dfs(search_tree, expected);

    expected = {-7, -4};

    do_inorder(search_tree, expected);

    search_tree.insert(-9);
    search_tree.insert(-8);
    search_tree.insert(-7);
    search_tree.insert(-10);
    search_tree.remove_value(-9);

    expected = {-4, -7, -8, -10, -7};

    do_dfs(search_tree, expected);

    expected = {-10, -8, -7, -7, -4};

    do_inorder(search_tree, expected);
}

TEST_F(BinarySearchTreeTextFixture, node_removal) {
    for (int i = 0; i < 10000; ++i) {
        search_tree.insert(get_random());
    }
    search_tree.remove_node(search_tree.root_node());
    ASSERT_TRUE(search_tree.empty());
}