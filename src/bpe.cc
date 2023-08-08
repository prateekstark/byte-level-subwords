#include <bpe.h>

namespace dokusha
{
    template <typename T>
    BPETokenizer<T>::BPETokenizer()
    {
        this->vocabularySize = 1;
        this->inverseVocabulary[0] = "_";

        for (uint8_t i = 0;; i++)
        {
            this->addToVocabulary(std::string(1, i));
            this->baseVocabulary.insert(std::string(1, i));

            if (i == 255)
            {
                break;
            }
        }
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
                    currentWord = " ";
                    continue;
                }

                tokens.clear();

                tokens.reserve(currentWord.size());
                for (const auto &cj : currentWord)
                {
                    tokens.emplace_back(1, cj);
                    // For BBPE, we can comment this line, since we have a base vocabulary. Increases performance by 30-40%
                    // this->addToVocabulary(token);
                }

                auto &wordTokenListWithFrequency = this->wordWiseTokenListWithFrequency[currentWord];

                if (wordTokenListWithFrequency.first.empty())
                {
                    wordTokenListWithFrequency.first = std::move(tokens);
                }
                wordTokenListWithFrequency.second++;

                currentWord.clear();
            }
            currentWord += ci;
        }

        for (const auto &rule : this->mergeRules)
        {
            for (auto &element : wordWiseTokenListWithFrequency)
            {
                this->applyMergeRule(rule, element.second.first);
            }
        }
    }

    template <typename T>
    void BPETokenizer<T>::applyMergeRule(const std::pair<std::pair<T, T>, T> &rule, std::vector<T> &rawTokenList)
    {
        if (rawTokenList.size() == 1)
        {
            return;
        }

        T token1 = rule.first.first;
        T token2 = rule.first.second;
        T mergedToken = rule.second;

        for (std::vector<std::string>::iterator it = rawTokenList.begin();
             it < rawTokenList.end() - 1;)
        {
            if (it->compare(token1) == 0 && (it + 1)->compare(token2) == 0)
            {
                *it = mergedToken;
                it++;
                if (it != rawTokenList.end())
                {
                    it = rawTokenList.erase(it);
                }
            }
            else
            {
                it++;
            }
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
                this->pairFrequency[std::make_pair(element.second.first[i],
                                                   element.second.first[i + 1])] += element.second.second;
            }
        }
    }

    template <typename T>
    const unsigned short BPETokenizer<T>::getVocabularySize() const
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
    void inline BPETokenizer<T>::addToVocabulary(const T &token, unsigned short tokenIndex)
    {
        if (this->vocabulary.find(token) == this->vocabulary.end())
        {
            this->vocabulary[token] = tokenIndex;
            this->inverseVocabulary[tokenIndex] = token;
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

        auto startVocabCount = this->getVocabularySize();

        for (const auto &token : tokensToBeRemoved)
        {
            this->inverseVocabulary.erase(this->vocabulary[token]);
            this->vocabulary.erase(token);
            this->vocabularySize--;
        }
        auto endVocabCount = this->getVocabularySize();

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
                this->applyMergeRule(rule, element);
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

    template <typename T>
    void BPETokenizer<T>::save(const std::string filepath) const
    {
        std::ofstream outFile(filepath, std::ios::binary);
        outFile.write(reinterpret_cast<const char *>(&this->vocabularySize), sizeof(this->vocabularySize));

        unsigned short stringLength;
        for (auto &token : this->vocabulary)
        {
            stringLength = token.first.size();
            outFile.write(reinterpret_cast<const char *>(&stringLength), sizeof(stringLength));
            outFile.write(token.first.c_str(), stringLength);
            outFile.write(reinterpret_cast<const char *>(&token.second), sizeof(token.second));
        }

        unsigned short mergeRuleSize = this->mergeRules.size();
        outFile.write(reinterpret_cast<const char *>(&mergeRuleSize), sizeof(mergeRuleSize));

        unsigned short token1Length;
        unsigned short token2Length;
        unsigned short combinedTokenLength;

        for (auto &rule : this->mergeRules)
        {
            token1Length = static_cast<unsigned short>(rule.first.first.size());
            token2Length = static_cast<unsigned short>(rule.first.second.size());
            combinedTokenLength = static_cast<unsigned short>(rule.second.size());
            outFile.write(reinterpret_cast<const char *>(&token1Length), sizeof(token1Length));
            outFile.write(rule.first.first.c_str(), token1Length);
            outFile.write(reinterpret_cast<const char *>(&token2Length), sizeof(token2Length));
            outFile.write(rule.first.second.c_str(), token2Length);
            outFile.write(reinterpret_cast<const char *>(&combinedTokenLength), sizeof(combinedTokenLength));
            outFile.write(rule.second.c_str(), combinedTokenLength);
        }

        outFile.close();
    }

    template <typename T>
    void BPETokenizer<T>::load(const std::string filepath)
    {
        std::ifstream inFile(filepath, std::ios::binary);
        inFile.read(reinterpret_cast<char *>(&this->vocabularySize), sizeof(this->vocabularySize));

        unsigned short stringLength;
        unsigned short tokenID;
        std::string tempToken;

        for (int i = 0; i < this->vocabularySize - 1; i++)
        {
            inFile.read(reinterpret_cast<char *>(&stringLength), sizeof(stringLength));
            tempToken.resize(stringLength);
            inFile.read(&tempToken[0], stringLength);

            inFile.read(reinterpret_cast<char *>(&tokenID), sizeof(tokenID));
            this->addToVocabulary(tempToken, tokenID);
        }

        unsigned short mergeRuleSize = this->mergeRules.size();

        inFile.read(reinterpret_cast<char *>(&mergeRuleSize), sizeof(mergeRuleSize));

        std::string token1;
        unsigned short token1Length;
        std::string token2;
        unsigned short token2Length;
        std::string combinedToken;
        unsigned short combinedTokenLength;
        for (int i = 0; i < mergeRuleSize; i++)
        {
            inFile.read(reinterpret_cast<char *>(&token1Length), sizeof(token1Length));
            token1.resize(token1Length);
            inFile.read(&token1[0], token1Length);

            inFile.read(reinterpret_cast<char *>(&token2Length), sizeof(token2Length));
            token2.resize(token2Length);
            inFile.read(&token2[0], token2Length);

            inFile.read(reinterpret_cast<char *>(&combinedTokenLength), sizeof(combinedTokenLength));
            combinedToken.resize(combinedTokenLength);
            inFile.read(&combinedToken[0], combinedTokenLength);
            this->addToMergeRule(std::make_pair(token1, token2), combinedToken);
        }

        inFile.close();
    }

    // Visualization functions
    template <typename T>
    void BPETokenizer<T>::printTokenizedText(const std::vector<int> &tokenizedText) const
    {
        for (const auto &index : tokenizedText)
        {
            std::cout << index << " ";
        }
        std::cout << std::endl;
    }

    template <typename T>
    void BPETokenizer<T>::printWordWiseTokenList() const
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
    void BPETokenizer<T>::printVocabulary(const bool &detailed) const
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
    void BPETokenizer<T>::printPairFrequency() const
    {
        for (const auto &element : this->pairFrequency)
        {
            std::cout << "(" << element.first.first << ", " << element.first.second
                      << ")"
                      << ": " << element.second << std::endl;
        }
    }

    template <typename T>
    void BPETokenizer<T>::printMergeRules() const
    {
        for (const auto &element : this->mergeRules)
        {
            std::cout << "(" << element.first.first << ", " << element.first.second
                      << "): " << element.second << std::endl;
        }
    }

    // Testing functions
    template <typename T>
    bool BPETokenizer<T>::operator==(const BPETokenizer<T> &other) const
    {
        if (this->vocabularySize != other.vocabularySize || this->vocabulary.size() != other.vocabulary.size() || this->mergeRules.size() != other.mergeRules.size())
        {
            return false;
        }

        for (auto &element : this->vocabulary)
        {
            auto otherElementIter = other.vocabulary.find(element.first);
            if (otherElementIter == other.vocabulary.end() || otherElementIter->second != element.second)
            {
                return false;
            }
        }

        for (auto &element : this->mergeRules)
        {
            auto otherElementIter = other.mergeRules.find(element.first);
            if (otherElementIter == other.mergeRules.end() || otherElementIter->second != element.second)
            {
                return false;
            }
        }

        return true;
    }

} // namespace dokusha

template class dokusha::BPETokenizer<std::string>;