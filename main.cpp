// Block Matching Algorithm for Image Denoising
//

#include <iostream>
#include <vector>
#include <chrono>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

typedef vector<int> Table1;
typedef vector<Table1> Table2;
typedef vector<Table2> Table3;

Mat inputImage, outputImage;
Table3 initialCost, aggregatedCost;
int width, height, windowSize, levels;

int computeMatchingCost(int x, int y, int label);
int findBestAssignment(int x, int y);

int main()
{
	windowSize = 3;
	levels = 256;

	// Start timer
	auto start = chrono::steady_clock::now();

	// Read noisy image
	inputImage = imread("input.png", IMREAD_GRAYSCALE);

	// Get image size
	width = inputImage.cols;
	height = inputImage.rows;

	// Cache initial matching cost
	initialCost = Table3(height, Table2(width, Table1(levels)));
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			for (int i = 0; i < levels; i++)
				initialCost[y][x][i] = computeMatchingCost(x, y, i);

	// Compute aggregated matching cost
	aggregatedCost = Table3(height, Table2(width, Table1(levels, 0)));
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			for (int i = 0; i < levels; i++)
				for (int dy = -windowSize / 2; dy < windowSize / 2 + windowSize % 2; dy++)
					for (int dx = -windowSize / 2; dx < windowSize / 2 + windowSize % 2; dx++)
						aggregatedCost[y][x][i] += (y + dy >= 0 && y + dy < height && x + dx >= 0 && x + dx < width) ? initialCost[y+dy][x+dx][i] : 0;

	// Initialize denoised image
	outputImage = Mat::zeros(height, width, CV_8U);

	// Update denoised image
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			int label = findBestAssignment(x, y);
			outputImage.at<uchar>(y, x) = label;
		}

	// Show denoised image
	namedWindow("Denoised Image", WINDOW_NORMAL);
	imshow("Denoised Image", outputImage);
	waitKey(1);

	// Save denoised image
	bool flag = imwrite("output.png", outputImage);

	// Stop timer
	auto end = chrono::steady_clock::now();
	auto diff = end - start;
	cout << "\nRunning Time: " << chrono::duration<double, milli>(diff).count() << " ms" << endl;

	waitKey(0);

	return 0;
}

int computeMatchingCost(int x, int y, int label)
{
	int pixel = inputImage.at<uchar>(y, x);
	int cost = abs(pixel - label);

	return cost;
}

int findBestAssignment(int x, int y)
{
	int label, min = INT_MAX;
	for (int i = 0; i < levels; i++)
	{
		int cost = aggregatedCost[y][x][i];
		if (cost < min)
		{
			label = i;
			min = cost;
		}
	}

	return label;
}
