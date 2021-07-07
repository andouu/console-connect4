#include <iostream>
#include <cmath>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <stack>
#include <unordered_map>
#include <utility>
#include <chrono>
#include <iomanip>
#include <vector>

using namespace std;

#define endl '\n'
#define DEBUG(x) cout << ">DEBUG " << #x << ": " << (x) << endl;

const int boardWidth = 7;
const int boardHeight = 6;
const char arrColors[2] = {'r', 'y'}; //represents the player colors.
const string colors[2] = {"\033[31mRed\033[m", "\033[33mYellow\033[m"}; //same as arrColors, but I was too lazy to type ternaries so made this for printing convenience.
const string colorPrefixes[2] = {"\033[31m", "\033[33m"}; //what am I doing anymore

static string board[boardHeight][boardWidth];
static int boardCapacity[boardWidth];
static string winner = "";

int currTurn = 0; //even = red's turn, odd = yellow's turn

bool gameEnded = false;

void deepPrint(int board[boardHeight][boardWidth], int width, int height) //prints 2d array.
{
    cout << "[ ";
    for(int i=0; i<height; i++)
    {
        for(int j=0; j<width && i+j<width+height; j++)
        {
            cout << board[i][j] << ", ";
        }
        cout << endl;
    }
    cout << board[height-1][width-1] << " ]" << endl;
}

void printBoard(string board[boardHeight][boardWidth], int width, int height)
{
    string YELLOW = "\033[33m";
    string RED = "\033[31m";
    string END = "\033[m";
    cout << string(width*4+1, '-') << endl;
    
    for(int i=0; i<height; i++)
    {
        cout << "| ";
        for(int j=0; j<width; j++)
        {
            string p;
            string curr = board[i][j];
            if(curr != "")
            {
                if(curr != "r" && curr != "y")
                    p = colorPrefixes[(currTurn-1)%2] + curr + END; //COLORED TEXT BABYY https://medium.com/@vitorcosta.matias/print-coloured-texts-in-console-a0db6f589138
                else
                    (curr == "r") ? p = RED + "r" + END : p = YELLOW + "y" + END;
            }
            else
                p = ' ';
            cout << p << " | ";
        }
        cout << endl;
    }
    cout << string(width*4+1, '-') << endl;
    cout << "  ";
    for(int i=0; i<width; i++)
    {
        cout << i+1 << "   ";
    }
    cout << endl;
}

bool inRange(int lower, int upper, int x) //returns whether x is an element of [lower, upper].
{
    if(lower <= x && x <= upper)
        return true;
    return false;
}

bool columnIsFull(int track[], int pos, int max) //returns whether the a column on the board is full. Accepts the tracking vector and position (0-indexed) to check.
{
    if(track[pos] == max)
        return true;
    return false;
}

void resetdfs(int (&arr)[boardHeight][boardWidth], stack<pair<int, int>> queue, vector<pair<int, int>> &chain, int &currSum, int width, int height)
{
    currSum = 0;
    chain.clear();
    queue = stack<pair<int,int>>();
    memset(arr, 0, width * height * sizeof(int)); //resets array
}

//replaces characters on the board with lines to indicate where the victory happened.
void strikeThrough(vector<pair<int, int>> &chain, int condition, int currTurn)
{
    //conditions:
    //0: l to r diagonal
    //1: r to l diagonal
    //2: horizontal
    //3: vertical
    string color, to;
    string escStart = "\033[" + color + "m";
    string escEnd = "\033[m";
    (currTurn%2 == 0) ? color = "31" : color = "33";

    switch(condition)
    {
        case 0:
            to = "/";
            break;
        case 1:
            to = "\\";
            break;
        case 2:
            to = "-";
            break;
        case 3:
            to = "|";
            break;
    }
    
    while(!chain.empty())
    {
        int tmp_x = chain.back().first;
        int tmp_y = chain.back().second;
        board[tmp_y][tmp_x] = to;
        DEBUG(tmp_x);
        DEBUG(tmp_y);
        chain.pop_back();
    }
}

