#include <iostream>
#include <ctime>
#include <queue>
#include <vector>
#include <random>
#include <stack>
#include <cmath>
#include <cstdio>
#include <stdio.h>
#include <iomanip>
//#include <stdc++.h>
#include <chrono>

const int MIN = 100000;

enum Direction {
    UP,
    DOWN,
    RIGHT,
    LEFT,
    None
};

//rework to size_t
class PuzzleState {
private:
    int** board;
    int zeroPos;
    int currentZeroPos;
    int g;
    int distance; // mahatan
    int size; // all size
    int N;// N x N
    std::stack<Direction> path;

    void free();

public:
    //PuzzleState();
    PuzzleState(int zeroP, int size);
    PuzzleState(const PuzzleState& other);
    PuzzleState& operator=(const PuzzleState& other);
    ~PuzzleState();

    //set the currecnt dist
    void setDist();
    int getCurrentZeroPos() { return currentZeroPos; };
    int getN() { return N; };
    int getG() { return g; };
    int getDistance() { return distance; };
    //manhatan 
    int manhatan();

    //checking if the puzzel is solvable 
    bool solvable();
    void printPath();
    bool isSolved();
    void getSuccessors(std::vector<PuzzleState>& succ);
    int search(int mainBound, PuzzleState currentBoard, bool &solved);
    bool idaStar();
};

std::ostream& operator<<(std::ostream& os, const Direction& dir);

int main() {
    int zeroPostion;
    int size;
    std::cout << "Enter values: ";
    std::cin >> size >> zeroPostion;
    PuzzleState myPuzz(zeroPostion, size);
    auto start = std::chrono::high_resolution_clock::now();
    myPuzz.idaStar();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    std::cout << std::endl << std::setprecision(6) << "Time : " << duration << std::endl;

    /*if (_CrtDumpMemoryLeaks())
    {
        std::cout << "Memory leak";
    }*/
    return 0;
}

void PuzzleState::free()
{
    for (int i = 0; i < N; ++i) {
        delete[] board[i];
    }
    delete[] board;
}

PuzzleState::PuzzleState(int zeroP, int _size)
{
    ++_size;
    size = _size;
    N = sqrt(_size);
    g = 0;
    if (zeroP == -1)
    {
        zeroPos = size - 1;// added 1
    }
    else
    {
        zeroPos = zeroP;
    }
    board = new int* [N];
    int currecntNumber;
    for (size_t i = 0; i < N; i++)
    {
        board[i] = new int[N];
    }
    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < N; j++)
        {
            std::cin >> currecntNumber;
            board[i][j] = currecntNumber;
            if (currecntNumber == 0)
            {
                currentZeroPos = i * N + j;
            }
        }
    }
    /*if (!solvable())
    {
        std::cout << "Not solvable!\n"; return;
    }*/
    setDist();
}

PuzzleState::PuzzleState(const PuzzleState& other)
{
    zeroPos = other.zeroPos;
    currentZeroPos = other.currentZeroPos;
    g = other.g;
    distance = other.distance;
    size = other.size;
    N = other.N;

    // Deep copy the board
    board = new int* [N];  
    for (int i = 0; i < N; ++i) {
        board[i] = new int[N];  
        for (int j = 0; j < N; ++j) {
            board[i][j] = other.board[i][j];  
        }
    }
    path = other.path;
}

PuzzleState& PuzzleState::operator=(const PuzzleState& other)
{
    if (this == &other) {
        return *this;  // Return the current object if they are the same
    }
    free();
    board = new int* [N];
    for (int i = 0; i < N; ++i) 
    {
        board[i] = new int[N];
    }


    zeroPos = other.zeroPos;
    size = other.size;
    N = other.N;
    g = other.g;
    distance = other.distance;
    currentZeroPos = other.currentZeroPos;

    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < N; j++)
        {
            board[i][j] = other.board[i][j];
        }
    }
    path = other.path;
    return *this;
}

PuzzleState::~PuzzleState()
{
    free();
}

void PuzzleState::setDist()
{
    distance = manhatan();
}

int PuzzleState::manhatan()
{
    int dist = 0;
    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col)
        {
            int ourNumber = board[row][col]; //the number on this position 
            if (zeroPos >= ourNumber && ourNumber != 0)
            {
                int targetX = (board[row][col] - 1) / N;
                int targetY = (board[row][col] - 1) % N;
                dist += abs(targetX - row) + abs(targetY - col);
            }
            else if (zeroPos < ourNumber && ourNumber != 0)
            {
                //++ourNumber;
                int targetX = ourNumber / N;
                int targetY = (ourNumber) % N;
                dist += abs(targetX - row) + abs(targetY - col);
            }
        }
    }
    return dist;
}

