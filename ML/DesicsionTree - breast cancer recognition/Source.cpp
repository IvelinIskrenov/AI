#include <iostream>
#include <vector>
#include <cmath>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <utility>
#include <unordered_map>
#include <numeric>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <ctime>

const std::vector<std::string> MAIN_ATTRIBUTES = { "age","menopause","tumor-size","inv-nodes","node-caps","deg-malig","breast","breast-quad","irradiat" };

const int MAX_DEPTH = 6;
const int MIN_SAMPLES = 10;
const double MIN_G = 0.01;
const bool PRE_PRUNING = true;

int ZERO = 0;
int TWO = 0;
int THREE = 0;

struct Node
{
    std::string feature; // attribute to split on
    std::string value;   // Value of the feature to split on // maybe delete (we are not using this)
    std::string result;  //class classification if its leaf
    bool leaf;   //is a leaf?
    std::vector<Node*> childrens; // string for picked value and node
    Node() { feature = ""; value = ""; result = ""; leaf = false; };
    ~Node() {
        for (auto& child : childrens) {
            delete child;  // Deleting the child node
        }
    }
};


struct Example
{
    std::vector<std::string> attributes; //attributes
    std::string label;                   //class
};

struct DataSet
{
    std::vector<Example> examples = {};
    size_t size = 0;
};

bool readDataFromFile(const std::string& filename, DataSet& data);
std::vector<std::string> getCommonAtrToFill(const DataSet& data);
void fillMissingValues(DataSet& data);

double calculateEntropy(const std::vector<std::string>& examples);
double calculateInformationGain(const DataSet& examples, int attributeIndex);
int best_attribute(const DataSet& data, const std::vector<int>& removedAttr);
bool prePruning(const DataSet& data, int depth, const std::vector<int> removedAttr);
std::string getMostCommonClass(const DataSet& data);
DataSet filterData(const DataSet& data, int attributeIndex, std::string value);
std::vector<std::string> getAllAttributeValues(const DataSet& data, int atributIndex);
int bestSplit(const DataSet& data, int attributeIndex);//find best split position
std::pair<DataSet, DataSet> splitDataset(const DataSet& dataset, int attributeIndex, const int value);
Node* build_tree(const DataSet& data, int depth = 0, std::vector<int> removedAttr = {});
size_t getIndexForAttribute(const std::string& attribute);
std::string predict(const Node* node, const Example& example);
double calculateAccuracy(const Node* tree, const DataSet& data);
void reducedErrorPruning(Node* node, const DataSet& validationData);
void applyPostPruning(Node* root, const DataSet& validationData);
double testAccuracy(const Node* tree, const DataSet& testData);
void shuffleAndSplitData(const DataSet& data, DataSet& trainData, DataSet& testData, double persentage);
void startValues() { ZERO = 0; TWO = 0; THREE = 0; };
void splitDataIntoStratifiedKFolds(const DataSet& data, std::vector<DataSet>& folds, int k);
void crossValidationTry(const DataSet& data, int folds);

int main()
{
    std::vector<std::string> attributesMain = { "age","menopause","tumor-size","inv-nodes","node-caps","del-malig","breast","breast-quad","irradiat" };
    std::string pathData = "C:/Users/zozo2/Desktop/DesicsionTree/breast-cancer.txt";
    //loading data
    DataSet data;
    if (!readDataFromFile(pathData, data)) // read it from txt file
    {
        std::cerr << "Error reading data!" << std::endl;
        return 1;
    }

    DataSet trainData;
    DataSet testData;

    shuffleAndSplitData(data, trainData, testData, 0.8);
    std::cout << "Train data loaded!\n";

    // build tree on training data
    Node* tree = build_tree(trainData);
    applyPostPruning(tree, trainData);

    // 1. train data accuracy
    double trainAccuracy = calculateAccuracy(tree, trainData);
    std::cout << "Train Set Accuracy: " << trainAccuracy * 100 << "%" << std::endl;

    // 2. 10 cross-validation 
    crossValidationTry(data, 10);

    // 3. test subset accuracy
    double testAccuracyVal = testAccuracy(tree, testData);
    std::cout << "Test Set Accuracy: " << testAccuracyVal * 100 << "%" << std::endl;
    return 0;
}


