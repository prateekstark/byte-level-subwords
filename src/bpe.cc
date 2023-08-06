#include <bpe.h>

namespace dokusha
{
    template <typename T>
    BPETokenizer<T>::BPETokenizer()
    {
        this->vocabularySize = 1;
        this->inverseVocabulary[0] = "_";
    }

    template <typename T>
    BPETokenizer<T>::~BPETokenizer() {}

    template <typename T>
    std::string BPETokenizer<T>::extractToken(std::string &currentWord,
                                              size_t &index)
    {
        return std::string(1, currentWord[index]);
    }

    template <typename T>
    void BPETokenizer<T>::addToCorpus(std::string &line)
    {
        trim(line);
        if (line.size() <= 1)
        {
            return;
        }
        line += " "; // In our case, we consider space as ending of the word!
        std::string currentWord;
        std::vector<T> tokens;
        std::string token = " ";

        for (const auto &ci : line)
        {
            if (ci == ' ')
            {
                if (currentWord.size() > 15)
                {
                    currentWord += ci;
                    continue;
                }

                tokens.clear();
                for (const auto &cj : currentWord)
                {
                    token = cj;
                    tokens.push_back(token);
                    this->addToVocabulary(token);
                }

                if (this->wordWiseTokenListWithFrequency.find(currentWord) != wordWiseTokenListWithFrequency.end())
                {
                    this->wordWiseTokenListWithFrequency[currentWord].second++;
                }
                else
                {
                    this->wordWiseTokenListWithFrequency[currentWord] = std::make_pair(tokens, 1);
                }

                currentWord.clear();
            }
            currentWord += ci;
        }
    }

    template <typename T>
    void BPETokenizer<T>::computePairFrequency()
    {
        this->pairFrequency.clear();

        for (const auto &element : this->wordWiseTokenListWithFrequency)
        {
            for (int i = 0; i < element.second.first.size() - 1; i++)
            {
                std::pair<T, T> pair =
                    std::make_pair(element.second.first[i],
                                   element.second.first[i + 1]);
                if (this->pairFrequency.find(pair) != this->pairFrequency.end())
                {
                    this->pairFrequency[pair] += element.second.second;
                }
                else
                {
                    this->pairFrequency[pair] = element.second.second;
                }
            }
        }
    }

    template <typename T>
    const size_t BPETokenizer<T>::getVocabularySize()
    {
        return this->vocabulary.size();
    }

    template <typename T>
    std::pair<T, T> BPETokenizer<T>::findBestPair()
    {
        int maxOccurance = 0;
        std::pair<T, T> bestPair;

        for (const auto &element : this->pairFrequency)
        {
            if (element.second > maxOccurance)
            {
                bestPair = element.first;
                maxOccurance = element.second;
            }
        }
        return bestPair;
    }

    template <typename T>
    void inline BPETokenizer<T>::addToMergeRule(const std::pair<T, T> &bestPair,
                                                const T &combinedToken)
    {
        this->mergeRules[bestPair] = combinedToken;
    }

    template <typename T>
    void inline BPETokenizer<T>::addToVocabulary(const T &token)
    {
        if (this->vocabulary.find(token) == this->vocabulary.end())
        {
            this->vocabulary[token] = this->vocabularySize;
            this->inverseVocabulary[this->vocabularySize] = token;
            this->vocabularySize++;
        }
    }

    template <typename T>
    void BPETokenizer<T>::pruneRedundantTokens()
    {
        std::unordered_set<T> tokensToBeRemoved;
        for (const auto &element : this->vocabulary)
        {
            tokensToBeRemoved.insert(element.first);
        }

        for (const auto &element : this->wordWiseTokenListWithFrequency)
        {
            for (const auto &token : element.second.first)
            {
                if (tokensToBeRemoved.find(token) != tokensToBeRemoved.end())
                {
                    tokensToBeRemoved.erase(token);
                    if (tokensToBeRemoved.empty())
                    {
                        return;
                    }
                }
            }
        }

        auto startVocabCount = this->vocabularySize;

        for (const auto &token : tokensToBeRemoved)
        {
            this->inverseVocabulary.erase(this->vocabulary[token]);
            this->vocabulary.erase(token);
            this->vocabularySize--;
        }
        auto endVocabCount = this->vocabularySize;

        print("Pruned Redundant tokens from " + std::to_string(startVocabCount) + " to " + std::to_string(endVocabCount));
    }

    template <typename T>
    void BPETokenizer<T>::updateWordWiseTokenList(const T &token1, const T &token2)
    {
        for (auto &element : this->wordWiseTokenListWithFrequency)
        {
            if (element.second.first.size() == 1)
            {
                continue;
            }

            for (std::vector<std::string>::iterator it = element.second.first.begin();
                 it < element.second.first.end() - 1;)
            {
                if (it->compare(token1) == 0 && (it + 1)->compare(token2) == 0)
                {
                    *it = token1 + token2;
                    it++;
                    if (it != element.second.first.end())
                    {
                        it = element.second.first.erase(it);
                    }
                }
                else
                {
                    it++;
                }
            }
        }
    }

