#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <cstdint>
// Initial values of
// Alpha and Beta
const int MAX = 1000;
const int MIN = -1000;

struct Board{
	char arr[3][3];
	int depth;

    Board& operator=(const Board& other)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                arr[i][j] = other.arr[i][j];
            }
        }

        //depth = other.depth;
        return *this;
    }
};



bool win(char playerWin, Board state)
{
    if (playerWin == 'X')
    {
        for (size_t i = 0; i < 3; i++)
        {
            if (state.arr[i][0] == 'X' && state.arr[i][1] == 'X' && state.arr[i][2] == 'X' ||
                state.arr[0][i] == 'X' && state.arr[1][i] == 'X' && state.arr[2][i] == 'X')
            {
                return true;
            }
        }
        if (state.arr[0][0] == 'X' && state.arr[1][1] == 'X' && state.arr[2][2] == 'X' ||
            state.arr[0][2] == 'X' && state.arr[1][1] == 'X' && state.arr[2][0] == 'X')
        {
            return true;
        }
    }
    else if (playerWin == 'O')
    {
        for (size_t i = 0; i < 3; i++)
        {
            if (state.arr[i][0] == 'O' && state.arr[i][1] == 'O' && state.arr[i][2] == 'O' ||
                state.arr[0][i] == 'O' && state.arr[1][i] == 'O' && state.arr[2][i] == 'O')
            {
                return true;
            }
        }
        if (state.arr[0][0] == 'O' && state.arr[1][1] == 'O' && state.arr[2][2] == 'O' ||
            state.arr[0][2] == 'O' && state.arr[1][1] == 'O' && state.arr[2][0] == 'O')
        {
            return true;
        }
    }
    return false;
}


void getAllNextStatesX(Board currentBoard, std::vector<Board> &succ)
{
    currentBoard.depth++;
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            if (currentBoard.arr[i][j] == '_')
            {
                currentBoard.arr[i][j] = 'X';
                succ.push_back(currentBoard);
                currentBoard.arr[i][j] = '_';
            }
        }
    }
}

void getAllNextStatesO(Board currentBoard, std::vector<Board>& succ)
{
    currentBoard.depth++;
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            if (currentBoard.arr[i][j] == '_')
            {
                currentBoard.arr[i][j] = 'O';
                succ.push_back(currentBoard);
                currentBoard.arr[i][j] = '_';
            }
        }
    }
}

//Alpha is the best value that the maximizer currently can guarantee at that level or above. 
//Beta is the best value that the minimizer currently can guarantee at that level or below.

int minimax(bool maximizingPlayer, Board state, int alpha, int beta, int sC, int &action)
{
    //Here we choose optimal decision 
    if (win('X', state))
    {
        return 10 - state.depth;
    }
    else if (win('O', state))
    {
        return state.depth - 10;
    }
    else if (state.depth == 9)
    {
        return 0;
    }

    if (maximizingPlayer)
    {
        int best = MIN;

        // Recur for left and 
        // right children
        std::vector<Board> successor;
        getAllNextStatesX(state, successor);
        int count = successor.size();
        for (int i = 0; i < count; i++)
        {
            int value = minimax(false, successor[i], alpha, beta, sC, action);
            if (value > best)
            {
                best = value;
                if (sC == count)
                {
                    action = i;  // Save the best action
                }
                
            }
            alpha = std::max(alpha, best);

            // Alpha Beta Pruning
            if (beta <= alpha)
                break;
        }
        return best;
    }
    else
    {
        int best = MAX;
        // Recur for left and
        // right children
        std::vector<Board> successor;
        getAllNextStatesO(state, successor);
        int count = successor.size();
        for (int i = 0; i < count; i++)
        {
            int value = minimax(true, successor[i], alpha, beta, sC, action);
            if (value < best)
            {
                best = value;
                if (sC == count)
                {
                    action = i;  // Save the best action
                }
            }
            beta = std::min(beta, best);

            // Alpha Beta Pruning
            if (beta <= alpha)
                break;
        }
        return best;
    }
}

void setBoard(Board& board)
{
    std::cout << "Enter board:" << std::endl;
    char pos;
    uint8_t d = 0;
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            std::cin >> pos;
            board.arr[i][j] = pos;
            if (pos != '_')
            {
                ++d;
            }
        }
    }
    board.depth = d;
}

void emptyBoard(Board& board)
{
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            board.arr[i][j] = '_';
        }
    }
    board.depth = 0;
}

void print(Board board)
{
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            std::cout << board.arr[i][j] << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void play()
{
    Board board;
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            board.arr[i][j] = '_';
        }
    }
    board.depth = 0;
    bool algFst = false;
    char choose;
    std::cout << "Do you want to be first? y/n \n";
    std::cin >> choose;
    if (choose == 'n')
    {
        algFst = true;
    }
    for (size_t i = 0; i < 9; i++)
    {
        if (algFst)
        {
            int action = MIN;
            std::vector<Board> successor;
            getAllNextStatesX(board, successor);
            minimax(true, board, MIN, MAX, successor.size(), action);
            board = successor[action];
            algFst = false;
            print(board);
        }
        else
        {
            int x;
            std::cout << "Choose position: ";
            std::cin >> x;
            board.arr[x / 3][x % 3] = 'O';
            algFst = true;
            print(board);
        }
        if (win('X', board))
        {
            std::cout << "X is the winner" << std::endl;
            break;
        }
        else if (win('O', board))
        {
            std::cout << "O is the winner" << std::endl;
            break;
        }
        else if (board.depth == 9)
        {
            std::cout << "Draw" << std::endl;
            break;
        }
        board.depth++;
    }
}

void testTurn()
{
    Board board;
    setBoard(board);
    Board bestMove;
    emptyBoard(bestMove);

    int action = MIN;
    std::vector<Board> successor;
    getAllNextStatesX(board, successor);
    minimax(true, board, MIN, MAX, successor.size(), action);// maximizing player is X
    print(successor[action]);
}

// Driver Code
int main()
{
    play();
    //testTurn();

    //1,2,6
    //2,6
    std::cout << "Ready";
    return 0;
}
