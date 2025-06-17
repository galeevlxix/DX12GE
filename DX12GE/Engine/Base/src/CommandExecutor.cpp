#include "../CommandExecutor.h"

CommandExecutor::CommandExecutor(KatamariGame* scene)
{
	m_scene = scene;
	reader_thread = std::thread(&CommandExecutor::read_loop, this);
}

bool CommandExecutor::HasInput()
{
	std::lock_guard<std::mutex> lock(mutex);
	return !lines.empty();
}

std::string CommandExecutor::GetInput()
{
	std::lock_guard<std::mutex> lock(mutex);
	if (lines.empty()) return "";
	std::string line = lines.front();
	lines.pop();
	return line;
}

void CommandExecutor::Exit()
{
	stop_flag = true;
	if (reader_thread.joinable()) {
		reader_thread.join();
	}
}

void CommandExecutor::Update()
{
	std::string input = GetInput();
	if (!input.empty()) 
	{
		std::string output;
		ProcessCommand(input, output);
		std::cout << output << std::endl;
	}
}

void CommandExecutor::ProcessCommand(const std::string& line, std::string& output)
{
	std::istringstream iss(line);
	std::vector<std::string> tokens;
	std::string token;

	while (iss >> token)
	{
		tokens.push_back(token);
	}

	if (tokens[0] == "set")
		ProcessSet(tokens, output);
	else if (tokens[0] == "save")
		ProcessSave(tokens, output);
	else
		output = "Ошибка: неизвестная команда " + tokens[0];
}

void CommandExecutor::ProcessSet(const std::vector<std::string>& tokens, std::string& output)
{
	if (tokens.size() <= 2)
	{
		output = "Ошибка: Недостаточно аргументов";
		return;
	}

	if (tokens[1] == "object")
		ProcessObject(tokens, output);
	else
		output = "Ошибка: неизвестная команда" + tokens[1];
}

void CommandExecutor::ProcessObject(const std::vector<std::string>& tokens, std::string& output)
{
	if (tokens.size() < 7)
	{
		output = "Ошибка: Недостаточно аргументов";
		return;
	}

	std::string name = tokens[2];
	Object3DEntity* obj = m_scene->Get(name);
	if (!obj)
	{
		output = "Ошибка: Объект по имени " + name + " не найден";
		return;
	}

	std::string action = tokens[3];
	Vector3 value;
	try
	{
		double x = std::stof(tokens[4]);
		double y = std::stof(tokens[5]);
		double z = std::stof(tokens[6]);
		value = Vector3(x, y, z);
	}
	catch (exception exc)
	{
		output = "Ошибка: координаты некорректны";
		return;
	}	

	if (action == "pos" || action == "position")
	{
		obj->Transform.SetPosition(value);

	}
	else if (action == "rot" || action == "rotation")
	{
		obj->Transform.SetRotationDegrees(value);
	}
	else if (action == "scl" || action == "scale")
	{
		obj->Transform.SetScale(value);
	}
	else if (action == "move")
	{
		obj->Transform.Move(value);
	}
	else if (action == "rotate")
	{
		obj->Transform.RotateDegrees(value);
	}
	else if (action == "expand")
	{
		obj->Transform.Expand(value.x, value.y, value.z);
	}
	else
	{
		output = "Ошибка: Неизвестное действие " + action;
		return;
	}

	output = "Объект изменен";
}

void CommandExecutor::ProcessGet(const std::vector<std::string>& tokens, std::string& output)
{
	if (tokens.size() <= 2)
	{
		output = "Ошибка: Недостаточно аргументов";
		return;
	}
}

void CommandExecutor::ProcessSave(const std::vector<std::string>& tokens, std::string& output)
{
	m_scene->Save();
	output = "Сцена сохранена";
}

void CommandExecutor::ProcessLoad(const std::vector<std::string>& tokens, std::string& output)
{
	
}



//<set|get|save|load>
//<obj_position|obj_rotation|obj_scale|obj_move|obj_rotate|obj_expand> <name_obj> <x> <y> <z>
