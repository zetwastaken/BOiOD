#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>

using namespace std;

const int nodeN = 40;
vector<pair<int, int>> node;       // Data of cities
vector<int> saPath;                // Current solution
vector<double> saDataDist;         // Data for Dist graph
vector<pair<double, bool>> saDataPraw; // Data for Praw graph
vector<double> saDataTemp;         // Data for Temp graph
double saBestDist;                 // Best solution (Dist)
vector<int> saBestPath;            // Best solution (path)
double saTempStart = 10000;        // Starting temperature
double saTempAlpha = 0.976;        // Temperature reduction coefficient
double saTemp;                     // Current temperature

int loopN = 0;                     // Steps remaining
int loopDelay = 10;                // Loop delay (ms)
const int graphWidth = 80;
const int graphHeight = 20;

const string GREEN = "\033[32m";
const string RESET = "\033[0m";

// Generate a random integer between min and max
int RandInt(int min, int max) {
    static random_device rd;
    static mt19937 gen(111);
    uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

// Generate a random path
vector<int> RandPath() {
    vector<int> path(node.size());
    iota(path.begin(), path.end(), 0);
    shuffle(path.begin(), path.end(), mt19937{random_device{}()});
    return path;
}

// Calculate the distance of the path
double PathDist(const vector<int>& path) {
    double dist = 0;
    for (size_t i = 0; i < path.size(); i++) {
        int n0 = path[i];
        int n1 = path[(i + 1) % path.size()];
        int x0 = node[n0].first;
        int y0 = node[n0].second;
        int x1 = node[n1].first;
        int y1 = node[n1].second;
        dist += round(sqrt((x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1)));
    }
    return dist;
}

// Make a move on the path
void PathMove(vector<int>& path, const pair<int, int>& move) {
    int a = move.first, b = move.second;
    while (a < b) {
        swap(path[a], path[b]);
        a++;
        b--;
    }
}

// Generate a random pair of indices
pair<int, int> RandAB() {
    int a = RandInt(0, node.size() - 1);
    int b = RandInt(0, node.size() - 2);
    if (b >= a) b++;
    if (a > b) swap(a, b);
    return {a, b};
}

// Draw the current path
void DrawPath(const vector<int>& path) {
    vector<vector<char>> canvas(graphHeight, vector<char>(graphWidth, ' '));

    for (const auto& p : node) {
        int x = p.first * graphWidth / 400;
        int y = p.second * graphHeight / 200;
        canvas[y][x] = 'O';
    }

    // Draw the path with asterisks
    for (size_t i = 0; i < path.size(); i++) {
        int n0 = path[i];
        int n1 = path[(i + 1) % path.size()];
        int x0 = node[n0].first * graphWidth / 400;
        int y0 = node[n0].second * graphHeight / 200;
        int x1 = node[n1].first * graphWidth / 400;
        int y1 = node[n1].second * graphHeight / 200;

        // Draw a simple line between the nodes
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;

        while (true) {
            if (x0 >= 0 && x0 < graphWidth && y0 >= 0 && y0 < graphHeight) {
                if (canvas[y0][x0] != 'O') {
                    canvas[y0][x0] = '*'; // Draw asterisks only if the cell is not a node
                }
            }
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
    }

    // Output the canvas
    ostringstream oss;
    for (const auto& row : canvas) {
        for (const auto& cell : row) {
            if (cell == 'O') {
                oss << GREEN << cell << RESET;
            } else {
                oss << cell;
            }
        }
        oss << '\n';
    }
    cout << oss.str();
}


// Simulated Annealing step
void SaStep(vector<int>& path, double temp) {
    double c0 = PathDist(path);
    double praw = 0;
    bool akce = false;
    for (int i = 0; i < 100; i++) {
        auto move = RandAB();
        PathMove(path, move);
        double c1 = PathDist(path);
        if (c1 > c0) {
            praw = exp((c0 - c1) / temp);
            akce = (RandInt(0, 10000) / 10000.0 < praw);
        }
        if (c1 <= c0 || akce) {
            c0 = c1; // Accept the move
        } else {
            PathMove(path, move); // Reject the move
        }
        if (saBestDist > c0) {
            saBestDist = c0;
            saBestPath = path;
        }
    }
    saDataDist.push_back(c0);
    saDataTemp.push_back(saTemp);
    saDataPraw.push_back({praw, akce});

    // Output current temperature, distance, and ASCII graphics
    cout << "Iteration: " << 400 - loopN << ", Temperature: " << temp << ", Path Distance: " << c0 << endl;
    DrawPath(path);
    if (400 - loopN == 1)
    {
        int temppp;
        cin >> temppp;
    }
}

// Initialize nodes with random coordinates
void NodeRand() {
    loopN = 0;
    node.clear();
    saPath.clear();
    saBestPath.clear();
    saDataDist.clear();
    saDataPraw.clear();
    saDataTemp.clear();
    for (int i = 0; i < nodeN; i++) {
        node.emplace_back(RandInt(0, 400), RandInt(0, 200));
    }
}

// Main loop of the program
void Loop() {
    if (loopN == 0) return;
    loopN--;
    SaStep(saPath, saTemp);
    saTemp *= saTempAlpha;
    if (loopN == 0) {
        saDataDist.push_back(saBestDist);
        saPath = saBestPath;
    }
    this_thread::sleep_for(chrono::milliseconds(loopDelay));
    Loop();
}

void Run() {
    auto startPath = RandPath();
    double startDist = PathDist(startPath);
    saPath = startPath;
    saDataDist.clear();
    saDataDist.push_back(startDist);
    saBestDist = startDist;
    saBestPath = startPath;

    loopN = 400;
    saTemp = saTempStart;
    saDataTemp.clear();
    saDataPraw.clear();
    Loop();
}

void Init() {
    NodeRand();
}

int main() {
    Init();
    cout << "Simulated Annealing TSP\n";

    Run();

    cout << "Best path distance: " << saBestDist << "\n";
    cout << "Best path: ";
    for (const auto& city : saBestPath) {
        cout << city << " ";
    }
    cout << "\n";

    return 0;
}
