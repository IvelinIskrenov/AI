#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <ostream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <numeric>

const size_t FEATURES = 16;
bool XRUN = true;
struct Attribute //single attribute apriority
{
	std::map<std::string ,std::pair<long double, long double>> apriority;// (for second pair) first is true for class A and second is false for class B (for the other class we will use 1 - first or second value)
    Attribute() {
        apriority["republican"].first = 0.0L; apriority["republican"].second = 0.0L;
        apriority["democrat"].first = 0.0L;;; apriority["democrat"].second = 0.0L;
    };
};

struct Apriority
{
	Attribute attributeTable[FEATURES]; // attribute values
};

struct Example
{
	char attributes[FEATURES]; // all features 
	std::string label; //class
};

struct Data
{
	std::vector<Example> examples;
};

class BernoulliNB {
private:
	std::map<std::string, double> classifyPr;
	Apriority table;
public:
	void calculateApriority(const Data& data);
	std::string predict(const Example& ex);
	double calculateAccuracy(const Data& data);
    void splitDataIntoStratifiedKFolds(const Data& data, std::vector<Data>& folds, int k);
    void crossValidation(const Data& data, int folds);

};

bool loading(const std::string& filename, Data& data);
void clearDataFromNoisyExamples(Data& data, int maxUnknownAttributes);
std::vector<char> mostCommonValues(Data& data); 
void fillMissingValues(Data& data);
void shuffleAndSplitData(const Data& data, Data& trainData, Data& testData, double percentage);
int charToInt(const char& letter);

int main()
{
    std::string filename = "house-votes-84.txt";
    Data data;
    if (!loading(filename, data)) // read it from txt file
    {
        std::cerr << "Error reading data!" << std::endl;
        return 1;
    }
    int x;
    std::cin >> x;
    if (x == 0)
    {
        XRUN == false;
    }
    if (XRUN)
    {
        clearDataFromNoisyExamples(data, 4);
        fillMissingValues(data);
    }
    Data trainData, testData;
    shuffleAndSplitData(data, trainData, testData, 0.8);
    BernoulliNB naiveB;
    //we have values '?' in the data
    naiveB.calculateApriority(trainData);

    // 1. train data accuracy
    double trainAccuracy = naiveB.calculateAccuracy(trainData);
    std::cout << "Train Set Accuracy: " << trainAccuracy * 100 << "%" << std::endl;

    // 2. 10 cross-validation 
    naiveB.crossValidation(data, 10);

    // 3. test subset accuracy
    double testAccuracyVal = naiveB.calculateAccuracy(testData);
    std::cout << "Test Set Accuracy: " << testAccuracyVal * 100 << "%" << std::endl;

	return 0;
}

void BernoulliNB::calculateApriority(const Data& data)
{
    for (const Example ex : data.examples)
    {
        classifyPr[ex.label]++;
        for (size_t i = 0; i < FEATURES; i++)
        {
            if (ex.attributes[i] == 'y')
            {
                table.attributeTable[i].apriority[ex.label].first++;
            }
            else if (ex.attributes[i] == 'n')
            {
                table.attributeTable[i].apriority[ex.label].second++;
            }
        }
    }
    double lambda = 1; //hyper param.
    double A = 2.0; //Possible values of attribute
    for (size_t i = 0; i < FEATURES; i++) // Using Laplace Smoothing
    {
        table.attributeTable[i].apriority["republican"].first = (table.attributeTable[i].apriority["republican"].first + lambda) / (classifyPr["republican"] + A * lambda);
        table.attributeTable[i].apriority["republican"].second = (table.attributeTable[i].apriority["republican"].second + lambda) / (classifyPr["republican"] + A * lambda);
        table.attributeTable[i].apriority["democrat"].first = (table.attributeTable[i].apriority["democrat"].first + lambda) / (classifyPr["democrat"] + A * lambda);
        table.attributeTable[i].apriority["democrat"].second = (table.attributeTable[i].apriority["democrat"].second + lambda) / (classifyPr["democrat"] + A * lambda);
    }
}

std::string BernoulliNB::predict(const Example& ex)
{
    double classRepublican = 0;
    double classDemocrat = 0;
    for (size_t i = 0; i < FEATURES; i++)
    {
        if (XRUN && ex.attributes[i] == '?') // we don't take the '?' under consideration, we skip it
        {
            continue;
        }
        //for class Republican
        int x = charToInt(ex.attributes[i]);
        double logValue = table.attributeTable[i].apriority["republican"].first * x + (1.0 - table.attributeTable[i].apriority["republican"].first) * (1 - x);
        classRepublican += log(logValue);

        //for class Democrat
        logValue = table.attributeTable[i].apriority["democrat"].first * x + (1.0 - table.attributeTable[i].apriority["democrat"].first) * (1 - x);
        classDemocrat += log(logValue);
    }
    double value = classifyPr["republican"] / (classifyPr["republican"] + classifyPr["democrat"]);
    classRepublican += log(value);
    value = classifyPr["democrat"] / (classifyPr["republican"] + classifyPr["democrat"]);
    classDemocrat += log(value);

    if (classRepublican > classDemocrat)
    {
        return "republican";
    }
    else
    {
        return "democrat";
    }
}

double BernoulliNB::calculateAccuracy(const Data& data)
{
    int correct = 0;
    for (const Example& example : data.examples)
    {
        std::string predictedClass = predict(example);
        if (predictedClass == example.label)
        {
            correct++;
        }
    }
    return (double)correct / data.examples.size();
}

