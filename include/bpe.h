#ifndef BPE_H
#define BPE_H

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <utils.h>
#include <omp.h>
#include <fstream>

namespace dokusha
{
    struct PairHash
    {
        template <class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2> &p) const
        {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            // Combine the hash values using a simple bitwise operation
            return h1 ^ h2;
        }
    };

    struct PairEqual
    {
        template <class T1, class T2>
        bool operator()(const std::pair<T1, T2> &lhs, const std::pair<T1, T2> &rhs) const
        {
            return lhs.first == rhs.first && lhs.second == rhs.second;
        }
    };

    template <typename T>
    class BPETokenizer
    {
    private:
        std::unordered_map<std::string, std::pair<std::vector<T>, unsigned>> wordWiseTokenListWithFrequency;
        std::unordered_map<std::pair<T, T>, T, PairHash, PairEqual> mergeRules;
        std::unordered_map<std::pair<T, T>, unsigned, PairHash, PairEqual> pairFrequency;
        unsigned short vocabularySize;
        std::unordered_map<T, unsigned short> vocabulary;
        std::unordered_map<unsigned short, T> inverseVocabulary;
        static constexpr unsigned short frequencyPruneThreshold = 2;
        std::unordered_set<T> baseVocabulary;

    public:
        BPETokenizer();
        ~BPETokenizer();
        void addToCorpus(std::string &line);
        void pruneWordList();
        void pruneRedundantTokens();
        void computePairFrequency();
        void applyMergeRule(const std::pair<std::pair<T, T>, T> &rule, std::vector<T> &rawTokenList);
        void inline addToMergeRule(const std::pair<T, T> &bestPair, const T &combinedToken);
        void inline addToVocabulary(const T &token);
        void inline addToVocabulary(const T &token, unsigned short tokenIndex);
        void updateWordWiseTokenList(const T &token1, const T &token2);
        T combineTokens(std::pair<T, T> bestPair);
        std::pair<T, T> findBestPair();
        std::string extractToken(std::string &currentWord, size_t &index);
        std::vector<unsigned short> tokenize(std::string text);
        std::string detokenize(std::vector<unsigned short> tokenizedText);
        void runLearningIteration();
        const unsigned short getVocabularySize() const;
        

        void save(const std::string filepath) const;
        void load(const std::string filepath);

        bool operator==(const BPETokenizer<T>& other) const;

        // Printing functions
        void printWordWiseTokenList() const;
        void printVocabulary(const bool &detailed) const;
        void printPairFrequency() const;
        void printMergeRules() const;
        void printTokenizedText(const std::vector<int> &tokenizedText) const;
    };
    
}

#endif