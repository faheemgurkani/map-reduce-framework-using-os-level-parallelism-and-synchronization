#include <iostream>
#include <pthread.h>
#include <cstring>
#include <sstream> // For stringstream

using namespace std;

#define MAX_WORDS 1000
#define MAX_KEYS 100

// Mutex for synchronization
pthread_mutex_t lock;

// Shared data
char* words[MAX_WORDS];          // Dynamically store words from the sentence
int wordCount = 0;               // Number of words in the input array
char intermediateKeys[MAX_WORDS][50]; // Array for intermediate keys
int intermediateValues[MAX_WORDS];    // Array for intermediate values
int intermediateCount = 0;            // Count of intermediate pairs

char groupedKeys[MAX_KEYS][50];  // Array for grouped keys
int groupedValues[MAX_KEYS];     // Array for grouped values
int groupedCount = 0;            // Count of grouped keys

char finalKeys[MAX_KEYS][50];    // Array for final keys
int finalValues[MAX_KEYS];       // Array for final values
int finalCount = 0;              // Count of final keys
int reduceIndices[MAX_KEYS]; // MAX_KEYS should be large enough to hold thread indices

// Function to tokenize the sentence into words
void tokenizeSentence(const std::string& sentence) 
{
    std::istringstream stream(sentence);
    std::string word;

    while (stream >> word) 
    {
        words[wordCount] = new char[word.length() + 1];
    
        strcpy(words[wordCount], word.c_str());
    
        wordCount++;
    }
}

// Map function
// void* mapPhase(void* arg) 
// {
//     int startIdx = *(int*)arg;
//     int chunkSize = (wordCount + 1) / 2; // Handling odd word counts
//     int endIdx = startIdx + chunkSize;

//     for (int i = startIdx; i < endIdx && i < wordCount; ++i) 
//     {
//         pthread_mutex_lock(&lock);
    
//         strcpy(intermediateKeys[intermediateCount], words[i]);
    
//         intermediateValues[intermediateCount] = 1;
    
//         intermediateCount++;
    
//         pthread_mutex_unlock(&lock);
//     }

//     return nullptr;
// }

void* mapPhase(void* arg) 
{
    int startIdx = *(int*)arg;
    int endIdx = *((int*)arg + 1);

    for (int i = startIdx; i < endIdx && i < wordCount; ++i) 
    {
        pthread_mutex_lock(&lock);
    
        strcpy(intermediateKeys[intermediateCount], words[i]);
    
        intermediateValues[intermediateCount] = 1;
    
        intermediateCount++;
    
        pthread_mutex_unlock(&lock);
    }
    
    return nullptr;
}

// Shuffle function
// void shufflePhase() 
// {
//     for (int i = 0; i < intermediateCount; ++i) 
//     {
//         bool found = false;
    
//         for (int j = 0; j < groupedCount; ++j) 
//         {
//             if (strcmp(groupedKeys[j], intermediateKeys[i]) == 0) 
//             {
//                 groupedValues[j]++;
            
//                 found = true;
            
//                 break;
//             }
//         }
//         if (!found) 
//         {
//             strcpy(groupedKeys[groupedCount], intermediateKeys[i]);
        
//             groupedValues[groupedCount] = 1;
        
//             groupedCount++;
//         }
//     }
// }

void shufflePhase() 
{
    groupedCount = 0; // Reset groupedCount to ensure fresh calculation

    for (int i = 0; i < intermediateCount; ++i) 
    {
        bool found = false;
    
        for (int j = 0; j < groupedCount; ++j) 
        {
            if (strcmp(groupedKeys[j], intermediateKeys[i]) == 0) 
            {
                groupedValues[j]++;
            
                found = true;
            
                break;
            }
        }
        
        if (!found) 
        {
            strcpy(groupedKeys[groupedCount], intermediateKeys[i]);
        
            groupedValues[groupedCount] = 1;
        
            groupedCount++;
        }
    }
}

// Reduce function
// void* reducePhase(void* arg) 
// {
//     int startIdx = *(int*)arg;
//     int chunkSize = (groupedCount + 1) / 2; // Handling odd grouped counts
//     int endIdx = startIdx + chunkSize;

//     for (int i = startIdx; i < endIdx && i < groupedCount; ++i) 
//     {
//         pthread_mutex_lock(&lock);
    
//         strcpy(finalKeys[finalCount], groupedKeys[i]);
    
//         finalValues[finalCount] = groupedValues[i];
    
//         finalCount++;
    
//         pthread_mutex_unlock(&lock);
//     }
 
//     return nullptr;
// }

// void* reducePhase(void* arg) 
// {
//     int startIdx = *(int*)arg;
//     int endIdx = *((int*)arg + 1);

//     for (int i = startIdx; i < endIdx && i < groupedCount; ++i) 
//     {
//         pthread_mutex_lock(&lock);
    
//         strcpy(finalKeys[finalCount], groupedKeys[i]);
    
//         finalValues[finalCount] = groupedValues[i];
    
//         finalCount++;
    