//TODO(?): rewrite hasWon to be smaller
bool hasWon(string board[boardHeight][boardWidth], int track[], int latest, int width, int height, int currTurn, string &winner) //returns whether the last position played won the game or not. Accepts currTurn
{                                                                                                                                //primarily to check if all possible moves have been played (tie check)                                                                                           
    int currSum = 0; //current sum of dfs                                                                     
    int x = latest;
    int y = height-track[latest]-1;
    int hasBeenTo[boardHeight][boardWidth] = {}; //array to keep track of checked nodes in dfs
    string lastPlayed = board[y][x];
    stack<pair<int, int>> queue; //stack for dfs to queue the next position to check
    vector<pair<int, int>> chain; //current chain of pieces
    pair<int, int> start{x, y};

    //dfs for all possible cases (diagonals, planes)
    //l to r diagonal:
    queue.push(start);
    chain.push_back(start);
    while(!queue.empty())
    {
        pair<int, int> coord = queue.top();
        int tmp_y = coord.second;
        int tmp_x = coord.first;
        hasBeenTo[tmp_y][tmp_x] = 1;
        currSum++;
        if(currSum >= 4)
        {
            strikeThrough(chain, 0, currTurn);
            return true;
        }
        queue.pop();
        
        int tr_x = tmp_x+1; //tl = top right
        int tr_y = tmp_y-1; 
        if(inRange(0, height-1, tr_y) && inRange(0, width-1, tr_x))
            if(board[tr_y][tr_x] == lastPlayed)
                if(hasBeenTo[tr_y][tr_x] != 1)
                {
                    queue.push(make_pair(tr_x, tr_y));
                    chain.push_back(make_pair(tr_x, tr_y));
                }

        int bl_x = tmp_x-1; //bl = bottom left
        int bl_y = tmp_y+1;
        if(inRange(0, height-1, bl_y) && inRange(0, width-1, bl_x))
            if(board[bl_y][bl_x] == lastPlayed)
                if(hasBeenTo[bl_y][bl_x] != 1)
                {
                    queue.push(make_pair(bl_x, bl_y));
                    chain.push_back(make_pair(bl_x, bl_y));
                }
    }
    
    resetdfs(hasBeenTo, queue, chain, currSum, boardWidth, boardHeight);

    //r to l diagonal:
    queue.push(start);
    chain.push_back(start);
    while(!queue.empty())
    {
        pair<int, int> coord = queue.top();
        int tmp_y = coord.second;
        int tmp_x = coord.first;
        hasBeenTo[tmp_y][tmp_x] = 1;
        currSum++;
        if(currSum >= 4)
        {
            strikeThrough(chain, 1, currTurn);
            return true;
        }
        queue.pop();
        
        int tl_x = tmp_x-1; //tl = top left
        int tl_y = tmp_y-1;
        if(inRange(0, height-1, tl_y) && inRange(0, width-1, tl_x))
            if(board[tl_y][tl_x] == lastPlayed)
                if(hasBeenTo[tl_y][tl_x] != 1)
                {
                    queue.push(make_pair(tl_x, tl_y));
                    chain.push_back(make_pair(tl_x, tl_y));
                }

        int br_x = tmp_x+1; //br = bottom right
        int br_y = tmp_y+1;
        if(inRange(0, height-1, br_y) && inRange(0, width-1, br_x))
            if(board[br_y][br_x] == lastPlayed)
                if(hasBeenTo[br_y][br_x] != 1)
                {
                    queue.push(make_pair(br_x, br_y));
                    chain.push_back(make_pair(br_x, br_y));
                }
    }

    resetdfs(hasBeenTo, queue, chain, currSum, boardWidth, boardHeight);

    //horizontal line case:
    queue.push(start);
    chain.push_back(start);
    while(!queue.empty())
    {
        pair<int, int> coord = queue.top();
        int tmp_y = coord.second;
        int tmp_x = coord.first;
        hasBeenTo[tmp_y][tmp_x] = 1;
        currSum++;
        if(currSum >= 4)
        {
            strikeThrough(chain, 2, currTurn);
            return true;
        }
        queue.pop();
        
        int l = tmp_x-1; //l = left
        int r = tmp_x+1; //r = right
        if(inRange(0, width-1, l))
            if(board[tmp_y][l] == lastPlayed)
                if(hasBeenTo[tmp_y][l] != 1)
                {
                    queue.push(make_pair(l, tmp_y));
                    chain.push_back(make_pair(l, tmp_y));
                }

        if(inRange(0, width-1, r))
            if(board[tmp_y][r] == lastPlayed)
                if(hasBeenTo[tmp_y][r] != 1)
                {
                    queue.push(make_pair(r, tmp_y));
                    chain.push_back(make_pair(r, tmp_y));
                }
    }

    resetdfs(hasBeenTo, queue, chain, currSum, boardWidth, boardHeight);

    //vertical line case:
    queue.push(start);
    chain.push_back(start);
    while(!queue.empty())
    {
        pair<int, int> coord = queue.top();
        int tmp_y = coord.second;
        int tmp_x = coord.first;
        hasBeenTo[tmp_y][tmp_x] = 1;
        currSum++;
        if(currSum >= 4)
        {
            strikeThrough(chain, 3, currTurn);
            return true;
        }
        queue.pop();
        
        int u = tmp_y+1; //u = up
        int d = tmp_y-1; //d = down
        if(inRange(0, height-1, u))
            if(board[u][tmp_x] == lastPlayed)
                if(hasBeenTo[u][tmp_x] != 1)
                {
                    queue.push(make_pair(tmp_x, u));
                    chain.push_back(make_pair(tmp_x, u));
                }

        if(inRange(0, height-1, d))
            if(board[d][tmp_x] == lastPlayed)
                if(hasBeenTo[d][tmp_x] != 1)
                {
                    queue.push(make_pair(tmp_x, d));
                    chain.push_back(make_pair(tmp_x, d));
                }
    }

    for(int i=0; i<height; i++)
    {
        for(int j=0; j<width; j++)
        {
            if(board[i][j] == "")
                return false;
        }
    }

    winner = "tie";
    return true;
}

