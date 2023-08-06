#include <bpe.h>
#include <cassert>
#include <fstream>
#include <chrono>
#include <filesystem>

using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

int main(int argc, char **argv)
{

    assert(argc == 2);
    dokusha::BPETokenizer<std::string> tokenizer;
    std::ifstream fio;
    std::string line;
    std::chrono::time_point<std::chrono::steady_clock> start, end;
    std::string buffer;

    int filesCompleted = 0;
    for (const auto &textFile : recursive_directory_iterator(argv[1]))
    {
        start = std::chrono::steady_clock::now();
        fio.open(textFile.path(), std::ios::binary);
        fio.seekg(0, std::ios::end);
        std::streampos fileSize = fio.tellg();
        fio.seekg(0, std::ios::beg);

        // Define a string to hold the binary data with appropriate size
        buffer.clear();
        buffer.resize(static_cast<size_t>(fileSize));
        // Read the binary data into the string buffer
        fio.read(&buffer[0], fileSize);
        std::stringstream ss(buffer);

        int lineIndex = 0;

        while (getline(ss, line))
        {

            lineIndex++;
            if (lineIndex < 3)
            {
                continue;
            }
            tokenizer.addToCorpus(line);
        }

        // Close the file after reading
        fio.close();
        filesCompleted++;

        fio.close();

        end = std::chrono::steady_clock::now();
        std::cout << "\r[Files completed: " << filesCompleted << "], Time(ms):"
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << ", Vocabulary Size: " << tokenizer.getVocabularySize() << " |";
        std::cout.flush();
        if (filesCompleted % 100 == 0) {
        break;
        // tokenizer.pruneWordFrequency();
        }
        // break;
    }
    std::cout << std::endl;
    tokenizer.printVocabulary(false);
    tokenizer.pruneWordList();
    // tokenizer.printWordWiseTokenList();
    // print(tokenizer.wordFrequency.size());

    while (tokenizer.getVocabularySize() < 1024)
    {
        start = std::chrono::steady_clock::now();
        tokenizer.runLearningIteration();
        end = std::chrono::steady_clock::now();
        std::cout << "\r[Vocab Size: " << tokenizer.getVocabularySize() << "], Time(s):"
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0 << " |";
        std::cout.flush();
    }
    tokenizer.pruneRedundantTokens();
    tokenizer.printVocabulary(false);
    // std::vector<unsigned short> tokenizedText = tokenizer.tokenize("I am Iron Man!, hehehehe");
    // std::cout << tokenizer.detokenize(tokenizedText) << std::endl;

    // std::string s = "abcd";
    // print(tokenizer.detokenize(tokenizer.tokenize(s)));

    return 0;
}