bool PuzzleState::solvable()
{
    int* arr = new int[size];
    int inversions = 0;
    int count = 0;
    int zPos;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            arr[count] = board[i][j];
            if (board[i][j] == 0)
            {
                zPos = i * N + j;
            }
            ++count;
        }
    }

    for (size_t i = 0; i < count - 1; i++)
    {
        for (size_t j = i + 1; j < count; j++)
        {
            if (arr[i] != 0 && arr[j] != 0 && arr[i] > arr[j])
            {
                ++inversions;
                continue;
            }
        }
    }
    /*inversions += std::abs(zeroPos - zPos);*/
    int zero_row = zPos / N;
    if ((N % 2 == 1 && inversions % 2 == 0) ||
        (N % 2 == 0 && (inversions + zero_row) % 2 == 1))
    {
        std::cout << "Solvable\n";
        delete[] arr;
        return true;
    }
    std::cout << "Not solvable\n";
    delete[] arr;
    return false;
}

void PuzzleState::printPath()
{
    std::cout << "Path: ";
    while (!path.empty())
    {
        std::cout << path.top() << " ";
        path.pop();
    }
    std::cout << "\n";
}

bool PuzzleState::isSolved()
{
    return distance == 0;
}

void PuzzleState::getSuccessors(std::vector<PuzzleState>& succ)
{
    PuzzleState copyState(*this);

    int row = currentZeroPos / N;
    int col = currentZeroPos - row * N;
    Direction lastDirection = None;
    if (!path.empty())
    {
        lastDirection = path.top();
    }

    if (row < N - 1 && lastDirection != UP)
    {
        int prevDist = distance;
        currentZeroPos = currentZeroPos + N;
        path.push(DOWN);
        ++g;
        board[row][col] = board[row + 1][col];
        board[row + 1][col] = 0;
        setDist();
        copyState = *this;
        succ.push_back(copyState);
        distance = prevDist;
        board[row + 1][col] = board[row][col];
        board[row][col] = 0;
        --g;
        path.pop();
        currentZeroPos = currentZeroPos - N;
    }
    
    if(row > 0 && lastDirection != DOWN)
    {
        int prevDist = distance;
        currentZeroPos = currentZeroPos - N;
        path.push(UP);
        ++g;
        board[row][col] = board[row - 1][col];
        board[row - 1][col] = 0;
        setDist();
        copyState = *this;
        succ.push_back(copyState);
        distance = prevDist;
        board[row - 1][col] = board[row][col];
        board[row][col] = 0;
        --g;
        path.pop();
        currentZeroPos = currentZeroPos + N;
    }
    
    if (col > 0 && lastDirection != RIGHT)
    {
        int prevDist = distance;
        currentZeroPos = currentZeroPos - 1;
        path.push(LEFT);
        ++g;
        board[row][col] = board[row][col - 1];
        board[row][col - 1] = 0;
        setDist();
        copyState = *this;
        succ.push_back(copyState);
        distance = prevDist;
        board[row][col - 1] = board[row][col];
        board[row][col] = 0;
        --g;
        path.pop();
        currentZeroPos = currentZeroPos + 1;
    }
    
    if (col < N - 1 && lastDirection != LEFT)
    {
        int prevDist = distance;
        currentZeroPos = currentZeroPos + 1;
        path.push(RIGHT);
        ++g;
        board[row][col] = board[row][col + 1];
        board[row][col + 1] = 0;
        setDist();
        copyState = *this;
        succ.push_back(copyState);
        distance = prevDist;
        board[row][col + 1] = board[row][col];
        board[row][col] = 0;
        --g;
        path.pop();
        currentZeroPos = currentZeroPos - 1;
    }
}

int PuzzleState::search(int mainBound, PuzzleState currentBoard, bool &solved)
{
    int f = currentBoard.getG() + currentBoard.getDistance();
    if (mainBound < f)
    {
        return f;
    }
    if (currentBoard.isSolved())
    {
        std::cout << "Turns: " << currentBoard.path.size() << " ";
        currentBoard.printPath();
        solved = true;
    }
    int minBound = MIN;
    std::vector<PuzzleState> successors;
    currentBoard.getSuccessors(successors);//LOSE DATA ADD ARGUMENT PUZZEL
    unsigned count = successors.size();
    for (size_t i = 0; i < count; i++)
    {
        int nB;
        nB = search(mainBound, successors[i], solved);
        if (nB < minBound)
        {
            minBound = nB;
        }
    }
    return minBound;
}

bool PuzzleState::idaStar()
{
    if (!solvable())
    {
        return false;
    }
    int bound = g + distance;
    int newBound;
    bool found = false;
    while (true)
    {
        newBound = search(bound, *this, found);
        if (found == true)
        {
            std::cout << "Founded!";
            return true;
        }
        bound = newBound;
    }
    return false;
}

std::ostream& operator<<(std::ostream& os, const Direction& dir) {
    switch (dir) {
    case UP:
        os << "Down";
        break;
    case DOWN:
        os << "Up";
        break;
    case LEFT:
        os << "Right";
        break;
    case RIGHT:
        os << "Left";
        break;
    }
    return os;
}