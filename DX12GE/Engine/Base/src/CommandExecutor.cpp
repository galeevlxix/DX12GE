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
	if (reader_thread.joinable()) 
	{
		// TODO: при вызове join происходит дедлок
		/*reader_thread.join();*/
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
	if (tokens[0] == "get")
		GetObjectInfo(tokens, output);
	else if (tokens[0] == "save")
		ProcessSave(tokens, output);
	else if (tokens[0] == "undo")
		ProcessUndo(output);
	else
	{
		output = "Ошибка: неизвестная команда " + tokens[0];
	}
}

void CommandExecutor::ProcessSet(const std::vector<std::string>& tokens, std::string& output)
{
	if (tokens.size() <= 2)
	{
		output = "Ошибка Set: Недостаточно аргументов";
		return;
	}

	if (tokens[1] == "object")
		ProcessObject(tokens, output);
	else
		output = "Ошибка Set: неизвестная команда" + tokens[1];
}

void CommandExecutor::ProcessObject(const std::vector<std::string>& tokens, std::string& output)
{
	if (tokens.size() < 7)
	{
		output = "Ошибка Set Object: Недостаточно аргументов";
		return;
	}

	ActionCommand command;


	std::string name = tokens[2];
	command.obj = m_scene->Get(name);
	if (!command.obj)
	{
		output = "Ошибка Set Object: Объект по имени " + name + " не найден";
		return;
	}

	command.action = tokens[3];

	try
	{
		float x = std::stof(tokens[4]);
		float y = std::stof(tokens[5]);
		float z = std::stof(tokens[6]);
		command.value = Vector3(x, y, z);
	}
	catch (exception exc)
	{
		output = "Ошибка Set Object: координаты некорректны";
		return;
	}	

	if (ProcessObjectAction(command, output))
	{
		output = "Объект изменен";
		action_commands.push(command);
	}	
}

bool CommandExecutor::ProcessObjectAction(ActionCommand& command, std::string& output)
{
	if (
		(command.action == "expand" || command.action == "scl" || command.action == "scale") && 
		(command.value.x == 0.0 || command.value.y == 0.0 || command.value.z == 0.0))
	{
		output = "Ошибка Set Object: Scale не может быть равен 0";
		return false;
	}

	if (command.action == "pos" || command.action == "position")
	{
		command.undo_value = command.obj->Transform.GetPosition();
		command.obj->Transform.SetPosition(command.value);
	}
	else if (command.action == "rot" || command.action == "rotation")
	{
		command.undo_value = command.obj->Transform.GetRotationDegrees();
		command.obj->Transform.SetRotationDegrees(command.value);
	}
	else if (command.action == "scl" || command.action == "scale")
	{
		command.undo_value = command.obj->Transform.GetScale();
		command.obj->Transform.SetScale(command.value);
	}
	else if (command.action == "move")
	{
		command.undo_value = -command.value;
		command.obj->Transform.Move(command.value);
	}
	else if (command.action == "rotate")
	{
		command.undo_value = -command.value;
		command.obj->Transform.RotateDegrees(command.value);
	}
	else if (command.action == "expand")
	{
		command.undo_value = 1.0 / command.value;
		command.obj->Transform.Expand(command.value.x, command.value.y, command.value.z);
	}
	else
	{
		output = "Ошибка Set Object: Неизвестное действие " + command.action;
		return false;
	}

	return true;
}

void CommandExecutor::GetObjectInfo(const std::vector<std::string>& tokens, std::string& output)
{
	if (tokens.size() <= 2)
	{
		output = "Ошибка Get: Недостаточно аргументов";
		return;
	}

	if (tokens[1] == "object")
	{
		ActionCommand command; 

		std::string name = tokens[2];
		command.obj = m_scene->Get(name);
		if (!command.obj)
		{
			output = "Ошибка Set Object: Объект по имени " + name + " не найден";
			return;
		}
		command.action = tokens[3];
		command.value = Vector3(1.0, 1.0, 1.0);

		ProcessObjectAction(command, output);

		std::string info = "(" + 
			std::to_string(command.undo_value.x) + ";" +
			std::to_string(command.undo_value.y) + ";" +
			std::to_string(command.undo_value.z) + ")";

		command.value = command.undo_value;
		ProcessObjectAction(command, output);

		output = info;
	}
		
	else
		output = "Ошибка Get: неизвестная команда" + tokens[1];
}

void CommandExecutor::ProcessSave(const std::vector<std::string>& tokens, std::string& output)
{
	m_scene->Save();
	output = "Сцена сохранена";
}

void CommandExecutor::ProcessLoad(const std::vector<std::string>& tokens, std::string& output)
{
	
}

void CommandExecutor::ProcessUndo(std::string& output)
{
	if (action_commands.size() == 0)
	{
		output = "Ошибка Undo: отсутствуют команды в стеке";
		return;
	}
	
	ActionCommand last = action_commands.top();
	last.value = last.undo_value;
	ProcessObjectAction(last, output);

	action_commands.pop();
	output = "Команда отменена";
}

//<set|get|save|load>
//<obj_position|obj_rotation|obj_scale|obj_move|obj_rotate|obj_expand> <name_obj> <x> <y> <z>
