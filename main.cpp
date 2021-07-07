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
#include <unordered_map>

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
    string YELLOW = "\033[33m"; //COLORED TEXT BABYY https://medium.com/@vitorcosta.matias/print-coloured-texts-in-console-a0db6f589138
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
                    p = colorPrefixes[(currTurn-1)%2] + curr + END; 
                else
                    (curr == "r") ? p = RED + "O" + END : p = YELLOW + "O" + END;
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

//replaces characters on the board with lines to indicate where the victory happened.
void strikeThrough(vector<pair<int, int>> &chain, int condition, int currTurn)
{
    string color, to;
    string escStart = "\033[" + color + "m";
    string escEnd = "\033[m";
    (currTurn%2 == 0) ? color = "31" : color = "33";

    //conditions:
    //0: l to r diagonal
    //1: r to l diagonal
    //2: horizontal
    //3: vertical
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
    
    for(int i=0; i<chain.size(); i++)
    {
        int tmp_x = chain[i].first;
        int tmp_y = chain[i].second;
        board[tmp_y][tmp_x] = to;
        cout << "(" << tmp_x << ", " << tmp_y << ")" << endl;
    }
}

pair<int, int> lru_diag = {1, -1};  //lru = left right up
pair<int, int> lrd_diag = {-1, 1};  //lrd = left right down
pair<int, int> rlu_diag = {-1, -1}; //rlu = right left up
pair<int, int> rld_diag = {1, 1};   //rld = right left down
pair<int, int> u_line = {0, 1};     //u = up (line)
pair<int, int> d_line = {0, -1};    //d = down (line)
pair<int, int> l_line = {-1, 0};    //l = left (line)
pair<int, int> r_line = {1, 0};     //r = right (line)
unordered_map<string, vector<pair<int, int>>> conditions({
    {"lr", {lru_diag, lrd_diag}},
    {"rl", {rlu_diag, rld_diag}},
    {"h", {l_line, r_line}},
    {"v", {u_line, d_line}},
});

unordered_map<string, int> conditionLink({
    {"lr", 0},
    {"rl", 1},
    {"h", 2},
    {"v", 3}
});

vector<string> indexMap({"lr", "rl", "h", "v"});

//initializes the stack, queue, chain vectors, etc. for the win check function.
void initdfs(stack<pair<pair<int, int>, string>> &que, int (&sums)[4], vector<vector<vector<int>>> &boolArr, vector<vector<pair<int, int>>> &chain, int latest) 
{
    int x = latest;
    int y = boardHeight-boardCapacity[latest]-1;
    string latPiece = board[y][x];

    for(int i=0; i<4; i++)
    {
        chain[i].push_back(make_pair(x, y));
        for(int j=0; j<2; j++)
        {
            boolArr[i][y][x] = 1;
            int tmp_x = x + conditions[indexMap[i]][j].first;
            int tmp_y = y + conditions[indexMap[i]][j].second;
            if(inRange(0, boardWidth-1, tmp_x) && inRange(0, boardHeight-1, tmp_y))
            if(board[tmp_y][tmp_x] == latPiece && boolArr[i][tmp_y][tmp_x] != 1)
            {
                que.push(make_pair(make_pair(tmp_x, tmp_y), indexMap[i]));
                sums[i]++;
            }
        }  
    }
}

bool hasWon(int latest)
{
    stack<pair<pair<int, int>, string>> queue; //queues coordinates and the condition to check for (l to r diagonal, horizontal line, etc.)
    vector<vector<vector<int>>> hasBeenTo = {4, vector<vector<int>>(boardHeight, vector<int>(boardWidth, 0))};
    vector<vector<pair<int, int>>> sequences = {4, vector<pair<int, int>>()}; //keeps track of current connected sequences
    int sumArr[4] = {1, 1, 1, 1};
    string lastCond = "";
    initdfs(queue, sumArr, hasBeenTo, sequences, latest);

    while(!queue.empty())
    {
        int x = queue.top().first.first;
        int y = queue.top().first.second;
        string piece = board[y][x];
        string cond = queue.top().second;
        sequences[conditionLink[cond]].push_back(make_pair(x, y));
        hasBeenTo[conditionLink[cond]][y][x] = 1;
        queue.pop();
        for(int i=0; i<2; i++)
        {
            int tmp_x = x + conditions[cond][i].first;
            int tmp_y = y + conditions[cond][i].second;
            if(inRange(0, boardWidth-1, tmp_x) && inRange(0, boardHeight-1, tmp_y))
                if(board[tmp_y][tmp_x] == piece && hasBeenTo[conditionLink[cond]][tmp_y][tmp_x] != 1)
                {
                    queue.push(make_pair(make_pair(tmp_x, tmp_y), cond));
                    sumArr[conditionLink[cond]]++;
                }
        }
        lastCond = cond;   
    }

    if(sumArr[conditionLink[lastCond]] >= 4)
    {
        strikeThrough(sequences[conditionLink[lastCond]], conditionLink[lastCond], currTurn);
        return true;
    }

    //check if all pieces have been played; if yes and the previous checks have returned false, then the game is a tie.
    if(currTurn != boardWidth*boardHeight - 1)
        return false;

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
            pos--;
        }

        board[boardHeight-boardCapacity[pos]-1][pos] = arrColors[currTurn%2];
        //auto start = chrono::high_resolution_clock::now(); //To record runtime of the win checking function
        gameEnded = hasWon(pos);
        //auto done = chrono::high_resolution_clock::now();
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