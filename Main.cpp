#include <iostream>
#include <string>
#include <chrono>

#include "GraphColoring.h"
#include <fstream>

using namespace std;
using namespace szx;

#define LOG

void loadInput(istream& is, GraphColoring& gc) {
	is >> gc.nodeNum >> gc.edgeNum >> gc.colorNum;
	gc.edges.resize(gc.edgeNum);
	for (auto edge = gc.edges.begin(); edge != gc.edges.end(); ++edge) { is >> (*edge)[0] >> (*edge)[1]; }
}

void saveOutput(ostream& os, NodeColors& nodeColors) {
	for (auto color = nodeColors.begin(); color != nodeColors.end(); ++color) { os << *color << endl; }
}

void test(istream& inputStream, ostream& outputStream, long long secTimeout, int randSeed) {
	cerr << "load input." << endl;
	GraphColoring gc;
	loadInput(inputStream, gc);

	cerr << "solve." << endl;
	chrono::steady_clock::time_point endTime = chrono::steady_clock::now() + chrono::seconds(secTimeout);
	NodeColors nodeColors(gc.nodeNum);
	solveGraphColoring(nodeColors, gc, [&]() -> bool { return endTime < chrono::steady_clock::now(); }, randSeed);

	cerr << "save output." << endl;
	saveOutput(outputStream, nodeColors);
}
void test(istream& inputStream, ostream& outputStream, long long secTimeout) {
	return test(inputStream, outputStream, secTimeout, static_cast<int>(time(nullptr) + clock()));
}


int main(int argc, char* argv[]) {
#ifdef LOG
	freopen("err.log", "w", stderr);
#endif
	cerr << "load environment." << endl;
	if (argc > 2) {
		long long secTimeout = atoll(argv[1]);
		int randSeed = atoi(argv[2]);
		test(cin, cout, secTimeout, randSeed);
	} else {
		if (argc > 1)
		{
			// usage : ./mac [filename] [time]
			string filename = argv[1];	// 0500.9
			string loadfile = "./Instance/DSJC" + filename + ".txt";
			string savefile = "./Results/DSJC" + filename + ".txt";
			// printf("load from: "); cout << loadfile << endl;
			// printf("save to: "); cout << savefile << endl;
			ifstream ifs(loadfile);
			ofstream ofs(savefile);
			// long long secTimeout = atoll(argv[2]);
			// printf("limit time: %lld\n", secTimeout);
			test(ifs, ofs, 4000); // for self-test.
		}
		else {
			ifstream ifs("/Users/star/workspace/Smart/GraphColoringProblem/Instance/DSJC1000.9.txt");
			ofstream ofs("/Users/star/workspace/Smart/GraphColoringProblem/Results/DSJC1000.9.txt");
			test(ifs, ofs, 4000); // for self-test.
		}
	}
	return 0;
}

/*
clang++ -fcolor-diagnostics -fansi-escape-codes -O3 -g -std=c++11 Main.cpp GraphColoring.cpp -o gcp
g++ Main.cpp GraphColoring.cpp -O3 -static -o gcp
x86_64-w64-mingw32-g++ Main.cpp GraphColoring.cpp -O3 -static -o gcp
*/

/*
zip ./朱焰星-华农-计科.zip ./Main.cpp ./GraphColoring.h ./GraphColoring.cpp ./gcp.exe
*/