//fill data set
bool readDataFromFile(const std::string& filename, DataSet& data) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Can't open the file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        Example example;

        std::string temp;
        for (int i = 0; i < 10; ++i) {
            if (!std::getline(ss, temp, ',')) {
                std::cerr << "Not enough data on the line: " << line << std::endl;
                return false;
            }
            if (i != 0)
            {
                example.attributes.push_back(temp);
            }
            else
            {
                example.label = temp;
            }
        }

        data.examples.push_back(example);
        data.size++;
    }
    fillMissingValues(data); 
    file.close();
    return true;
}

std::vector<std::string> getCommonAtrToFill(const DataSet& data)
{
    std::vector<std::string> fillValues;
    for (size_t i = 0; i < 9; i++)
    {
        std::unordered_map<std::string, int> valuesOfAtr;
        for (const Example ex : data.examples)
        {
            valuesOfAtr[ex.attributes[i]]++;
        }

        std::string mostCommon;
        int maxCount = 0;

        for (const std::pair<std::string, int> pair : valuesOfAtr) {
            if (pair.second > maxCount) {
                maxCount = pair.second;
                mostCommon = pair.first;
            }
        }
        fillValues.push_back(mostCommon);
    }
    return fillValues;
}

void fillMissingValues(DataSet& data)
{
    std::vector<std::string> missingValuesAtr;
    missingValuesAtr = getCommonAtrToFill(data);
    size_t dataCount = data.size;
    for (size_t i = 0; i < dataCount; i++)
    {
        for (size_t j = 0; j < 9; j++)
        {
            if (data.examples[i].attributes[j] == "?")
            {
                data.examples[i].attributes[j] = missingValuesAtr[j];
            }
        }
    }
}

//smallest value better
double calculateEntropy(const std::vector<Example>& examples)
{
    size_t S = examples.size();
    double entropy = 0.0;
    std::map<std::string, int> classCounts;

    for (size_t i = 0; i < S; i++)
    {
        classCounts[examples[i].label]++;
    }


    for (const std::pair<const std::string, int>& pair : classCounts)
    {
        double probability = (double)pair.second / S;
        entropy -= probability * log2(probability);
    }
    return entropy;
}

//biggest value better
double calculateInformationGain(const DataSet& data, int attributeIndex)
{
    std::map<std::string, std::vector<Example>> subsets;
    for (const Example example : data.examples)
    {
        subsets[example.attributes[attributeIndex]].push_back(example);//We separate the examples from the specific attribute 
    }

    double entropyBeforeSplit = calculateEntropy(data.examples);
    double weightedEntropyAfterSplit = 0.0;

    for (const std::pair<std::string, std::vector<Example>> subset : subsets)
    {
        weightedEntropyAfterSplit += ((double)subset.second.size() / data.size) * calculateEntropy(subset.second);
    }

    return entropyBeforeSplit - weightedEntropyAfterSplit;
}

std::vector<int> getAtrValInt(std::vector<std::string> uniqueValues)//get split points
{
    std::vector<int> result;
    int secondNum;
    size_t count = uniqueValues.size();
    for (size_t i = 0; i < count; i++)
    {
        size_t pos = uniqueValues[i].find('-');
        secondNum = std::stoi(uniqueValues[i].substr(pos + 1));
        result.push_back(secondNum);
    }
    std::sort(result.begin(),result.end());
    return result;
}

