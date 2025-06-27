#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <random>
#include <limits>
#include <iomanip>;

struct DataPointExamles 
{
    std::vector<double> values;
    std::string label;
};

// Node for the k-D tree
struct KDNode {
    DataPointExamles point;
    KDNode* left;
    KDNode* right;
    ~KDNode() {
        delete left;  // Recursively delete the left subtree
        delete right; // Recursively delete the right subtree
    }
};

void loadingData(const std::string& filename, std::vector<DataPointExamles>& data)
{
    std::ifstream file(filename);
    std::string line;
    while (getline(file, line))
    {
        std::stringstream ss(line);
        std::string value;

        DataPointExamles point;
        while (getline(ss, value, ','))
        {
            if (isdigit(value[0]) || value[0] == '-')
            {
                point.values.push_back(std::stod(value));
            }
            else
            {
                point.label = value;
            }
        }
        data.push_back(point);
    }
}

// Function to calculate Euclidean distance
double euclideanDistance(const std::vector<double>& a, const std::vector<double>& b) {
    double sum = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        sum += pow(a[i] - b[i], 2);
    }
    return sqrt(sum);
}

// Comparator for sorting points by a specific dimension
bool compareByDimension(const DataPointExamles& a, const DataPointExamles& b, int dim) {
    return a.values[dim] < b.values[dim];
}

// Build k-D tree recursively
KDNode* buildKDTree(std::vector<DataPointExamles>& points, int depth) {
    if (points.empty()) return nullptr;

    int dim = depth % points[0].values.size();
    sort(points.begin(), points.end(), [dim](const DataPointExamles& a, const DataPointExamles& b) {
        return compareByDimension(a, b, dim);
        });

    int median = points.size() / 2;
    KDNode* node = new KDNode;
    node->point = points[median];

    std::vector<DataPointExamles> left(points.begin(), points.begin() + median);
    std::vector<DataPointExamles> right(points.begin() + median + 1, points.end());

    node->left = buildKDTree(left, depth + 1);
    node->right = buildKDTree(right, depth + 1);

    return node;
}

// k-NN search using k-D tree
void kNearestNeighbors(KDNode* root, const DataPointExamles& target, int k, int depth,
    std::vector<std::pair<double, std::string>>& neighbors)
{
    if (!root) return;

    int dim; 
    int tVs = target.values.size();
    dim = depth % tVs;
    double dist = euclideanDistance(target.values, root->point.values);

    if (neighbors.size() < k) 
    {
        neighbors.push_back({ dist, root->point.label });
        push_heap(neighbors.begin(), neighbors.end(), std::greater<>());
    }
    else if (dist < neighbors.front().first) 
    {
        pop_heap(neighbors.begin(), neighbors.end(), std::greater<>());
        neighbors.pop_back();
        neighbors.push_back({ dist, root->point.label });
        push_heap(neighbors.begin(), neighbors.end(), std::greater<>());
    }

    KDNode* next = target.values[dim] < root->point.values[dim] ? root->left : root->right;
    KDNode* other = next == root->left ? root->right : root->left;

    kNearestNeighbors(next, target, k, depth + 1, neighbors);

    if (neighbors.size() < k || fabs(target.values[dim] - root->point.values[dim]) < neighbors.front().first) 
    {
        kNearestNeighbors(other, target, k, depth + 1, neighbors);
    }
}

//Predict the label of a target point
std::string predictLabel(KDNode* root, const DataPointExamles& target, int k) {
    std::vector<std::pair<double, std::string>> neighbors;
    kNearestNeighbors(root, target, k, 0, neighbors);

    std::map<std::string, int> labelCount;
    for (const auto& neighbor : neighbors) {
        labelCount[neighbor.second]++;
    }

    std::string bestLabel;
    int maxCount = 0;
    for (const std::pair<std::string,int> cl : labelCount) {
        if (cl.second > maxCount) {
            maxCount = cl.second;
            bestLabel = cl.first;
        }
    }

    return bestLabel;
}

//We don't need normalize data, beacuse all values are in cm! We need normalization when we have different units of measuremen

void shuffleAndSplitData(const std::vector<DataPointExamles>& data, std::vector<DataPointExamles>& trainData, std::vector<DataPointExamles>& testData, double persentage)
{
    std::vector<DataPointExamles> Setosa;
    std::vector<DataPointExamles> Versicolour;
    std::vector<DataPointExamles> Virginica;

    for (const auto& ex : data) {
        if (ex.label == "Iris-setosa") {
            Setosa.push_back(ex);
        }
        else if (ex.label == "Iris-versicolor") {
            Versicolour.push_back(ex);
        }
        else if (ex.label == "Iris-virginica")
        {
            Virginica.push_back(ex);
        }
    }
    //  Shuffel the data from the 3 classes
    std::random_shuffle(Setosa.begin(), Setosa.end());
    std::random_shuffle(Versicolour.begin(), Versicolour.end());
    std::random_shuffle(Virginica.begin(), Virginica.end());

    //Split 80% for training and 20% for testing
    size_t setosaSize = Setosa.size() * persentage;
    size_t versicolourSize = Versicolour.size() * persentage;
    size_t virginicaSize = Virginica.size() * persentage;

    std::rotate(Setosa.begin(), Setosa.begin(), Setosa.end());
    std::rotate(Versicolour.begin(), Versicolour.begin(), Versicolour.end());
    std::rotate(Virginica.begin(), Virginica.begin(), Virginica.end());

    trainData.clear();
    testData.clear();

    //Train data
    trainData.insert(trainData.end(), Setosa.begin(), Setosa.begin() + setosaSize);
    trainData.insert(trainData.end(), Versicolour.begin(), Versicolour.begin() + versicolourSize);
    trainData.insert(trainData.end(), Virginica.begin(), Virginica.begin() + virginicaSize);

    //test data
    testData.insert(testData.end(), Setosa.begin() + setosaSize, Setosa.end());
    testData.insert(testData.end(), Versicolour.begin() + versicolourSize, Versicolour.end());
    testData.insert(testData.end(), Virginica.begin() + virginicaSize, Virginica.end());
}

