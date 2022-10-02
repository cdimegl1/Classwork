#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "predictor.hpp"

using namespace std;

int main(int argc, char* argv[]) {
	unsigned long long address;
	unsigned long long target;
	string behavior;

	vector<BranchPredictor::Predictor*> predictors;
	predictors.reserve(27);
	predictors.push_back(new BranchPredictor::AlwaysTaken);
	predictors.push_back(new BranchPredictor::AlwaysNonTaken);
	predictors.push_back(new BranchPredictor::Bimodal(16));
	predictors.push_back(new BranchPredictor::Bimodal(32));
	predictors.push_back(new BranchPredictor::Bimodal(128));
	predictors.push_back(new BranchPredictor::Bimodal(256));
	predictors.push_back(new BranchPredictor::Bimodal(512));
	predictors.push_back(new BranchPredictor::Bimodal(1024));
	predictors.push_back(new BranchPredictor::Bimodal(2048));
	predictors.push_back(new BranchPredictor::BimodalTwoBit(16));
	predictors.push_back(new BranchPredictor::BimodalTwoBit(32));
	predictors.push_back(new BranchPredictor::BimodalTwoBit(128));
	predictors.push_back(new BranchPredictor::BimodalTwoBit(256));
	predictors.push_back(new BranchPredictor::BimodalTwoBit(512));
	predictors.push_back(new BranchPredictor::BimodalTwoBit(1024));
	predictors.push_back(new BranchPredictor::BimodalTwoBit(2048));
	predictors.push_back(new BranchPredictor::GShare(3, 2048));
	predictors.push_back(new BranchPredictor::GShare(4, 2048));
	predictors.push_back(new BranchPredictor::GShare(5, 2048));
	predictors.push_back(new BranchPredictor::GShare(6, 2048));
	predictors.push_back(new BranchPredictor::GShare(7, 2048));
	predictors.push_back(new BranchPredictor::GShare(8, 2048));
	predictors.push_back(new BranchPredictor::GShare(9, 2048));
	predictors.push_back(new BranchPredictor::GShare(10, 2048));
	predictors.push_back(new BranchPredictor::GShare(11, 2048));
	predictors.push_back(new BranchPredictor::Tournament(2048, 2048));
	predictors.push_back(new BranchPredictor::BranchTargetBuffer(512, 512));
	ifstream inFile(argv[1]);
	while(inFile >> hex >> address >> behavior >> hex >> target) {
		for(BranchPredictor::Predictor*& predictor: predictors) {
			predictor->predict(address, behavior, target);
		}
	}
	inFile.close();
	
	ofstream outFile(argv[2]);
	BranchPredictor::Predictor* next = predictors[1];
	for(int i = 0; i < predictors.size(); i++) {
		outFile << predictors[i]->correct << "," << predictors[i]->branches << ";";
		if(i == predictors.size() - 1) {
			outFile << "\n";
			break;
		}
		if(typeid(*next) == typeid(*predictors[i])) {
			outFile << " ";
		} else {
			outFile << "\n";
		}
		next = predictors[i + 2];
	}
	outFile.close();

	for(BranchPredictor::Predictor*& predictor: predictors) {
		delete predictor;
	}
}
