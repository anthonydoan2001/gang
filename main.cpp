#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include <queue>
#include <thread>
#include <map>
#include <pthread.h>
#include <algorithm>

using namespace std;

struct Node{
    char character;
    int frequency;
    Node* left;
    Node* right;
    Node(){
        character = '\0';
        frequency = 0;
        left = NULL;
        right = NULL;
    }
    Node(char c, int f){
        character = c;
        frequency = f;
        left = NULL;
        right = NULL;
    }
    ~Node() {
        delete left;
        delete right;
    }
};

struct compare {
    bool operator()(Node* left, Node* right) {
        if (left->frequency == right->frequency) {
            if (left->character == right->character) {
                return false;
            }
            return left->character > right->character;
        }
        return left->frequency > right->frequency;
    }
};

priority_queue<Node*, vector<Node*>, compare> pq;
queue<char> compressedData;
Node* root = NULL;
int numThreads;
map<char, int> frequencyMap;
pthread_mutex_t compressedDataMutex = PTHREAD_MUTEX_INITIALIZER;

void traverse(Node* node, string code, const vector<char>& characters, vector<string>& huffmanCodes) {
    if (node == nullptr) {
        return;
    }
    if (node->character != '\0') {
        int index = find(characters.begin(), characters.end(), node->character) - characters.begin();
        huffmanCodes[index] = code;
    }
    if (node->left) {
        traverse(node->left, code + "0", characters, huffmanCodes);
    }
    if (node->right) {
        traverse(node->right, code + "1", characters, huffmanCodes);
    }
    delete node;
}

void* decompress(void* threadId) {
    long tid = (long) threadId;
    while (true) {
        pthread_mutex_lock(&compressedDataMutex);
        if (compressedData.empty()) {
            pthread_mutex_unlock(&compressedDataMutex);
            break;
        }
        Node* node = root;
        while (node->left || node->right) {
            char bit = compressedData.front();
            compressedData.pop();
            if (bit == '0') {
                node = node->left;
            } else {
                node = node->right;
            }
        }
        cout << node->character;
        pthread_mutex_unlock(&compressedDataMutex);
    }
    pthread_exit(NULL);
}
int main() {
    string inputFileName, compressedFileName;
    cin >> inputFileName >> compressedFileName;
    ifstream inputFile(inputFileName), compressedFile(compressedFileName);

    char c;
    int freq;
    while (inputFile >> noskipws >> c) {
        inputFile >> freq;
        frequencyMap[c] = freq;
        pq.push(new Node(c, freq));
    }
    inputFile.close();
    
    while(pq.size() > 1){
        Node* nodeOne = pq.top();
        pq.pop();
        Node* nodeTwo = pq.top();
        pq.pop();
        Node* nodeThree = new Node;
        nodeThree->frequency = nodeOne->frequency + nodeTwo->frequency;
        nodeThree->character = '\0';
        nodeThree->left = nodeOne;
        nodeThree->right = nodeTwo;
        pq.push(nodeThree);
    }
    root = pq.top();
    pq.pop();
    
    char byte;
    while (compressedFile.get(byte)) {
        for (int i = 7; i >= 0; i--) {
            char t = (byte >> i) & 1;
            compressedData.push(t);
        }
    }
    compressedFile.close();
    numThreads = thread::hardware_concurrency();
    if (numThreads == 0) {
        numThreads = 1;
    }
    pthread_t threads[numThreads];
    for (long i = 0; i < numThreads; i++) {
        pthread_create(&threads[i], NULL, decompress, (void*) i);
    }
    for (long i = 0; i < numThreads; i++) {
       	pthread_join(threads[i], NULL);
    }
    return 0;
}
