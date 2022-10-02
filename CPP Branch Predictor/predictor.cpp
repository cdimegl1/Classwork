#include "predictor.hpp"

namespace BranchPredictor {

	Predictor::Predictor() {
		branches = 0;
		correct = 0;
	}

	bool AlwaysTaken::predict(const unsigned long long address, const std::string& behavior, const unsigned long long target) {
		branches++;
		if(behavior == "T") {
			correct++;
		}
		return true;
	}

	bool AlwaysNonTaken::predict(const unsigned long long address, const std::string& behavior, const unsigned long long target) {
		branches++;
		if(behavior == "NT") {
			correct++;
		}
		return false;
	}

	Bimodal::Bimodal(unsigned int size) : Predictor(), table(new std::string[size]), size(size) {
		for(int i = 0; i < size; i++) {
			table[i] = "T";
		}
	}

	bool Bimodal::predict(const unsigned long long address, const std::string& behavior, const unsigned long long target) {
		branches++;
		unsigned long long  bits = (size - 1) & address;
		bool retVal = table[bits] == "T";
		if(table[bits] == behavior) {
			correct++;
		} else {
			table[bits] = behavior;
		}
		return retVal;
	}

	BimodalTwoBit::BimodalTwoBit(unsigned int size) : Predictor(), table(new int[size]), size(size) {
		for(int i = 0; i < size; i++) {
			table[i] = 3;
		}
	}

	bool BimodalTwoBit::predict(const unsigned long long address, const std::string& behavior, const unsigned long long target) {
		branches++;
		unsigned long long bits = (size - 1) & address;
		bool retVal = false;
		if(table[bits] >= 2)
			retVal = true;
		if(behavior == "T") {
			if(table[bits] >= 2) {
				correct++;
			}
			table[bits] += 1;
		} else {
			if(table[bits] <= 1) {
				correct++;
			}
			table[bits] -= 1;
		}
		if(table[bits] < 0)
			table[bits] = 0;
		if(table[bits] > 3)
			table[bits] = 3;
		return retVal;
	}

	bool GShare::predict(const unsigned long long address, const std::string& behavior, const unsigned long long target) {
		bool retVal = BimodalTwoBit::predict(address ^ history, behavior, target);
		history = ((history << 1) | (behavior == "T" ? 1 : 0)) & (0xffffffffU >> (32 - historyLength));
		return retVal;
	}

	bool Tournament::predict(const unsigned long long address, const std::string& behavior, const unsigned long long target) {
		branches++;
		bool retVal = false;
		unsigned long long bits = (size - 1) & address;
		bool gBranch = gs.predict(address, behavior, target);
		bool bmBranch = bm.predict(address, behavior, target);

		if(table[bits] <= 1) {
			if(gBranch && behavior == "T") {
				correct++;
			} else if(!gBranch && behavior == "NT") {
				correct++;
			}
		} else {
			if(bmBranch && behavior == "T") {
				correct++;
			} else if(!bmBranch && behavior == "NT") {
				correct++;
			}
		}
		if(gBranch != bmBranch) {
			if(behavior == "T") {
				if(gBranch)
					table[bits]--;
				else
					table[bits]++;
			} else {
				if(gBranch)
					table[bits]++;
				else
					table[bits]--;
			}
		}
		if(table[bits] > 3) table[bits] = 3;
		if(table[bits] < 0) table[bits] = 0;
		return retVal;
	}


	bool BranchTargetBuffer::predict(const unsigned long long address, const std::string& behavior, const unsigned long long target) {

		bool retVal = false;
		unsigned long long bits = (size - 1) & address;	
		bool bmBranch = bm.predict(address, behavior, target);
		if(bmBranch) {
			branches++;
			if(target == table[bits]) {
				correct++;
				retVal = true;
			}
		}
		if(behavior == "T") {
			table[bits] = target;
		}
		return retVal;
	}
}