//         pthread_mutex_unlock(&lock);
//     }
    
//     return nullptr;
// }

void* reducePhase(void* arg) 
{
    int* range = (int*)arg; // Casting the argument
    int startIdx = range[0];
    int endIdx = range[1];

    delete[] range; // Free allocated memory for this thread

    for (int i = startIdx; i < endIdx && i < groupedCount; ++i) 
    {
        pthread_mutex_lock(&lock);

        strcpy(finalKeys[finalCount], groupedKeys[i]);
 
        finalValues[finalCount] = groupedValues[i];
 
        finalCount++;

        pthread_mutex_unlock(&lock);
    }
 
    return nullptr;
}

int calculateThreadCount(int totalItems, int maxThreads) 
{
    return std::min(totalItems, maxThreads);
}

int main() 
{
    // Initializing mutex
    pthread_mutex_init(&lock, nullptr);

    // Inputting the sentence
    std::string sentence;
    std::cout << "Please, input a sentence: ";
    std::getline(std::cin, sentence);

    // Validating input
    if (sentence.empty()) 
    {
        std::cerr << "Error: Empty sentence entered.\n";
    
        return 1;
    }

    // cout << "A" << endl;    // For, testing

    // Tokenizing sentence into words
    tokenizeSentence(sentence);

    // cout << "A" << endl;    // For, testing

    // Mapping Phase
    // pthread_t mapThreads[2];
    
    // Alternate implementation
    int maxThreads = 15; // Can be set this based on your system's capabilities.
    int mapThreadCount = calculateThreadCount(wordCount, maxThreads);
    int reduceThreadCount = calculateThreadCount(groupedCount, maxThreads);

    // cout << "A" << endl;    // For, testing

    pthread_t mapThreads[mapThreadCount];

    // cout << "A" << endl;    // For, testing

    int mapIndices[mapThreadCount + 1];

    // cout << "A" << endl;    // For, testing

    for (int i = 0; i <= mapThreadCount; ++i)
        mapIndices[i] = i * (wordCount / mapThreadCount);

    // cout << "A" << endl;    // For, testing

    for (int i = 0; i < mapThreadCount; ++i)
        pthread_create(&mapThreads[i], nullptr, mapPhase, &mapIndices[i]);

    // cout << "A" << endl;    // For, testing

    for (int i = 0; i < mapThreadCount; ++i)
        pthread_join(mapThreads[i], nullptr);

    // cout << "A" << endl;    // For, testing

    // int indices[2] = {0, (wordCount + 1) / 2};

    // for (int i = 0; i < 2; ++i)
    //     pthread_create(&mapThreads[i], nullptr, mapPhase, &indices[i]);
    
    // for (int i = 0; i < 2; ++i)
    //     pthread_join(mapThreads[i], nullptr);

    // Shuffling Phase
    shufflePhase();

    // cout << "B" << endl;    // For, testing

    // Reducing Phase
    // pthread_t reduceThreads[2];

    if (groupedCount == 0)
        std::cout << "No data to process in the reduce phase.\n";
    else 
    {
        // Proceeding with reduce thread creation and workload distribution
        int reduceThreadCount = std::min(groupedCount, MAX_WORDS);
        int baseWorkload = groupedCount / reduceThreadCount;
        int extraWorkload = groupedCount % reduceThreadCount;

        reduceIndices[0] = 0;

        for (int i = 1; i <= reduceThreadCount; ++i)
            reduceIndices[i] = reduceIndices[i - 1] + baseWorkload + (i <= extraWorkload ? 1 : 0);

        pthread_t reduceThreads[reduceThreadCount];

        for (int i = 0; i < reduceThreadCount; ++i) 
        {
            int* range = new int[2];
        
            range[0] = reduceIndices[i];
            range[1] = reduceIndices[i + 1];

            pthread_create(&reduceThreads[i], nullptr, reducePhase, range);
        }

        for (int i = 0; i < reduceThreadCount; ++i) 
            pthread_join(reduceThreads[i], nullptr);
    }

    // int reduceIndices[2] = {0, (groupedCount + 1) / 2};

    // for (int i = 0; i < 2; ++i)
    //     pthread_create(&reduceThreads[i], nullptr, reducePhase, &reduceIndices[i]);

    // for (int i = 0; i < 2; ++i)
    //     pthread_join(reduceThreads[i], nullptr);

    // Displaying the output
    // std::cout << "Final Output:\n";
    // for (int i = 0; i < finalCount; ++i) 
    //     std::cout << finalKeys[i] << ": " << finalValues[i] << "\n";

    if (finalCount == 0)
        std::cout << "No final results to display.\n";
    else 
    {
        std::cout << "\nFinal Output:\n";
    
        for (int i = 0; i < finalCount; ++i)
            std::cout << "> " << finalKeys[i] << ": " << finalValues[i] << "\n";
    }

    // Cleaning-up dynamically allocated memory
    for (int i = 0; i < wordCount; ++i)
        delete[] words[i];

    // Destroying mutex
    pthread_mutex_destroy(&lock);

    return 0;
}
