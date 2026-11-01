#include <fstream>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

#include "fabricapr.h"

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc < 2) {
        std::cerr << "Использование: " << argv[0] << " <входной_файл>\n";
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in) {
        std::cerr << "Не удалось открыть файл: " << argv[1] << '\n';
        return 2;
    }

    SentenceFactory factory;
    const auto sentences = readSentences(in, factory);

    for (std::size_t i = 0; i < sentences.size(); ++i)
        std::cout << i + 1 << ": " << *sentences[i] << '\n';

    return 0;
}