    template <typename T>
    const void BPETokenizer<T>::printWordWiseTokenList()
    {
        for (const auto &element : this->wordWiseTokenListWithFrequency)
        {
            std::cout << element.first << ": (";
            for (const auto &tokens : element.second.first)
            {
                std::cout << tokens << " ";
            }
            std::cout << "\b) " << std::endl;
        }
    }

    template <typename T>
    const void BPETokenizer<T>::printVocabulary(const bool &detailed)
    {
        std::cout << "Vocabulary" << std::endl;

        if (!detailed)
        {
            std::vector<T> keyVector;
            for (const auto &element : this->vocabulary)
            {
                keyVector.push_back(element.first);
            }
            sort(keyVector.begin(), keyVector.end());

            for (const auto &element : keyVector)
            {
                std::cout << element << " ";
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "Vocabulary" << std::endl;
            for (const auto &element : this->vocabulary)
            {
                std::cout << element.first << " " << element.second << std::endl;
            }
        }
    }

    template <typename T>
    const void BPETokenizer<T>::printPairFrequency()
    {
        for (const auto &element : this->pairFrequency)
        {
            std::cout << "(" << element.first.first << ", " << element.first.second
                      << ")"
                      << ": " << element.second << std::endl;
        }
    }

    template <typename T>
    const void BPETokenizer<T>::printMergeRules()
    {
        for (const auto &element : this->mergeRules)
        {
            std::cout << "(" << element.first.first << ", " << element.first.second
                      << "): " << element.second << std::endl;
        }
    }

    template <typename T>
    void BPETokenizer<T>::runLearningIteration()
    {
        std::pair<std::string, std::string> bestPair;
        this->computePairFrequency();
        bestPair = this->findBestPair();
        std::string combined_token = bestPair.first + bestPair.second;
        this->addToMergeRule(bestPair, combined_token);
        this->addToVocabulary(combined_token);
        this->updateWordWiseTokenList(bestPair.first, bestPair.second);
    }

    template <typename T>
    void BPETokenizer<T>::pruneWordList()
    {
        size_t startNumWords = this->wordWiseTokenListWithFrequency.size();
        for (auto it = this->wordWiseTokenListWithFrequency.begin(); it != this->wordWiseTokenListWithFrequency.end();)
        {
            if (it->second.second < this->frequencyPruneThreshold)
            {
                it = this->wordWiseTokenListWithFrequency.erase(it);
            }
            else
            {
                it++;
            }
        }
        size_t endNumWords = this->wordWiseTokenListWithFrequency.size();
        print("Pruned word frequency from " + std::to_string(startNumWords) + " to " + std::to_string(endNumWords));
    }

    template <typename T>
    const void BPETokenizer<T>::printTokenizedText(const std::vector<int> &tokenizedText)
    {
        for (const auto &index : tokenizedText)
        {
            std::cout << index << " ";
        }
        std::cout << std::endl;
    }

    template <typename T>
    std::vector<unsigned short> BPETokenizer<T>::tokenize(std::string text)
    {
        std::vector<unsigned short> tokenizedText;
        trim(text);

        text += " "; // In our case, we consider space as ending of the word!
        std::string currentWord;
        std::string token;

        std::vector<std::vector<T>> tokenList;
        for (const auto &ci : text)
        {
            if (ci == ' ')
            {
                std::vector<T> tokens;
                for (const auto &cj : currentWord)
                {
                    token = std::string(1, cj);
                    tokens.push_back(token);
                }
                tokenList.push_back(tokens);
                currentWord.clear();
            }
            currentWord += ci;
        }

        for (const auto &rule : this->mergeRules)
        {
            for (auto &element : tokenList)
            {
                if (element.size() == 1)
                {
                    continue;
                }

                T token1 = rule.first.first;
                T token2 = rule.first.second;
                T mergedToken = rule.second;

                for (std::vector<std::string>::iterator it = element.begin();
                     it < element.end() - 1;)
                {
                    if (it->compare(token1) == 0 && (it + 1)->compare(token2) == 0)
                    {
                        *it = mergedToken;
                        it++;
                        if (it != element.end())
                        {
                            it = element.erase(it);
                        }
                    }
                    else
                    {
                        it++;
                    }
                }
            }
        }

        for (const auto &word : tokenList)
        {
            for (const auto &token : word)
            {
                if (vocabulary.find(token) == vocabulary.end())
                {
                    tokenizedText.push_back(0);
                }
                else
                {
                    tokenizedText.push_back(vocabulary[token]);
                }
            }
        }

        // this->printTokenizedText(tokenizedText);

        return tokenizedText;
    }

    template <typename T>
    std::string BPETokenizer<T>::detokenize(std::vector<unsigned short> tokenizedText)
    {
        std::string result = "";
        for (auto &token : tokenizedText)
        {
            result += this->inverseVocabulary[token];
        }

        return result;
    }

} // namespace dokusha

template class dokusha::BPETokenizer<std::string>;