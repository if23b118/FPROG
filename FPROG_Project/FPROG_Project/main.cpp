#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <numeric>
#include <boost/regex.hpp>

//----------------------------------------------------------------
//BAUM
enum class Color { RED, BLACK };

template <typename T>
struct Node {
    T value;
    Color color;
    std::shared_ptr<Node<T>> left;  // Pointer auf das linke Kind
    std::shared_ptr<Node<T>> right;  // Pointer auf das rechte Kind

    // Konstruktor für einen neuen Knoten
    Node(T val, Color col,
         std::shared_ptr<Node<T>> left = nullptr,
         std::shared_ptr<Node<T>> right = nullptr)
        : value(val), color(col), left(left), right(right) {}
};

// Alias für einen Zeiger auf einen Knoten
template <typename T>
using NodePtr = std::shared_ptr<Node<T>>;

template <typename T>
class RedBlackTree {
private:
    NodePtr<T> root;  // Wurzel des Baums

    // Linksrotation (wird durchgeführt, wenn das rechte Kind rot ist und das linke Kind schwarz ist)
    auto rotateLeft(NodePtr<T> node) const -> NodePtr<T> {
        if (!node || !node->right) return node;
        return std::make_shared<Node<T>>(
            node->right->value, node->color,
            std::make_shared<Node<T>>(node->value, Color::RED, node->left, node->right->left),
            node->right->right);
    }

    // Rechtsrotation (wird durchgeführt, wenn das linke Kind und sein linkes Kind rot sind)
    auto rotateRight(NodePtr<T> node) const -> NodePtr<T> {
        return std::make_shared<Node<T>>(
            node->left->value, node->color,
            node->left->left,
            std::make_shared<Node<T>>(node->value, Color::RED, node->left->right, node->right));
    }

    // Recoloring (ändert die Farbe von zwei Kindern eines Knotens auf schwarz)
    auto recolor(NodePtr<T> node) const -> NodePtr<T> {
        return std::make_shared<Node<T>>(
            node->value, Color::RED,
            std::make_shared<Node<T>>(node->left->value, Color::BLACK, node->left->left, node->left->right),
            std::make_shared<Node<T>>(node->right->value, Color::BLACK, node->right->left, node->right->right));
    }

    // Hilfsfunktion für das Einfügen eines Werts in den Baum
    auto insertHelper(NodePtr<T> node, const T& value) const -> NodePtr<T> {
        if (!node) return std::make_shared<Node<T>>(value, Color::RED);  // Erstelle einen roten Knoten für die Einfügung

        // Traversiere den Baum
        if (value < node->value) {
            node->left = insertHelper(node->left, value);  // Rekursiv links einfügen
        } else if (value > node->value) {
            node->right = insertHelper(node->right, value);  // Rekursiv rechts einfügen
        } else {
            return node;  // Verhindert Duplikate
        }

        // Kopiere den aktuellen Knoten, um sicherzustellen, dass wir eine Kopie anstelle einer Modifikation haben
        NodePtr<T> newNode = std::make_shared<Node<T>>(node->value, node->color, node->left, node->right);

        // Baumstruktur korrigieren
        if (isRed(newNode->right) && !isRed(newNode->left)) {
            newNode = rotateLeft(newNode);  // Linksrotation, wenn das rechte Kind rot ist und das linke schwarz ist
        }
        if (isRed(newNode->left) && isRed(newNode->left->left)) {
            newNode = rotateRight(newNode);  // Rechtsrotation, wenn das linke Kind und sein linkes Kind rot sind
        }
        if (isRed(newNode->left) && isRed(newNode->right)) {
            newNode = recolor(newNode);  // Recoloring, wenn beide Kinder rot sind
        }

        return newNode;  // Gib den neuen Knoten zurück
    }

    // Prüft, ob der Knoten rot ist
    auto isRed(NodePtr<T> node) const -> bool {
        return node && node->color == Color::RED;
    }

public:
    RedBlackTree() : root(nullptr) {}  // Konstruktor: Anfangszustand des Baums ist leer

