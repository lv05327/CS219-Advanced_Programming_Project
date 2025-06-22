#include <iostream>
#include <string>
#include <chrono>
#include "Mat.h"
#include <iomanip>
#include <omp.h>

using namespace std;
using namespace std::chrono;

int main()
{
    omp_set_num_threads(12);
    string filename;
    string command;

    cout << "Enter image filename: ";
    cin >> filename;
    filename = "../" + filename;
    cout << "Enter command (adjust | resize | blend): ";
    cin >> command;

    auto start_1 = high_resolution_clock::now();
    Mat img = imread(filename);
    auto end_1 = high_resolution_clock::now();
    auto duration_1 = duration<double, std::milli>(end_1 - start_1).count();
    cout << fixed << setprecision(3);
    cout << "Operation read completed in " << duration_1 << " ms.\n";

    Mat result(1, 1, 8);
    Mat img2 = img.clone();
    if (img.getData() == nullptr)
    {
        cerr << "Failed to load image: " << filename << endl;
        return 1;
    }

    auto start = high_resolution_clock::now();

    if (command == "adjust")
    {
        result = img.adjustBrightnessContrast(40, 1.5f);
    }
    else if (command == "resize")
    {
        result = img.resize(img.getWidth() * 2, img.getHeight() * 2);
    }
    else if (command == "blend")
    {
        result = blend(img, img2);
    }
    else
    {
        cerr << "Unknown command: " << command << endl;
        return 1;
    }

    auto end = high_resolution_clock::now();
    auto duration_3 = duration<double, std::milli>(end - start).count();
    cout << fixed << setprecision(3);
    cout << "Operation [" << command << "] completed in " << duration_3 << " ms.\n";

    auto start_2 = high_resolution_clock::now();
    imwrite("output.bmp", result);
    auto end_2 = high_resolution_clock::now();
    auto duration_2 = duration<double, std::milli>(end_2 - start_2).count();
    cout << fixed << setprecision(3);
    cout << "Operation write completed in " << duration_2 << " ms.\n";
    
    filename = "../garbage.bmp";
    result = imread(filename);
    Mat garbage = result.clone();
    imwrite("../garbage_out.bmp",garbage);
    return 0;
}
