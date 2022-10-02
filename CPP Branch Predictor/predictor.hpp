#include <string>

namespace BranchPredictor {

	class Predictor {
		public:
			Predictor();
			virtual bool predict(const unsigned long long address, const std::string& behavior, const unsigned long long target) = 0;
			virtual ~Predictor() {};
			unsigned long long branches;
			unsigned long long correct;
	};

	class AlwaysTaken : public virtual Predictor {
		public:
			AlwaysTaken() : Predictor() {}
			bool predict(const unsigned long long address, const std::string& behavior, const unsigned long long target);
			~AlwaysTaken() {}
	};

	class AlwaysNonTaken : public virtual Predictor {
		public:
			AlwaysNonTaken() : Predictor() {}
			bool predict(const unsigned long long address, const std::string& behavior, const unsigned long long target);
			~AlwaysNonTaken() {}
	};

	class Bimodal : public virtual Predictor {
		private:
			unsigned int size;
			std::string* table;
		public:
			Bimodal(unsigned int size);
			bool predict(const unsigned long long address, const std::string& behavior, const unsigned long long target);
			~Bimodal() {delete[] table;}
	};

	class BimodalTwoBit : public virtual Predictor {
		private:
			unsigned int size;
			int* table;
		public:
			BimodalTwoBit(unsigned int size);
			bool predict(const unsigned long long address, const std::string& behavior, const unsigned long long target);
			~BimodalTwoBit() {delete[] table;}
	};

	class GShare : BimodalTwoBit, public virtual Predictor {
		private:
			unsigned int historyLength;
			unsigned int history = 0;
		public:
			GShare(unsigned int historyLength, unsigned int bmTableSize) : Predictor(), BimodalTwoBit(bmTableSize), historyLength(historyLength) {}
			bool predict(const unsigned long long address, const std::string& behavior, const unsigned long long target);
			~GShare() {}
	};

	class Tournament : public virtual Predictor {
		private:
			BimodalTwoBit bm;
			GShare gs;
			unsigned int size;
			int* table;
		public:
			Tournament(unsigned int tableSize, unsigned int bmTableSize) : size(tableSize), table(new int[tableSize]), bm(bmTableSize), gs(11, bmTableSize) {
				for(int i = 0; i < tableSize; i++) {
					table[i] = 0;
				}
			}
			bool predict(const unsigned long long address, const std::string& behavior, const unsigned long long target);
			~Tournament() { delete[] table; }
	};
	
	class BranchTargetBuffer : public virtual Predictor {
		private:
			Bimodal bm;
			unsigned int size;
			unsigned long long* table;
		public:
			BranchTargetBuffer(unsigned int tableSize, unsigned int bmTableSize) : size(tableSize), table(new unsigned long long[tableSize]), bm(bmTableSize) {
				for(int i = 0; i < tableSize; i++) {
					table[i] = 0;
				}
			}
			bool predict(const unsigned long long address, const std::string& behavior, const unsigned long long target);
			~BranchTargetBuffer() { delete[] table; }
	};
}