void BernoulliNB::splitDataIntoStratifiedKFolds(const Data& data, std::vector<Data>& folds, int k)
{
    std::vector<Example> classA, classB;
    for (const auto& ex : data.examples) {
        if (ex.label == "republican") {  // Class A
            classA.push_back(ex);
        }
        else if (ex.label == "democrat") {  // Class B
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
    }
}

void BernoulliNB::crossValidation(const Data& data, int folds)
{
    std::vector<Data> stratifiedFolds;
    splitDataIntoStratifiedKFolds(data, stratifiedFolds, folds);
    std::vector<double> accuracies;

    for (int i = 0; i < folds; ++i) {
        // Use one fold as test data
        Data testData = stratifiedFolds[i];
        Data trainingData;

        // Merge the other folds into training data
        for (int j = 0; j < folds; ++j) {
            if (j != i) {
                trainingData.examples.insert(trainingData.examples.end(),
                    stratifiedFolds[j].examples.begin(),
                    stratifiedFolds[j].examples.end());
            }
        }

        BernoulliNB naiveBayes;
        naiveBayes.calculateApriority(trainingData);

        // Test the model
        double accuracy = naiveBayes.calculateAccuracy(testData);
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

bool loading(const std::string& filename, Data& data)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Can't open the file: " << filename << std::endl;
        return false;
    }
    std::string line;
    while (getline(file, line))
    {
        std::stringstream ss(line);
        std::string value;
        Example ex;
        size_t count = 0;
        while (getline(ss, value, ','))
        {
            if (value == "y" || value == "n" || value == "?")
            {
                ex.attributes[count] = value[0];
                ++count;
            }
            else
            {
                ex.label = value;
            }
        }
        data.examples.push_back(ex);
    }
    return true;
}

void clearDataFromNoisyExamples(Data& data, int maxUnknownAttributes)
{
    if (maxUnknownAttributes <= 2)
    {
        std::cout << "Data loss!\n";
        return;
    }
    std::vector<int> forRemove; 
    size_t count = data.examples.size();
    for (size_t i = 0; i < count; i++)
    {
        int currentUnknownCount = 0;
        for (size_t i = 0; i < FEATURES; ++i)
        {
            if (data.examples[i].attributes[i] == '?')
            {
                ++currentUnknownCount;
            }
        }
        if (currentUnknownCount > maxUnknownAttributes)
        {
            forRemove.push_back(i);
        }
    }

    size_t dataSize = data.examples.size();
    std::sort(forRemove.rbegin(), forRemove.rend());
    for (int index : forRemove) 
    {
        if (index >= 0 && index < dataSize) 
        { 
            data.examples.erase(data.examples.begin() + index);
        }
    }
}

std::vector<char> mostCommonValues(Data& data)
{
    std::vector<char> result;
    size_t count = data.examples.size();
    std::vector<std::pair<int, int>> helpVector;
    std::pair<int, int> x;
    x.first = 0;
    x.second = 0;
    for (size_t i = 0; i < FEATURES; i++)
    {
        helpVector.push_back(x);
    }
    for (const Example ex : data.examples)
    {
        for (size_t i = 0; i < FEATURES; i++)
        {
            if (ex.attributes[i] == 'y')
            {
                helpVector[i].first++;
            }
            else if (ex.attributes[i] == 'n')
            {
                helpVector[i].second++;
            }
        }
    }
    for (size_t i = 0; i < FEATURES; i++)
    {
        if (helpVector[i].first > helpVector[i].second)
        {
            result.push_back('y');
        }
        else
        {
            result.push_back('n');
        }
    }
    return result;
}

void fillMissingValues(Data& data)
{
    std::vector<char> mostCommonVal = mostCommonValues(data);
    for (Example& ex : data.examples)
    {
        for (size_t i = 0; i < FEATURES; i++)
        {
            if (ex.attributes[i] == '?')
            {
                ex.attributes[i] = mostCommonVal[i];
            }
        }
    }
}

void shuffleAndSplitData(const Data& data, Data& trainData, Data& testData, double percentage)
{
    std::vector<Example> republican;
    std::vector<Example> democrat;

    for (const auto& ex : data.examples) {
        if (ex.label == "republican") {
            republican.push_back(ex);
        }
        else if (ex.label == "democrat") {
            democrat.push_back(ex);
        }
    }
    //  Shuffle the data from the 2 classes
    std::random_shuffle(republican.begin(), republican.end());
    std::random_shuffle(democrat.begin(), democrat.end());

    //Split 80% for training and 20% for testing
    size_t republicanTrainSize = republican.size() * percentage;
    size_t democratTrainSize = democrat.size() * percentage;

    std::rotate(republican.begin(), republican.begin(), republican.end());
    std::rotate(democrat.begin(), democrat.begin(), democrat.end());

    trainData.examples.clear();
    testData.examples.clear();

    //Train data
    trainData.examples.insert(trainData.examples.end(), republican.begin(), republican.begin() + republicanTrainSize);
    trainData.examples.insert(trainData.examples.end(), democrat.begin(), democrat.begin() + democratTrainSize);

    //test data
    testData.examples.insert(testData.examples.end(), republican.begin() + republicanTrainSize, republican.end());
    testData.examples.insert(testData.examples.end(), democrat.begin() + democratTrainSize, democrat.end());
}

int charToInt(const char& letter)
{
    if (letter == 'y')
    {
        return 1;
    }
    else if (letter == 'n')
    {
        return 0;
    }
}