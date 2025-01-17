#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <cmath>
#include <numeric>
#include <algorithm>

using namespace std;


double calculateMedianOfVector(vector<double>& dataVector) {

    sort(dataVector.begin(), dataVector.end());

    size_t n = dataVector.size();
    if (n % 2 == 0) {

        return (dataVector[n / 2 - 1] + dataVector[n / 2]) / 2.0;
    }
        return dataVector[n / 2];
}

class Preparation {
public:
    static int getRandomNumber() {
        static random_device rd;
        static mt19937 gen(rd());
        static uniform_int_distribution<> dis(1, 100);
        return dis(gen);
    }

    static vector<vector<int>> GetMatrix(int dimension) {
        vector<vector<int>> matrix(dimension, vector<int>(dimension));

        for (auto& row : matrix) {
            for (auto& element : row) {
                element = getRandomNumber();
            }
        }
        return matrix;
    }

    static void printMatrix(vector<vector<int>>& matrix) {
        for (auto& row : matrix) {
            for (auto& element : row) {
                cout << element << " ";
            }
            cout << "\n";
        }
    }
};

void ComputationNonParallel(vector<vector<int>>& Matrix1, vector<vector<int>>& Matrix2, vector<vector<int>>& resultMatrix,
                             int mutiplierMatrix1, int mutiplierMatrix2, int start_row, int end_row) {
    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < Matrix1.size(); ++j) {
            resultMatrix[i][j] = mutiplierMatrix1 * Matrix1[i][j] + mutiplierMatrix2 * Matrix2[i][j];
        }
    }
}

void ComputationParallel(vector<vector<int>>& Matrix1, vector<vector<int>>& Matrix2, vector<vector<int>>& ResultMatrix,
                          int dimension, int mutiplierMatrix1, int mutiplierMatrix2, int amountThread) {
    vector<thread> vectorThreads;
    vectorThreads.reserve(amountThread);
    int rows_per_thread = dimension / amountThread;

    for (int t = 0; t < amountThread; ++t) {
        int start_row = t * rows_per_thread;
        int end_row = (t == amountThread - 1) ? dimension : (t + 1) * rows_per_thread;

        vectorThreads.push_back(thread(ComputationNonParallel, ref(Matrix1), ref(Matrix2), ref(ResultMatrix),
                                       mutiplierMatrix1, mutiplierMatrix2, start_row, end_row));
    }

    for (auto& th : vectorThreads) {
        th.join();
    }
}

int main() {
    vector<int> vectorDimention = {50, 100, 500, 1000, 10000};
    const int TimesToRepeat = 100;
    vector<double> durationTime;

    string fileName = "TimeExecution.txt";
    ofstream outFile(fileName, ios::app);

    cout<< "Non-Parallel Thread ";
    // Non-Parallel Calculation
    outFile << "Non-Parallel\n";
    for (int& dimension : vectorDimention) {
        vector<double> averageTimePerDimension;

        for (int i = 0; i < TimesToRepeat; ++i) {
            cout<< i<<endl;
            vector<vector<int>> A = Preparation::GetMatrix(dimension);
            vector<vector<int>> B = Preparation::GetMatrix(dimension);
            vector<vector<int>> C(dimension, vector<int>(dimension));
            int k1 = Preparation::getRandomNumber(), k2 = Preparation::getRandomNumber();

            auto startTime = chrono::high_resolution_clock::now();
            ComputationNonParallel(A, B, C, k1, k2, 0, dimension);
            auto endTime = chrono::high_resolution_clock::now();
            auto duration = chrono::duration<double>(endTime - startTime).count();

            averageTimePerDimension.push_back(duration);
        }
        durationTime.push_back(calculateMedianOfVector(averageTimePerDimension));

    }

    for (int i = 0; i < vectorDimention.size(); i++) {
        outFile << "Thread: 1; Dimensions: " << vectorDimention[i] << "; Average Time in seconds: " << durationTime[i] << "\n";
        cout  << "Thread: 1; Dimensions: " << vectorDimention[i] << "; Average Time in seconds: " << durationTime[i] << "\n";
    }
cout<< "Parallel Thread ";
    // Parallel Calculation
    vector<int> amountOfThread = {4, 8, 16, 32, 64, 128};
    for (int numThread : amountOfThread) {
        outFile << "Parallel Thread: " << numThread << "\n";

        for (int& dimension : vectorDimention) {
            vector<double> averageTimeExePerDimension;

            for (int i = 0; i < TimesToRepeat; ++i) {
                cout<< i<<endl;
                vector<vector<int>> A = Preparation::GetMatrix(dimension);
                vector<vector<int>> B = Preparation::GetMatrix(dimension);
                vector<vector<int>> C(dimension, vector<int>(dimension));
                int k1 = Preparation::getRandomNumber(), k2 = Preparation::getRandomNumber();

                auto startTime = chrono::high_resolution_clock::now();
                ComputationParallel(A, B, C, dimension, k1, k2, numThread);
                auto endTime = chrono::high_resolution_clock::now();
                auto duration = chrono::duration<double>(endTime - startTime).count();

                averageTimeExePerDimension.push_back(duration);
            }

            outFile << "Dimensions: " << dimension << "; Average Time in seconds: "
                    << calculateMedianOfVector(averageTimeExePerDimension)<< "\n";
            cout << "Dimensions: " << dimension << "; Average Time in seconds: "
                    << calculateMedianOfVector(averageTimeExePerDimension)<< "\n";
        }
    }
    outFile.close();
}
