#include <bpe.h>
#include <cassert>
#include <fstream>
#include <chrono>

using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

int main(int argc, char **argv)
{

    assert(argc == 2);
    dokusha::BPETokenizer<std::string> tokenizer;
    std::fstream fio;
    std::string line;
    std::chrono::time_point<std::chrono::steady_clock> start, end;

    int filesCompleted = 0;
    for (const auto &textFile : recursive_directory_iterator(argv[1]))
    {
        start = std::chrono::steady_clock::now();
        fio.open(textFile.path(), std::ios::in);
        fio.seekg(0, std::ios::beg);
        int lineIndex = 0;

        while (fio)
        {
            getline(fio, line);
            lineIndex++;
            if (lineIndex < 3)
            {
                continue;
            }
            tokenizer.addToCorpus(line);
        }
        filesCompleted++;

        // if (filesCompleted % 100 == 0) {
        //     tokenizer.printVocabulary(false);
        // }

        fio.close();

        end = std::chrono::steady_clock::now();
        std::cout << "\r[Files completed: " << filesCompleted << "], Time(ms):"
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << ", Vocabulary Size: " << tokenizer.getVocabularySize() << " |";
        std::cout.flush();
    }
    std::cout << std::endl;
    tokenizer.printVocabulary(false);

    /*

    line =
        "                               Hello, world! I am IronAB Man... I am "
        "the savious of thos world. I am a good boy.";
    tokenizer.addToCorpus(line);

    for (int i = 0; i < 50; i++)
    {
        tokenizer.runLearningIteration();
    }
    // tokenizer.printVocabulary(true);
    std::vector<int> tokenizedText = tokenizer.tokenize("I am Iron Man!, hehehehe");
    std::cout << tokenizer.detokenize(tokenizedText) << std::endl;

    std::string s = "abcd";
    print(tokenizer.detokenize(tokenizer.tokenize(s)));

    */

    return 0;
}