void refreshConsole()
{
    cout << string(5, '\n');
}

int main()
{
    while(!gameEnded)
    {
        string player;
        player = colors[currTurn%2];

        refreshConsole();
        cout << "Current board (may be empty):\n" << endl;
        printBoard(board, boardWidth, boardHeight);
        cout << "It's " << player << "'s turn. Enter a position on the board to play: ";
        
        int pos;
        cin >> pos;
        pos--; //align with 0-indexing

        while(!inRange(0, boardWidth-1, pos) || cin.fail() || columnIsFull(boardCapacity, pos, boardHeight)) //range of [0, boardWidth-1] to compensate for 0-indexing.
        {
            if(inRange(0, boardWidth-1, pos) && columnIsFull(boardCapacity, pos, boardHeight))
                cout << "That column is filled! Pick another column. ";
            else
                cout << "Invalid input. " << "It's " << player << "'s turn. Enter a position from 1-7 to play the turn! ";
                
            cin.clear();
            cin.ignore(256, endl);
            cin >> pos;
        }
        board[boardHeight-boardCapacity[pos]-1][pos] = arrColors[currTurn%2];
        auto start = chrono::high_resolution_clock::now();
        gameEnded = hasWon(board, boardCapacity, pos, boardWidth, boardHeight, currTurn, winner);
        auto done = chrono::high_resolution_clock::now();
        //cout << "Win check runtime(ms): " << setprecision(20) << (double)chrono::duration_cast<chrono::milliseconds>(done-start).count() << endl; 
        boardCapacity[pos]++, currTurn++;
    }
    if(winner != "tie")
        winner = colors[(currTurn-1)%2];

    refreshConsole();
    printBoard(board, boardWidth, boardHeight);
    if(winner != "tie")
        cout << "\n" << winner << " has won the game!";
    else
        cout << "\n" << "The game ends in a tie... How did yall do that?";
    return 0;
}