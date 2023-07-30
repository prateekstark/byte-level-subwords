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
        if (line[0] == ' ')
        {
            return;
        }
        line += " "; // In our case, we consider space as ending of the word!
        std::string currentWord;
        std::vector<T> tokens;
        for (size_t i = 0; i < line.size(); i++)
        {
            if (line[i] == ' ')
            {
                if (currentWord.size() > 15)
                {
                    currentWord = "" + line[i];
                    continue;
                }

                tokens.clear();
                for (int j = 0; j < currentWord.size(); j++)
                {
                    std::string token(1, currentWord[j]);
                    tokens.push_back(token);
                    this->addToVocabulary(token);
                }

                if (wordFrequency.find(currentWord) != wordFrequency.end())
                {
                    wordFrequency[currentWord] += 1;
                }
                else
                {
                    wordWiseTokenList[currentWord] = tokens;
                    wordFrequency[currentWord] = 1;
                }
                currentWord = "";
            }
            currentWord += line[i];
        }
    }

    template <typename T>
    void BPETokenizer<T>::computePairFrequency()
    {
        this->pairFrequency.clear();
        for (auto element : this->wordWiseTokenList)
        {
            std::string word = element.first;
            for (int i = 0; i < element.second.size() - 1; i++)
            {
                std::pair<std::string, std::string> pair =
                    std::pair<std::string, std::string>(element.second[i],
                                                        element.second[i + 1]);
                if (pairFrequency.find(pair) != pairFrequency.end())
                {
                    pairFrequency[pair] += wordFrequency[word];
                }
                else
                {
                    pairFrequency[pair] = wordFrequency[word];
                }
            }
        }
    }

    template <typename T>
    size_t BPETokenizer<T>::getVocabularySize()
    {
        return this->vocabulary.size();
    }

    template <typename T>
    std::pair<T, T> BPETokenizer<T>::findBestPair()
    {
        int maxOccurance = 0;
        std::pair<T, T> bestPair;
        for (auto element : this->pairFrequency)
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
    void BPETokenizer<T>::addToMergeRule(std::pair<T, T> bestPair,
                                         T combinedToken)
    {
        this->mergeRules[bestPair] = combinedToken;
    }

    template <typename T>
    void BPETokenizer<T>::addToVocabulary(T &token)
    {
        if (this->vocabulary.find(token) == this->vocabulary.end())
        {
            this->vocabulary[token] = this->vocabularySize;
            this->inverseVocabulary[this->vocabularySize] = token;
            this->vocabularySize++;
        }
    }

    template <typename T>
    void BPETokenizer<T>::updateWordWiseTokenList(T token1, T token2)
    {
        for (auto &element : this->wordWiseTokenList)
        {
            if (element.second.size() == 1)
            {
                continue;
            }

            for (std::vector<std::string>::iterator it = element.second.begin();
                 it < element.second.end() - 1;)
            {
                if (it->compare(token1) == 0 && (it + 1)->compare(token2) == 0)
                {
                    *it = token1 + token2;
                    it++;
                    if (it != element.second.end())
                    {
                        it = element.second.erase(it);
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
    void BPETokenizer<T>::printWordWiseTokenList()
    {
        for (auto element : this->wordWiseTokenList)
        {
            std::cout << element.first << ": (";
            for (std::string tokens : element.second)
            {
                std::cout << tokens << " ";
            }
            std::cout << "\b) " << std::endl;
        }
    }

    template <typename T>
    void BPETokenizer<T>::printVocabulary(bool detailed)
    {
        if (!detailed)
        {
            std::cout << "Vocabulary" << std::endl;
            for (auto element : this->vocabulary)
            {
                std::cout << element.first << " ";
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "Vocabulary" << std::endl;
            for (auto element : this->vocabulary)
            {
                std::cout << element.first << " " << element.second << std::endl;
            }
            // std::cout << std::endl;
        }
    }

    template <typename T>
    void BPETokenizer<T>::printPairFrequency()
    {
        for (auto &element : this->pairFrequency)
        {
            std::cout << "(" << element.first.first << ", " << element.first.second
                      << ")"
                      << ": " << element.second << std::endl;
        }
    }

    template <typename T>
    void BPETokenizer<T>::printMergeRules()
    {
        for (auto &element : this->mergeRules)
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
    void BPETokenizer<T>::printTokenizedText(std::vector<int> tokenizedText)
    {
        for (auto &index : tokenizedText)
        {
            std::cout << index << " ";
        }
        std::cout << std::endl;
    }

    template <typename T>
    std::vector<int> BPETokenizer<T>::tokenize(std::string text)
    {
        std::vector<int> tokenizedText;
        trim(text);

        text += " "; // In our case, we consider space as ending of the word!
        std::string currentWord;
        std::string token;

        std::vector<std::vector<T>> tokenList;
        for (size_t i = 0; i < text.size(); i++)
        {
            if (text[i] == ' ')
            {
                std::vector<T> tokens;
                for (int j = 0; j < currentWord.size(); j++)
                {
                    token = std::string(1, currentWord[j]);
                    tokens.push_back(token);
                }
                tokenList.push_back(tokens);
                currentWord = "";
            }
            currentWord += text[i];
        }

        // for (auto& word: tokenList) {
        //     for (auto &token: word) {
        //         std::cout << token << std::endl;
        //     }
        // }

        for (auto rule : this->mergeRules)
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

        for (auto &word : tokenList)
        {
            for (auto &token : word)
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
    std::string BPETokenizer<T>::detokenize(std::vector<int> tokenizedText)
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