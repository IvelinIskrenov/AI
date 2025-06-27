#include <iostream>
#include <vector>
bool isGoalState(char* broad, char* zeroState, size_t size)
{
	unsigned i = 0;
	for (size_t i = 0; i < size; i++)
	{
		if (broad[i] != zeroState[i])
		{
			return false;
		}
	}
	return true;
}

void printResult(std::vector<char*> result, size_t size)
{
	for (size_t i = 0; i < result.size(); i++)
	{
		size_t j = 0;
		while (j < size)
		{
			std::cout << result[i][j];
			j++;
		}
		std::cout << std::endl;
	}
}

bool dfs(char* board, char* finalState, std::vector<char*> &path, size_t size, int emptyPos)
{
	char* copyBoard = new char[size];
	copyBoard[size] = '\0';
	for (size_t i = 0; i < size; i++)
	{
		copyBoard[i] = board[i];
	}
	path.push_back(copyBoard);

	if (isGoalState(board,finalState,size))
	{
		printResult(path,size);
		return true;
	}

	
	if (emptyPos - 2 >= 0 && board[emptyPos - 2] == '>')
	{
		std::swap(board[emptyPos], board[emptyPos - 2]);
		if (dfs(board, finalState, path, size, emptyPos - 2) == true)
		{
			return true;
		}
		std::swap(board[emptyPos], board[emptyPos - 2]);
		//path.pop_back();
	}
	if (emptyPos - 1 >= 0 && board[emptyPos - 1] == '>')
	{
		std::swap(board[emptyPos], board[emptyPos - 1]);
		if (dfs(board, finalState, path, size, emptyPos - 1) == true)
		{
			return true;
		}
		std::swap(board[emptyPos], board[emptyPos - 1]);
		//path.pop_back();
	}
	if (emptyPos + 1 < size && board[emptyPos + 1] == '<')
	{
		std::swap(board[emptyPos], board[emptyPos + 1]); 
		if (dfs(board, finalState, path, size, emptyPos + 1) == true)
		{
			return true;
		}	
		std::swap(board[emptyPos], board[emptyPos + 1]);
		//path.pop_back();
	}
	if (emptyPos + 2 < size && board[emptyPos + 2] == '<')
	{
		std::swap(board[emptyPos], board[emptyPos + 2]);
		if (dfs(board, finalState, path, size, emptyPos + 2) == true)
		{
			return true;
		}
		std::swap(board[emptyPos], board[emptyPos + 2]);
		//path.pop_back();
	}
	
	
	path.pop_back();
	return false;
}


int main()
{

	/*char* x = new char[3];
	x[0] = '>';
	x[1] = '_';
	x[2] = '<';

	std::swap(x[0], x[1]);
	std::cout << x[0] << " " << x[1] << " " << x[2];*/
	unsigned n;
	std::cout << "Enter n: ";
	std::cin >> n;
	if (n <= 0)
	{
		std::cerr << "N should be positive!";
		return 1;
	}
	unsigned size = (n * 2) + 1;
	char *broad = new char[size];
	char *finalState = new char[size];
	int emptyPos = size / 2;
	for (size_t i = 0; i < size; i++)
	{
		(i <= size / 2) ? broad[i] = '>' : broad[i] = '<';
	}
	broad[(size / 2)] = '_';
	
	
	for (size_t i = 0; i < size; i++)
	{
		(i <= size / 2) ? finalState[i] = '<' : finalState[i] = '>';
	}
	finalState[(size / 2)] = '_';
	std::vector<char*> result;
	dfs(broad, finalState,result,size,emptyPos);

	return 0;
}