#pragma once
#include <Engine/Core/Common.h>

namespace Spikey {

	class IndexQueue {
	public:
		IndexQueue() : NextIndex(0) {}

		uint32 Grab() {
			uint32 index = NextIndex;

			if (Queue.size() > 0) {
				index = Queue.back();
				Queue.pop_back();
			}
			else {
				NextIndex++;
			}
			return index;
		}

		void Release(uint32 index) { Queue.push_back(index); }

	private:
		uint32 NextIndex;
		std::vector<uint32> Queue;
	};

	template<typename T>
	void SwapDelete(std::vector<T>& v, uint32 idx) {
		if (v.size() > 0) {
			v[idx] = std::move(v.back());
		}
		v.pop_back();
	}
}