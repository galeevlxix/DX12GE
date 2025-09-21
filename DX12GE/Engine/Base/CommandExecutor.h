#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <string>
#include <sstream>
#include <vector>
#include <stack>

#include "../../Game/KatamariGame.h"

class CommandExecutor
{
private:
	struct ActionCommand
	{
		Object3DEntity* obj;
		std::string action;
		Vector3 value;
		Vector3 undo_value;
	};

	std::string path = "../../DX12GE/Resources/commands.json";
	KatamariGame* m_scene;

	void ProcessCommand(const std::string& line, std::string& output);

	void ProcessSet(const std::vector<std::string>& tokens, std::string& output);
	void ProcessSave(const std::vector<std::string>& tokens, std::string& output);
	void ProcessLoad(const std::vector<std::string>& tokens, std::string& output);
	void ProcessUndo(std::string& output);

	void ProcessObject(const std::vector<std::string>& tokens, std::string& output);
	bool ProcessObjectAction(ActionCommand& command, std::string& output);

	void GetObjectInfo(const std::vector<std::string>& tokens, std::string& output);
	
public:
	CommandExecutor(KatamariGame* scene);
	void Exit();
	void Update();
private:
	std::atomic<bool> stop_flag;
	std::thread reader_thread;
	std::mutex mutex;
	std::condition_variable cv;
	std::queue<std::string> lines;
	std::stack<ActionCommand> action_commands;

	void read_loop() 
	{
		std::string line;
		while (!stop_flag) 
		{
			std::getline(std::cin, line);
			{
				std::lock_guard<std::mutex> lock(mutex);
				lines.push(line);
			}
			cv.notify_one(); // Оповестить основной поток
		}
	}
	bool HasInput();
	std::string GetInput();
};