int bestSplit(const DataSet& data, int attributeIndex)
{
    std::vector<std::string> attributeValues; //strings
    std::vector<std::string> vec;

    for (const Example ex : data.examples) 
    {
        vec.push_back(ex.attributes[attributeIndex]);
    }

    std::unordered_set<std::string> unique_set(vec.begin(), vec.end());
    std::vector<std::string> unique_vec(unique_set.begin(), unique_set.end());
    std::vector<int> splitPoints = getAtrValInt(unique_vec);

    double bestGain = -1;
    int bestSplit = -1;
    size_t sCount = splitPoints.size() - 1;
    // We check between each consecutive value to create a potential split point
    for (size_t i = 0; i < sCount; ++i) // loop all split positions
    {
        // Split data into two subsets: values <= splitPoint and values > splitPoint
        std::vector<Example> subset1, subset2;
        for (const Example& ex : data.examples)
        {
            int secondNum;
            size_t pos = ex.attributes[attributeIndex].find('-');
            secondNum = std::stoi(ex.attributes[attributeIndex].substr(pos + 1));
            if (secondNum <= splitPoints[i])
            {
                subset1.push_back(ex);
            }
            else
            {
                subset2.push_back(ex);
            }
        }
        //// Calculate information gain for this split
        double gain = calculateInformationGain(data, attributeIndex);
        // If this split gives a better gain, update the best split
        if (gain > bestGain)
        {
            bestGain = gain;
            bestSplit = splitPoints[i];
        }
    }

    return bestSplit;
}

//add a array with remove attributes
int best_attribute(const DataSet& data, const std::vector<int>& removedAttr)
{
    double best_gain = -1000;
    int best_attribute = -1;
    size_t dataSize = data.size;
    double currentGain = -1;
    size_t rAsize = removedAttr.size();
    for (int i = 0; i < 9; i++)
    {
        bool skip = false;
        for (size_t j = 0; j < rAsize; j++)
        {
            if (removedAttr[j] == i)
            {
                skip = true;
            }
        }
        if (!skip)
        {
            currentGain = calculateInformationGain(data, i); 
            if (currentGain > best_gain)
            {
                best_gain = currentGain;
                best_attribute = i;
            }
        }
    }
    return best_attribute;
}

//prepruning try minSamples 10, maxDepth 6, add G
bool prePruning(const DataSet& data, int depth, const std::vector<int> removedAttr)
{
    // If maximum depth is reached or not enough samples, stop growing the tree
    if (depth >= MAX_DEPTH || data.size < MIN_SAMPLES)
    {
        return true;
    }
    int bestAtr = best_attribute(data, removedAttr);
    double bestAttributeGain = calculateInformationGain(data, bestAtr);
    if (bestAttributeGain < MIN_G)
    {
        return true;
    }
    return false;
}

std::string getMostCommonClass(const DataSet& data)
{
    size_t dataCount = data.size;
    std::string mostCommonClass;
    int mostCommonClassCount = -1;
    std::unordered_map<std::string, int> classify;//map
    for (size_t i = 0; i < dataCount; i++)
    {
        classify[data.examples[i].label]++;
    }
    for (const std::pair<std::string, int> pair : classify)
    {
        if (pair.second > mostCommonClassCount)
        {
            mostCommonClass = pair.first;
        }
    }
    return mostCommonClass;
}

//here we can get all the values from MAIN DATA SET AND TEST HOW IT WILL WORK
std::vector<std::string> getAllAttributeValues(const DataSet& data, int atributIndex)
{
    std::map<std::string, int> atrValue;
    std::vector<std::string> result;
    size_t dataCount = data.size;

    for (size_t i = 0; i < dataCount; i++)
    {
        atrValue[data.examples[i].attributes[atributIndex]]++;
    }
    for (const std::pair<std::string, int> pair : atrValue)
    {
        result.push_back(pair.first);
    }
    return result;
}

