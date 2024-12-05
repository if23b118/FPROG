#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <numeric>
#include <regex>

//----------------------------------------------------------------
//TREE
enum class Color { RED, BLACK };

template <typename T>
struct Node {
    T value;
    Color color;
    std::shared_ptr<Node<T>> left;
    std::shared_ptr<Node<T>> right;

    Node(T val, Color col,
         std::shared_ptr<Node<T>> left = nullptr,
         std::shared_ptr<Node<T>> right = nullptr)
        : value(val), color(col), left(left), right(right) {}
};

template <typename T>
using NodePtr = std::shared_ptr<Node<T>>;

template <typename T>
class RedBlackTree {
private:
    NodePtr<T> root;

    auto rotateLeft(NodePtr<T> node) const -> NodePtr<T> {
        if (!node || !node->right) return node;
        return std::make_shared<Node<T>>(
            node->right->value, node->color,
            std::make_shared<Node<T>>(node->value, Color::RED, node->left, node->right->left),
            node->right->right);
    }

    auto rotateRight(NodePtr<T> node) const -> NodePtr<T> {
        return std::make_shared<Node<T>>(
            node->left->value, node->color,
            node->left->left,
            std::make_shared<Node<T>>(node->value, Color::RED, node->left->right, node->right));
    }

    auto recolor(NodePtr<T> node) const -> NodePtr<T> {
        return std::make_shared<Node<T>>(
            node->value, Color::RED,
            std::make_shared<Node<T>>(node->left->value, Color::BLACK, node->left->left, node->left->right),
            std::make_shared<Node<T>>(node->right->value, Color::BLACK, node->right->left, node->right->right));
    }

    auto insertHelper(NodePtr<T> node, T value) const -> NodePtr<T> {
        if (!node) return std::make_shared<Node<T>>(value, Color::RED);  // Create a red node for insertion

        // Traverse the tree
        if (value < node->value) {
            node->left = insertHelper(node->left, value);
        } else if (value > node->value) {
            node->right = insertHelper(node->right, value);
        } else {
            return node;  // Prevent duplicates
        }

        // Fix the tree properties
        if (isRed(node->right) && !isRed(node->left)) {
            node = rotateLeft(node);  // Rotate left if right child is red and left is black
        }
        if (isRed(node->left) && isRed(node->left->left)) {
            node = rotateRight(node);  // Rotate right if left-left child is red
        }
        if (isRed(node->left) && isRed(node->right)) {
            node = recolor(node);  // Recolor if both left and right children are red
        }

        return node;
    }


    auto isRed(NodePtr<T> node) const -> bool {
        return node && node->color == Color::RED;
    }

public:
    RedBlackTree() : root(nullptr) {}

    auto insert(T value) const -> RedBlackTree<T> {
        auto newRoot = insertHelper(root, value);  // Insert the value into the tree
        // Ensure the root is always black
        return RedBlackTree(std::make_shared<Node<T>>(newRoot->value, Color::BLACK, newRoot->left, newRoot->right));
    }


    auto inOrder() const -> std::vector<T> {
        std::vector<T> result;
        std::function<void(NodePtr<T>)> traverse = [&](NodePtr<T> node) {
            if (!node) return;
            traverse(node->left);
            result.push_back(node->value);
            traverse(node->right);
        };
        traverse(root);
        return result;
    }

private:
    explicit RedBlackTree(NodePtr<T> rootNode) : root(rootNode) {}
};

//----------------------------------------------------------------
//FUNCTIONS

auto readFile(const std::string &filename) -> std::vector<std::string> {
    std::ifstream file(filename);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

auto tokenize(const std::string &text) -> std::vector<std::string> {
    std::vector<std::string> words;
    std::regex word_regex("[a-zA-Z0-9]+");

    auto words_begin = std::sregex_iterator(text.begin(), text.end(), word_regex);
    auto words_end = std::sregex_iterator();

    for (auto it = words_begin; it != words_end; ++it) {
        words.push_back(it->str());
    }

    return words;
}

auto writeFile(const std::string &filename, const std::vector<std::string> &words) -> void {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
        return;
    }
    for (const auto &word : words) {
        file << word << "\n";
    }
    //std::cout << "Words written to " << filename << std::endl;  // Debugging line
}


//----------------------------------------------------------------
//MAIN

int main() {
    const std::string inputFile = "/Users/marianowak/CLionProjects/FPROG_Project/war_and_peace.txt";
    const std::string outputFile = "output.txt";

    auto lines = readFile(inputFile);
    if (lines.empty()) {
        std::cerr << "Error: Input file is empty or not found: " << inputFile << std::endl;
        return 1;
    }

    RedBlackTree<std::string> tree = std::accumulate(lines.begin(), lines.end(), RedBlackTree<std::string>{},
        [&](RedBlackTree<std::string> currentTree, const std::string &line) {
            auto words = tokenize(line);
            //std::cout << "Total words in line: " << words.size() << std::endl;

            if (!words.empty()) {
                // Insert words into the tree
                for (const auto &word : words) {
                   // std::cout << "Inserting: " << word << std::endl;
                    currentTree = currentTree.insert(word);  // Insert word into the tree
                }
            }

            /*
            auto sortedWords = currentTree.inOrder();
            std::cout << "Tree contents after inserting line: ";
            for (const auto &word : sortedWords) {
                std::cout << word << " ";
            }
            std::cout << std::endl;
            */

            return currentTree;
        });
    
    auto sortedWords = tree.inOrder();

    // Debugging sorted words

    /*std::cout << "Sorted words: ";
    for (const auto &word : sortedWords) {
        std::cout << word << " ";
    }
    std::cout << std::endl;
    */

    writeFile(outputFile, sortedWords);

    //std::cout << "Words sorted and written to " << outputFile << std::endl;
    return 0;
}