void splitDataIntoStratifiedKFolds(const std::vector<DataPointExamles>& data, std::vector<std::vector<DataPointExamles>>& folds, int k) {
    // Separate examples by class
    std::vector<DataPointExamles> Setosa, Versicolour, Virginica;

    for (const auto& ex : data) {
        if (ex.label == "Iris-setosa") {
            Setosa.push_back(ex);
        }
        else if (ex.label == "Iris-versicolor") {
            Versicolour.push_back(ex);
        }
        else if (ex.label == "Iris-virginica")
        {
            Virginica.push_back(ex);
        }
    }
    std::random_shuffle(Setosa.begin(), Setosa.end());
    std::random_shuffle(Versicolour.begin(), Versicolour.end());
    std::random_shuffle(Virginica.begin(), Virginica.end());

    // Create k empty folds
    folds.resize(k);

    // Divide each class into k folds while maintaining proportion
    size_t setosaSize = Setosa.size();
    size_t versicolourSize = Versicolour.size();
    size_t virginicaSize = Virginica.size();

    size_t setosaFoldSize = setosaSize / k;
    size_t versicolourFoldSize = versicolourSize / k;
    size_t virginicaFoldSize = virginicaSize / k;

    for (int i = 0; i < k; ++i) {
        // Add proportional examples of Class A to the fold
        folds[i].insert(folds[i].end(),
            Setosa.begin() + i * setosaFoldSize,
            Setosa.begin() + (i + 1) * setosaFoldSize);

        // Add proportional examples of Class B to the fold
        folds[i].insert(folds[i].end(),
            Versicolour.begin() + i * versicolourFoldSize,
            Versicolour.begin() + (i + 1) * versicolourFoldSize);

        folds[i].insert(folds[i].end(),
            Virginica.begin() + i * virginicaFoldSize,
            Virginica.begin() + (i + 1) * virginicaFoldSize);
    }
}

double calculateAccuracy(std::vector<DataPointExamles> testData, KDNode* root, int k)
{
    int correct = 0;
    for (const auto& testPoint : testData) {
        std::string predicted = predictLabel(root, testPoint, k);
        if (predicted == testPoint.label) {
            correct++;
        }
    }
    return (correct / static_cast<double>(testData.size()));
}

void crossValidationTry(const std::vector<DataPointExamles>& data, int folds, int k) {
    std::vector<std::vector<DataPointExamles>> stratifiedFolds;
    splitDataIntoStratifiedKFolds(data, stratifiedFolds, folds);

    std::vector<double> accuracies;

    for (int i = 0; i < folds; ++i) {
        // Use one fold as test data
        std::vector<DataPointExamles> testData = stratifiedFolds[i];
        std::vector<DataPointExamles> trainingData;

        // Merge the other folds into training data
        for (int j = 0; j < folds; ++j) {
            if (j != i) {
                trainingData.insert(trainingData.end(),
                    stratifiedFolds[j].begin(),
                    stratifiedFolds[j].end());
            }
        }
        KDNode* root = buildKDTree(trainingData, 0);
        // Test the model
        double accuracy = calculateAccuracy(testData, root, k);
        accuracies.push_back(accuracy);
    }

    // Calculate mean and standard deviation
    double sum = 0.0;
    for (double acc : accuracies) {
        sum += acc;
    }
    double averageAccuracy = sum / accuracies.size();
    double variance = 0.0;
    for (double acc : accuracies) {
        variance += std::pow(acc - averageAccuracy, 2);
    }
    double stdDev = std::sqrt(variance / accuracies.size());

    // Output results
    std::cout << "K-Fold Cross-Validation Results (" << folds << "-fold):" << std::endl;
    for (int i = 0; i < folds; ++i) {
        std::cout << "Accuracy for Fold " << i + 1 << ": " << std::setprecision(4) << accuracies[i] * 100 << "%" << std::endl;
    }
    std::cout << "Average Accuracy: " << std::setprecision(4) << averageAccuracy * 100 << "%" << std::endl;
    std::cout << "Standard Deviation: " << std::setprecision(4) << stdDev * 100 << "%" << std::endl;
}

int main() {
    std::vector<DataPointExamles> data;
    std::string filename = "C:/Users/zozo2/Desktop/AI/Homeworks/kNN/iris.txt";
    loadingData(filename, data);

    // Split data
    std::vector<DataPointExamles> trainData, testData;

    shuffleAndSplitData(data, trainData, testData, 0.8); //80:20 1:1:1

    // Build k-D tree
    KDNode* root = buildKDTree(trainData, 0);

    //Test the model
    int k = 11;
    std::cout << "Test Set Accuracy: " << std::setprecision(4) << calculateAccuracy(testData, root, k) * 100 << "%\n";
    crossValidationTry(data, 10, k);

    double testAccuracyVal = calculateAccuracy(testData, root, k);
    std::cout << "Test Set Accuracy: " << std::setprecision(4) << testAccuracyVal * 100 << "%" << std::endl;

    return 0;
}