std::pair<DataSet, DataSet> splitDataset(const DataSet& dataset, int attributeIndex, const int value) {
    DataSet left, right;

    for (const Example example : dataset.examples) 
    {
        int secondNum;
        size_t pos = example.attributes[attributeIndex].find('-');
        secondNum = std::stoi(example.attributes[attributeIndex].substr(pos + 1));
        if (secondNum <= value) 
        {
            left.examples.push_back(example);
            left.size++;
        }
        else 
        {
            right.examples.push_back(example);
            right.size++;
        }
    }
    return { left, right };
}

DataSet filterData(const DataSet& data, int attributeIndex, std::string value)
{
    DataSet newData;
    size_t dataCount = data.size;
    for (const Example ex : data.examples)
    {
        if (ex.attributes[attributeIndex] == value)
        {
            newData.examples.push_back(ex);
            newData.size++;
        }
    }
    return newData;
}
bool isPure(const DataSet& data)
{
    std::map<std::string, int> classes;
    for (const auto& ex : data.examples)
    {
        classes[ex.label]++;
    }
    return classes.size() == 1;
}

//ID3 algorithm
Node* build_tree(const DataSet& data, int depth, std::vector<int> removedAttr)
{
    Node* node = new Node();

    if (isPure(data))
    {
        node->leaf = true;
        node->result = data.examples[0].label;
        node->childrens = {};
        return node;
    }

    //if all examples have the same class or pre-pruning, return a leaf
    if (PRE_PRUNING && prePruning(data, depth, removedAttr))
    {
        node->leaf = true;
        node->result = getMostCommonClass(data);
        node->childrens = {};
        return node;
    }

    if (removedAttr.size() >= 7)
    {
        node->leaf = true;
        node->result = getMostCommonClass(data);
        return node;
    }

    //Don't check for empty attributes because we have pre-pruning function

    int bestAttribute = best_attribute(data, removedAttr);
    int bestSplitPos = -1;
    if (bestAttribute == 0 || bestAttribute == 2 || bestAttribute == 3)
    {
        if (bestAttribute == 0 && ZERO < 2)
        {
            std::pair < std::string, double > bestArttrib;
            bestSplitPos = bestSplit(data, bestAttribute);
            ZERO++;
        }
        else if (bestAttribute == 2 && TWO < 2)
        {
            std::pair < std::string, double > bestArttrib;
            bestSplitPos = bestSplit(data, bestAttribute);
            TWO++;
        }
        else if(bestAttribute == 3 && THREE < 2)
        {
            std::pair < std::string, double > bestArttrib;
            bestSplitPos = bestSplit(data, bestAttribute);
            THREE++;
        }
    }
    if (bestSplitPos == -1)
    {
        removedAttr.push_back(bestAttribute);
        std::vector<std::string> childsVal = getAllAttributeValues(data, bestAttribute);
        size_t count = childsVal.size();
        for (size_t i = 0; i < count; i++)
        {
            //filter data
            DataSet newData = filterData(data, bestAttribute, childsVal[i]);
            Node* child = new Node();
            child = build_tree(newData, depth + 1, removedAttr);//i am not very sure for this
            child->feature = MAIN_ATTRIBUTES[bestAttribute];
            child->value = childsVal[i];
            node->childrens.push_back(child);
        }
        return node;
    }

    Node* childLeft = new Node();
    Node* childRight = new Node();
    std::pair<DataSet, DataSet> dataChild = splitDataset(data, bestAttribute, bestSplitPos);
    std::string splitPos;
    splitPos = std::to_string(bestSplitPos);

    childLeft = build_tree(dataChild.first, depth + 1, removedAttr);
    childLeft->feature = MAIN_ATTRIBUTES[bestAttribute];
    childLeft->value = splitPos;
    node->childrens.push_back(childLeft);

    childRight = build_tree(dataChild.second, depth + 1, removedAttr);
    childRight->feature = MAIN_ATTRIBUTES[bestAttribute];
    childRight->value = splitPos;
    node->childrens.push_back(childRight);

    return node;
}

