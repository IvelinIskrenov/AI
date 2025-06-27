#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <list>
#include <random>

struct QueensState
{
	int N;
	int* nQueens;
	int* queensPerRow;
	int* queensPerD1;
	int* queensPerD2;
	QueensState(int n);
};

int chooseRandom(const std::vector<int>& numbers)
{
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> distN(0, numbers.size() - 1);
	return numbers[distN(rng)];
}

void inputQ(QueensState& q)//fill the board
{
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> distN(0, q.N - 1);

	int startQueen = distN(rng); // STARTQUEEN is a ROW
	q.nQueens[0] = startQueen;
	q.queensPerRow[startQueen]++;
	q.queensPerD1[0 - startQueen + q.N - 1]++;
	q.queensPerD2[0 + startQueen]++;
	
	for (size_t col = 1; col < q.N; col++)
	{
		std::vector<int> minConf;
		int prevConflict = (q.queensPerD1[col - 0 + (q.N - 1)] + q.queensPerD2[col + 0] + q.queensPerRow[0]);//possible problem, here queens conflict its +3 // input the first element as minConflict
		minConf.push_back(0);
		int currentConflict;
		for (size_t row = 1; row < q.N; row++)
		{
			currentConflict = (q.queensPerD1[col - row + (q.N - 1)] + q.queensPerD2[col + row] + q.queensPerRow[row]);
			if (currentConflict < prevConflict)
			{
				minConf.clear();
				minConf.push_back(row);
				prevConflict = currentConflict;
			}
			else if(currentConflict == prevConflict)
			{
				minConf.push_back(row);
			}
		}
		int randomQueenChoose = chooseRandom(minConf);// randomQueenChoose is a ROW
		q.nQueens[col] = randomQueenChoose;
		q.queensPerRow[randomQueenChoose]++;
		q.queensPerD1[col - randomQueenChoose + (q.N - 1)]++;
		q.queensPerD2[col + randomQueenChoose]++;
	}
}


bool hasConflict(const QueensState& q)
{
	for (size_t col = 0; col < q.N; col++)
	{
		int conf = (q.queensPerD1[col - q.nQueens[col] + (q.N - 1)] + q.queensPerD2[col + q.nQueens[col]] + q.queensPerRow[q.nQueens[col]]);
		if (conf > 3)
		{
			return false;
		}
	}
	return true;
}


void nextStep(QueensState& q)
{
	//make next QueensState

	//1. find the all queens with max conflict 
	//2. random choose one 
	//3. move the queen to random min conflict postion
	std::vector<int> maxConf;
	maxConf.push_back(0);// push column 0
	int prevConflictMAX = (q.queensPerD1[0 - (q.N - 1) + (q.N - 1)] + q.queensPerD1[0 + (q.N - 1)] + q.queensPerRow[0]);
	for (size_t col = 1; col < q.N; col++) //loop all Queens
	{
		int currentConflict;
		currentConflict = (q.queensPerD1[col - q.nQueens[col] + (q.N - 1)] + q.queensPerD2[col + q.nQueens[col]] + q.queensPerRow[q.nQueens[col]]);//give us the current conflict
		if (currentConflict > prevConflictMAX)
		{
			maxConf.clear();
			maxConf.push_back(col);
			prevConflictMAX = currentConflict;
		}
		else if(currentConflict == prevConflictMAX)
		{
			maxConf.push_back(col);
		}
	}
	int randomQueenToMove = chooseRandom(maxConf);//we choosen the column
	std::vector<int> minConf;
	minConf.push_back(0);//we push the first positon of the column (randomQueenToMove)
	int prevConflictMIN = (q.queensPerD1[randomQueenToMove - 0 + (q.N - 1)] + q.queensPerD2[randomQueenToMove + 0] + q.queensPerRow[0]); // input the first element as minCoflict
	int currentConflict;
	for (size_t row = 1; row < q.N; row++)
	{
		currentConflict = (q.queensPerD1[randomQueenToMove - row + (q.N - 1)] + q.queensPerD2[randomQueenToMove + row] + q.queensPerRow[row]);
		if (currentConflict < prevConflictMIN)
		{
			minConf.clear();
			minConf.push_back(row);
			prevConflictMIN = currentConflict;
		}
		else if (currentConflict == prevConflictMIN)
		{
			minConf.push_back(row);
		}
	}
	int minConfPostion = chooseRandom(minConf); // this is the row
	q.queensPerD1[randomQueenToMove - q.nQueens[randomQueenToMove] + (q.N - 1)]--;
	q.queensPerD2[randomQueenToMove + q.nQueens[randomQueenToMove]]--;
	q.queensPerRow[q.nQueens[randomQueenToMove]]--;
	
	q.nQueens[randomQueenToMove] = minConfPostion;//set the row of the column randomQueenToMove
	q.queensPerD1[randomQueenToMove - q.nQueens[randomQueenToMove] + (q.N - 1)]++;
	q.queensPerD2[randomQueenToMove + q.nQueens[randomQueenToMove]]++;
	q.queensPerRow[minConfPostion]++;
}

void print(QueensState q)
{
	for (size_t i = 0; i < q.N; i++)
	{
		std::cout << i << " " << q.nQueens[i] << std::endl;
	}
}

bool solveQueens(QueensState& queen)
{
	if (hasConflict(queen))
	{
		std::cout << "Solved\n\n";
		print(queen);
		return true;
	}
	nextStep(queen);// move to the next QueenState // rework update
	solveQueens(queen);
}


void testQueens(QueensState& q)
{
	for (size_t col = 0; col < q.N; col++)
	{
		int row;
		std::cout << "Enter row: ";
		std::cin >> row;

		q.nQueens[col] = row;
		q.queensPerRow[row]++;
		q.queensPerD1[col - row + (q.N - 1)]++;
		q.queensPerD2[col + row]++;
	}
}

int main()
{
	int n;
	std::cout << "Enter the N board: ";
	std::cin >> n;
	if (n > 3)
	{
		QueensState q(n);
		inputQ(q);
		//testQueens(q);
		solveQueens(q);
	}
	else
	{
		std::cout << "Can't be solved, N should be > 3!\n";
	}
	return 0;
}

QueensState::QueensState(int n)
{
	N = n;
	nQueens = new int[N];
	queensPerRow = new int[N];
	queensPerD1 = new int[N * 2 - 1];
	queensPerD2 = new int[N * 2 - 1];
	for (size_t i = 0; i < N; i++)
	{
		queensPerRow[i] = 0;
	}
	for (size_t i = 0; i < N * 2 - 1; i++)
	{
		queensPerD1[i] = 0;
		queensPerD2[i] = 0;
	}
}
