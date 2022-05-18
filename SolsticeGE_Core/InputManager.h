#pragma once

#include <unordered_set>

namespace SolsticeGE {
	class InputManager
	{
	public:

		InputManager();

		static void recieveBtnDown(const int& key);
		static void recieveBtnUp(const int& key);

		static bool isBtnDown(const int& key);
		static bool isBtnUp(const int& key);

	private:
		static std::unordered_set<int> down_buttons;
	};
}