size_t getIndexForAttribute(const std::string& attribute)
{
    size_t count = MAIN_ATTRIBUTES.size();
    for (size_t i = 0; i < count; i++)
    {
        if (MAIN_ATTRIBUTES[i] == attribute)
        {
            return i;
        }
    }
    return -1;
}

// predict funciton
std::string predict(const Node* node, const Example& example)
{
    if (node->leaf)
    {
        return node->result;
    }
    int index = getIndexForAttribute(node->childrens[0]->feature);

    // searching for attribute and the value
    if (index == 0 || index == 2 || index == 3)
    {
        int secondNumEx;
        size_t pos1 = example.attributes[index].find('-');
        secondNumEx = std::stoi(example.attributes[index].substr(pos1 + 1));

        int valueNumChild;
        size_t pos2 = node->childrens[0]->value.find('-');
        valueNumChild = std::stoi(node->childrens[0]->value.substr(pos2 + 1));

        if (secondNumEx <= valueNumChild)
        {
            predict(node->childrens[0], example);
        }
        if (secondNumEx > valueNumChild && node->childrens.size() == 2)// checks
        {
            predict(node->childrens[1], example);
        }
    }
    for (const auto& child : node->childrens)
    {   
        if (child->value == example.attributes[index])
        {
            return predict(child, example);
        }
    }

    return ""; //never print this
}

double calculateAccuracy(const Node* tree, const DataSet& data)
{
    int correct = 0;
    for (const Example& example : data.examples)
    {
        std::string predictedClass = predict(tree, example);
        if (predictedClass == example.label)
        {
            correct++;
        }
    }
    return (double)correct / data.size;
}

// Reduced Error Pruning
void reducedErrorPruning(Node* node, const DataSet& validationData)
{
    if (node == nullptr || node->leaf)
    {
        return;
    }

    for (auto& child : node->childrens)
    {
        reducedErrorPruning(child, validationData);
    }

    // Accuracy before pruning
    double beforePruningAccuracy = calculateAccuracy(node, validationData);

    // save the original information
    std::string originalFeature = node->feature;
    std::vector<Node*> originalChildrens = node->childrens;

    // cut and set the leaf
    node->leaf = true;
    node->result = getMostCommonClass(validationData);

    // check the accuracy after cutting
    double afterPruningAccuracy = calculateAccuracy(node, validationData);

    //if the accuracy increse or not decrease, return the before state
    if (afterPruningAccuracy < beforePruningAccuracy)
    {
        node->leaf = false;
        node->feature = originalFeature;
        node->childrens = originalChildrens;
    }
}

void applyPostPruning(Node* root, const DataSet& validationData)
{
    reducedErrorPruning(root, validationData);
}
// test the model (exmaples subset) (testData)
double testAccuracy(const Node* tree, const DataSet& testData)
{
    return calculateAccuracy(tree, testData);
}

void shuffleAndSplitData(const DataSet& data, DataSet& trainData, DataSet& testData, double persentage) 
{
    std::vector<Example> noRecidiv;
    std::vector<Example> withRecidiv;

    for (const auto& ex : data.examples) {
        if (ex.label == "no-recurrence-events") {
            noRecidiv.push_back(ex);
        }
        else if (ex.label == "recurrence-events") {
            withRecidiv.push_back(ex);
        }
    }
    //  Shuffel the data from the 2 classes
    std::random_shuffle(noRecidiv.begin(), noRecidiv.end());
    std::random_shuffle(withRecidiv.begin(), withRecidiv.end());

    //Split 80% for training and 20% for testing
    size_t noRecidivTrainSize = noRecidiv.size() * persentage;
    size_t withRecidivTrainSize = withRecidiv.size() * persentage;

    std::rotate(noRecidiv.begin(), noRecidiv.begin() /*+ offset*/, noRecidiv.end());
    std::rotate(withRecidiv.begin(), withRecidiv.begin() /*+ offset*/, withRecidiv.end());

    trainData.examples.clear();
    testData.examples.clear();

    //Train data
    trainData.examples.insert(trainData.examples.end(), noRecidiv.begin(), noRecidiv.begin() + noRecidivTrainSize);
    trainData.examples.insert(trainData.examples.end(), withRecidiv.begin(), withRecidiv.begin() + withRecidivTrainSize);
    trainData.size = trainData.examples.size();

    //test data
    testData.examples.insert(testData.examples.end(), noRecidiv.begin() + noRecidivTrainSize, noRecidiv.end());
    testData.examples.insert(testData.examples.end(), withRecidiv.begin() + withRecidivTrainSize, withRecidiv.end());
    testData.size = testData.examples.size();
}

