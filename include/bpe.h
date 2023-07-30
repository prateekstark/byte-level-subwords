#ifndef BPE_H
#define BPE_H

#include <iostream>
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include <vector>
#include <utils.h>

namespace dokusha
{

    template <typename T>
    class BPETokenizer
    {
    private:
        std::unordered_map<std::string, std::vector<T>> wordWiseTokenList;
        std::unordered_map<std::string, int> wordFrequency;
        std::map<std::pair<T, T>, T> mergeRules;
        std::map<std::pair<T, T>, int> pairFrequency;
        size_t vocabularySize;
        std::unordered_map<T, size_t> vocabulary;
        std::unordered_map<size_t, T> inverseVocabulary;

    public:
        BPETokenizer();
        ~BPETokenizer();
        void addToCorpus(std::string &line);
        void computePairFrequency();
        void addToMergeRule(std::pair<T, T> bestPair, T combinedToken);
        void addToVocabulary(T &token);
        void updateWordWiseTokenList(T token1, T token2);
        T combineTokens(std::pair<T, T> bestPair);
        std::pair<T, T> findBestPair();
        std::string extractToken(std::string &currentWord, size_t &index);
        std::vector<int> tokenize(std::string line);
        std::string detokenize(std::vector<int> tokenizedText);
        void runLearningIteration();
        size_t getVocabularySize();

        // Printing functions
        void printWordWiseTokenList();
        void printVocabulary(bool detailed);
        void printPairFrequency();
        void printMergeRules();
        void printTokenizedText(std::vector<int> tokenizedText);
    };

}

#endif