    // Einfügen eines Werts in den Baum
    auto insert(const T& value) const -> RedBlackTree<T> {
        auto newRoot = insertHelper(root, value);  // Wert einfügen
        // Sicherstellen, dass die Wurzel immer schwarz ist
        return RedBlackTree(std::make_shared<Node<T>>(newRoot->value, Color::BLACK, newRoot->left, newRoot->right));
    }

    // Inorder-Traversierung (durchläuft den Baum in aufsteigender Reihenfolge)
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
    // Konstruktor, um den Baum mit einer gegebenen Wurzel zu erstellen
    explicit RedBlackTree(NodePtr<T> rootNode) : root(rootNode) {}
};

//----------------------------------------------------------------
//FUNCTIONS

// Konvertiert einen String in Kleinbuchstaben
auto toLower(const std::string& str) -> std::string {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return lowerStr;
}

// Liest eine Datei und gibt die Zeilen als Vektor zurück
auto readFile(const std::string& filename) -> std::vector<std::string> {
    std::ifstream file(filename);
    std::vector<std::string> lines;

    // Verwende std::copy, um Zeilen in den Vector zu kopieren
    std::copy(
        std::istream_iterator<std::string>(file),
        std::istream_iterator<std::string>(),
        std::back_inserter(lines)
    );

    return lines;
}

auto tokenize(const std::string& text) -> std::vector<std::string> {
    std::vector<std::string> words;
    boost::regex word_regex("[a-zA-Z0-9]+");  // Regex, um Wörter zu extrahieren

    // Definiere die rekursive Funktion mit explizitem Typ
    std::function<std::vector<std::string>(boost::sregex_iterator, boost::sregex_iterator)> tokenizeHelper =
        [&](boost::sregex_iterator it, boost::sregex_iterator end) -> std::vector<std::string> {
            if (it == end) return words;
            words.push_back(toLower(it->str()));
            return tokenizeHelper(std::next(it), end);
        };

    return tokenizeHelper(boost::sregex_iterator(text.begin(), text.end(), word_regex), boost::sregex_iterator());
}

// Schreibt die sortierten Wörter in eine Ausgabedatei
auto writeFile(const std::string& filename, const std::vector<std::string>& words) -> void {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
        return;
    }

    // Definiere die rekursive Funktion mit explizitem Typ
    std::function<void(std::vector<std::string>::const_iterator, std::vector<std::string>::const_iterator)> writeHelper =
        [&](std::vector<std::string>::const_iterator it, std::vector<std::string>::const_iterator end) {
            if (it == end) return;
            file << *it << "\n";
            writeHelper(std::next(it), end);
        };

    writeHelper(words.begin(), words.end());
}

//----------------------------------------------------------------
//MAIN

int main() {
    const std::string inputFile = "/Users/marianowak/Downloads/FPROG_Project 2/war_and_peace.txt";
    const std::string outputFile = "output.txt";

    // Liest die Datei und überprüft, ob sie leer oder nicht gefunden wurde
    const auto lines = readFile(inputFile);
    if (lines.empty()) {
        std::cerr << "Error: Input file is empty or not found: " << inputFile << std::endl;
        return 1;
    }

    // Erstelle den Red-Black Tree und füge alle Wörter hinzu
    RedBlackTree<std::string> tree = std::accumulate(lines.begin(), lines.end(), RedBlackTree<std::string>{},
        [&](RedBlackTree<std::string> currentTree, const std::string& line) {
            // Tokenisiere die Zeile einmal
            const auto words = tokenize(line);

            // Füge jedes Wort in den Baum ein (Baum wird bei jeder Einfügung kopiert)
            return std::accumulate(words.begin(), words.end(), currentTree,
                [](RedBlackTree<std::string> tree, const std::string& word) {
                    return tree.insert(word);
                });
        });

    // Hole die sortierten Wörter durch Inorder-Traversierung
    const auto sortedWords = tree.inOrder();

    // Schreibe die sortierten Wörter in die Ausgabedatei
    writeFile(outputFile, sortedWords);

    return 0;
}