void splitDataIntoStratifiedKFolds(const DataSet& data, std::vector<DataSet>& folds, int k) 
{
    // Separate examples by class
    std::vector<Example> classA, classB;
    for (const auto& ex : data.examples) {
        if (ex.label == "no-recurrence-events") {  // Class A
            classA.push_back(ex);
        }
        else if (ex.label == "recurrence-events") {  // Class B
            classB.push_back(ex);
        }
    }

    // Shuffle both classes
    std::srand(std::time(0));
    int randomNumber = std::rand() % 10 + 1;
    for (size_t i = 0; i < randomNumber; i++)
    {
        std::random_shuffle(classA.begin(), classA.end());
        std::random_shuffle(classB.begin(), classB.end());
    }

    // Create k empty folds
    folds.resize(k);

    // Divide each class into k folds while maintaining proportion
    size_t classAFoldSize = classA.size() / k;
    size_t classBFoldSize = classB.size() / k;

    for (int i = 0; i < k; ++i) {
        // Add proportional examples of Class A to the fold
        folds[i].examples.insert(folds[i].examples.end(),
            classA.begin() + i * classAFoldSize,
            classA.begin() + (i + 1) * classAFoldSize);

        // Add proportional examples of Class B to the fold
        folds[i].examples.insert(folds[i].examples.end(),
            classB.begin() + i * classBFoldSize,
            classB.begin() + (i + 1) * classBFoldSize);

        // Update fold size
        folds[i].size = folds[i].examples.size();
    }
}

void crossValidationTry(const DataSet& data, int folds) {
    std::vector<DataSet> stratifiedFolds;
    splitDataIntoStratifiedKFolds(data, stratifiedFolds, folds);

    std::vector<double> accuracies;

    for (int i = 0; i < folds; ++i) {
        // Use one fold as test data
        DataSet testData = stratifiedFolds[i];
        DataSet trainingData;

        // Merge the other folds into training data
        for (int j = 0; j < folds; ++j) {
            if (j != i) {
                trainingData.examples.insert(trainingData.examples.end(),
                    stratifiedFolds[j].examples.begin(),
                    stratifiedFolds[j].examples.end());
            }
        }
        trainingData.size = trainingData.examples.size();

        // Build and prune the tree
        Node* tree = build_tree(trainingData);
        applyPostPruning(tree, trainingData);

        // Test the model
        double accuracy = calculateAccuracy(tree, testData);
        accuracies.push_back(accuracy);
    }
    // Calculate mean and standard deviation
    double averageAccuracy = std::accumulate(accuracies.begin(), accuracies.end(), 0.0) / accuracies.size();
    double variance = 0.0;
    for (double acc : accuracies) {
        variance += std::pow(acc - averageAccuracy, 2);
    }
    double stdDev = std::sqrt(variance / accuracies.size());

    std::cout << "10-Fold Cross-Validation Results:" << std::endl;
    for (int i = 0; i < folds; ++i) {
        std::cout << "Accuracy Fold " << i + 1 << ": " << accuracies[i] * 100 << "%" << std::endl;
    }
    std::cout << "Average Accuracy: " << averageAccuracy * 100 << "%" << std::endl;
    std::cout << "Standard Deviation: " << stdDev * 100 << "%" << std::endl;
}