#include "InputManager.h"

using namespace SolsticeGE;

std::unordered_set<int> InputManager::down_buttons;

InputManager::InputManager()
{
}

/// <summary>
/// Recieves a key press, adds key to set
/// </summary>
/// <param name="key"></param>
void InputManager::recieveBtnDown(const int& key)
{
	InputManager::down_buttons.emplace(key);
}

/// <summary>
/// Recieves a key release, removes key from set
/// </summary>
/// <param name="key"></param>
void InputManager::recieveBtnUp(const int& key)
{
	InputManager::down_buttons.erase(key);
}

bool InputManager::isBtnDown(const int& key)
{
	return InputManager::down_buttons.contains(key);
}

bool InputManager::isBtnUp(const int& key)
{
	return !InputManager::down_buttons.contains(key);
}
