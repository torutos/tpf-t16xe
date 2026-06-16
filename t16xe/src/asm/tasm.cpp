#include "tasm.hpp"
#include <exception>
#include <iostream>
#include <algorithm>

int safe_stoi(const std::string &str, int base = 0)
{
	try
	{
		return std::stoi(str, nullptr, base);
	}
	catch (std::exception &e)
	{
		std::cerr << "STOI ERROR: '" << str << "' - " << e.what() << "\n";
		throw;
	}
}
std::string clean_token(const std::string &s)
{
	std::string t = s;
	size_t comma = t.find(',');
	if (comma != std::string::npos)
		t.erase(comma, 1);
	size_t start = t.find_first_not_of(" \t");
	if (start == std::string::npos)
		return "";
	size_t end = t.find_last_not_of(" \t");
	return t.substr(start, end - start + 1);
}

uint16_t tasm::parse_immediate(const std::string& src) {
    if (src[0] == '\'') {
        return src[1];
    if (src.length() > 1 && src[1] == '\'') {
        return src[2];
    }
    return safe_stoi(src.substr(1));
}



void tasm::assemble_line(const std::string &line)
{
	// std::cout << "LINE: " << line << "\n";
	// std::cout << "start/assemble_line/strel.done output: "
	//          << labels.find("strlen.done")->second << std::endl;
	std::istringstream iss(line);
	std::string mnemonic;
	iss >> mnemonic;

	if (mnemonic == "org")
	{
		std::string arg;
		iss >> arg;
		org_addr = safe_stoi(arg);
		return;
	}
	if (mnemonic.empty() || mnemonic[0] == ';')
		return;

	if (mnemonic == "HLT")
	{
		code_output.push_back(0x00);
	}
	else if (mnemonic == "MOV")
	{
		std::string dst, src;
		iss >> dst >> src;
		dst = clean_token(dst);
		src = clean_token(src);

		if (src[0] == '#' || src[0] == '\'')
		{
			code_output.push_back(0x01);
			code_output.push_back(regs[dst]);
			uint16_t val = parse_immediate(src);
			code_output.push_back(val & 0xFF);
			code_output.push_back(val >> 8);
		}
		else if (labels.count(src))
		{
			// MOV reg, label
			code_output.push_back(0x01);
			code_output.push_back(regs[dst]);
			uint16_t val = labels.find(src)->second;
			code_output.push_back(val & 0xFF);
			code_output.push_back(val >> 8);
		}
		else if (!current_global_label.empty() && labels.count(current_global_label + src))
		{
			// MOV reg, локальная_метка
			code_output.push_back(0x01);
			code_output.push_back(regs[dst]);
			uint16_t val = labels.find(current_global_label + src)->second;
			code_output.push_back(val & 0xFF);
			code_output.push_back(val >> 8);
		}
		else if (regs.count(src))
		{
			// MOV reg, reg (бывший MOVR)
			code_output.push_back(0x02); // MOV_REG_REG
			code_output.push_back(regs[dst]);
			code_output.push_back(regs[src]);
		}
	}
	else if (mnemonic == "STOR")
	{
		std::string src, dst;
		iss >> src >> dst;
		// std::cout << "STOR: src=" << src << " dst=" << dst << "\n";
		dst = clean_token(dst);
		src = clean_token(src);
		if (src[0] == '[')
		{
			src = src.substr(1);

			src.pop_back(); // убрать ']' или ','
			if (regs.count(src))
			{
				// STOR [reg], reg
				code_output.push_back(0x25); // новый опкод
				code_output.push_back(regs[src]);
				code_output.push_back(regs[dst]);
			}
			else
			{
				// STOR [addr], reg
				uint16_t addr = safe_stoi(src);
				code_output.push_back(0x0C);
				code_output.push_back(addr & 0xFF);
				code_output.push_back(addr >> 8);
				code_output.push_back(regs[dst]);
			}
		}
		// std::cout << "STOR src after clean: '" << src << "'\n";
	}
	else if (mnemonic == "LOAD")
	{
		std::string dst, src;
		iss >> dst >> src;
		dst = clean_token(dst);
		src = clean_token(src);

		if (src[0] == '[')
		{
			src = src.substr(1);
			src.pop_back(); // убрать ']'

			if (src[0] == 'B' || src[0] == 'A' || src[0] == 'C' || src[0] == 'D' ||
				src[0] == 'S')
			{
				// LOAD reg, [reg]
				code_output.push_back(0x0F); // MOV_REG_MEMREG
				code_output.push_back(regs[dst]);
				code_output.push_back(regs[src]);
			}
			else
			{
				// LOAD reg, [addr] — абсолютный адрес
				uint16_t addr = safe_stoi(src);
				code_output.push_back(0x0B); // MOV_REG_MEM
				code_output.push_back(regs[dst]);
				code_output.push_back(addr & 0xFF);
				code_output.push_back(addr >> 8);
			}
		}
	}
	else if (mnemonic == "ADD")
	{
		std::string dst, src;
		iss >> dst >> src;
		dst = clean_token(dst);
		src = clean_token(src);
		if (src[0] == '#')
		{
			code_output.push_back(0x17); // ADD_REG_IMM
			code_output.push_back(regs[dst]);
			uint16_t val = parse_immediate(src);
			code_output.push_back(val & 0xFF);
			code_output.push_back(val >> 8);
		}
		else
		{
			code_output.push_back(0x03); // ADD_REG_REG
			code_output.push_back(regs[dst]);
			code_output.push_back(regs[src]);
		}
	}
	else if (mnemonic == "SUB")
	{
		std::string dst, src;
		iss >> dst >> src;
		dst = clean_token(dst);
		src = clean_token(src);
		if (src[0] == '#')
		{
			code_output.push_back(0x18); // ADD_REG_IMM
			code_output.push_back(regs[dst]);
			uint16_t val = parse_immediate(src);
			code_output.push_back(val & 0xFF);
			code_output.push_back(val >> 8);
		}
		else
		{
			code_output.push_back(0x04); // ADD_REG_REG
			code_output.push_back(regs[dst]);
			code_output.push_back(regs[src]);
		}
	}
	else if (mnemonic == "CMP")
	{
		std::string dst, src;
		iss >> dst >> src;
		dst = clean_token(dst);
		src = clean_token(src);
		if (src[0] == '#')
		{
			code_output.push_back(0x19); // ADD_REG_IMM
			code_output.push_back(regs[dst]);
			uint16_t val = parse_immediate(src);
			code_output.push_back(val & 0xFF);
			code_output.push_back(val >> 8);
		}
		else
		{
			code_output.push_back(0x05); // ADD_REG_REG
			code_output.push_back(regs[dst]);
			code_output.push_back(regs[src]);
		}
	}
	else if (mnemonic == "JEQ")
	{
		std::string arg;
		iss >> arg;
		int8_t offset;
		if (labels.count(arg))
		{
			// std::cout << "JEQ " << arg << " label_addr=" <<
			// labels.find(arg)->second
			//           << " code_size=" << code_output.size() << "\n";
			// Стало:
			offset = labels.find(arg)->second - (code_output.size() + 2);
		}
		else if (!current_global_label.empty() && labels.count(current_global_label + arg))
		{
			// локальная метка с префиксом
			arg = current_global_label + arg;
			offset = labels.find(arg)->second - (code_output.size() + 2);
		}
		else
		{
			offset = safe_stoi(arg);
		}
		code_output.push_back(0x06);
		code_output.push_back(offset);
	}
	else if (mnemonic == "JNE")
	{
		std::string arg;
		iss >> arg;
		int8_t offset;
		if (labels.count(arg))
		{
			offset = labels.find(arg)->second - (code_output.size() + 2);
		}
		else if (!current_global_label.empty() && labels.count(current_global_label + arg))
		{
			// локальная метка с префиксом
			arg = current_global_label + arg;
			offset = labels.find(arg)->second - (code_output.size() + 2);
		}
		else
		{
			offset = safe_stoi(arg);
		}
		code_output.push_back(0x07);
		code_output.push_back(offset);
	}
	else if (mnemonic == "JMP")
	{
		// std::cout << "DEBUG JMP 1: strlen.done=" <<
		// labels.find("strlen.done")->second<< "\n";
		std::string arg;
		iss >> arg;
		// std::cout << "DEBUG JMP 2: arg=" << arg
		//           << " strlen.done=" << labels.find("strlen.done")->second <<
		//           "\n";
		uint8_t seg = 0;
		uint16_t addr;
		if (labels.count(arg))
		{
			//   std::cout << "DEBUG JMP 3: before labels[arg] strlen.done="
			//             << labels.find("strlen.done")->second << "\n";
			addr = labels.find(arg)->second;
			// std::cout << "DEBUG JMP 4: after labels[arg] strlen.done="
			//           << labels.find("strlen.done")->second << " addr=" << addr <<
			//           "\n";
		}
		else if (!current_global_label.empty() && labels.count(current_global_label + arg))
		{
			arg = current_global_label + arg;
			addr = labels.find(arg)->second;
		}
		else
		{
			addr = safe_stoi(arg);
		}
		// std::cout << "DEBUG JMP 5: before push strlen.done="
		//          << labels.find("strlen.done")->second << "\n";
		code_output.push_back(0x08);
		// std::cout << "DEBUG JMP 6: after push 08 strlen.done="
		//           << labels.find("strlen.done")->second << "\n";
		code_output.push_back(seg);
		// std::cout << "DEBUG JMP 7: after push seg strlen.done="
		//           << labels.find("strlen.done")->second << "\n";
		code_output.push_back(addr & 0xFF);
		// std::cout << "DEBUG JMP 8: after push lo strlen.done="
		//           << labels.find("strlen.done")->second << "\n";
		code_output.push_back(addr >> 8);
		// std::cout << "DEBUG JMP 9: end strlen.done=" <<
		// labels.find("strlen.done")->second
		//           << "\n";
	}
	else if (mnemonic == "PUSH")
	{
		std::string reg;
		iss >> reg;
		code_output.push_back(0x09);
		code_output.push_back(regs[reg]);
	}
	else if (mnemonic == "POP")
	{
		std::string reg;
		iss >> reg;
		code_output.push_back(0x0A);
		code_output.push_back(regs[reg]);
	}
	else if (mnemonic == "INC")
	{
		std::string reg;
		iss >> reg;
		code_output.push_back(0x0D);
		code_output.push_back(regs[reg]);
	}
	else if (mnemonic == "DEC")
	{
		std::string reg;
		iss >> reg;
		code_output.push_back(0x0E);
		code_output.push_back(regs[reg]);
	}
	else if (mnemonic == "AND")
	{
		std::string dst, src;
		iss >> dst >> src;
		dst = clean_token(dst);
		src = clean_token(src);
		if (src[0] == '#')
		{
			code_output.push_back(0x1A); // ADD_REG_IMM
			code_output.push_back(regs[dst]);
			uint16_t val = parse_immediate(src);
			code_output.push_back(val & 0xFF);
			code_output.push_back(val >> 8);
		}
		else
		{
			code_output.push_back(0x10); // ADD_REG_REG
			code_output.push_back(regs[dst]);
			code_output.push_back(regs[src]);
		}
	}
	else if (mnemonic == "OR")
	{
		std::string dst, src;
		iss >> dst >> src;
		dst = clean_token(dst);
		src = clean_token(src);
		if (src[0] == '#')
		{
			code_output.push_back(0x1B); // ADD_REG_IMM
			code_output.push_back(regs[dst]);
			uint16_t val;
			uint16_t val = parse_immediate(src);
			code_output.push_back(val & 0xFF);
			code_output.push_back(val >> 8);
		}
		else
		{
			code_output.push_back(0x11); // ADD_REG_REG
			code_output.push_back(regs[dst]);
			code_output.push_back(regs[src]);
		}
	}
	else if (mnemonic == "XOR")
	{
		std::string dst, src;
		iss >> dst >> src;
		dst = clean_token(dst);
		src = clean_token(src);
		if (src[0] == '#')
		{
			code_output.push_back(0x1C); // ADD_REG_IMM
			code_output.push_back(regs[dst]);
			uint16_t val;
			uint16_t val = parse_immediate(src);
			code_output.push_back(val & 0xFF);
			code_output.push_back(val >> 8);
		}
		else
		{
			code_output.push_back(0x12); // ADD_REG_REG
			code_output.push_back(regs[dst]);
			code_output.push_back(regs[src]);
		}
	}
	else if (mnemonic == "SHL")
	{
		std::string dst, src;
		iss >> dst >> src;
		dst = clean_token(dst);
		src = clean_token(src);
		code_output.push_back(0x13);
		code_output.push_back(regs[dst]);
		code_output.push_back(regs[src]);
	}
	else if (mnemonic == "SHR")
	{
		std::string dst, src;
		iss >> dst >> src;
		dst = clean_token(dst);
		src = clean_token(src);
		code_output.push_back(0x14);
		code_output.push_back(regs[dst]);
		code_output.push_back(regs[src]);
	}
	else if (mnemonic == "CALL")
	{
		std::string arg;
		iss >> arg;
		if (labels.count(arg))
		{
			// std::cout << labels.find(arg)->second;
		}
		// std::cout << "\n";
		uint8_t seg = 0;
		uint16_t addr;
		if (labels.count(arg))
		{
			addr = labels.find(arg)->second;
		}
		else if (!current_global_label.empty() && labels.count(current_global_label + arg))
		{
			arg = current_global_label + arg;
			addr = labels.find(arg)->second;
		}
		else
		{
			addr = safe_stoi(arg);
		}
		code_output.push_back(0x15);
		code_output.push_back(seg);
		code_output.push_back(addr & 0xFF);
		code_output.push_back(addr >> 8);
	}
	else if (mnemonic == "RET")
	{
		code_output.push_back(0x16);
	}
	else if (mnemonic == "LDB")
	{
		std::string dst, src;
		iss >> dst >> src;
		dst = clean_token(dst);
		src = clean_token(src);
		if (src[0] == '[')
		{
			src = src.substr(1);
			src.pop_back();
			if (regs.count(src))
			{
				// LDB reg, [reg]
				code_output.push_back(0x1D);
				code_output.push_back(regs[dst]);
				code_output.push_back(regs[src]);
			}
			else
			{
				// LDB reg, [addr]
				uint16_t addr = safe_stoi(src);
				code_output.push_back(0x23); // новый опкод
				code_output.push_back(regs[dst]);
				code_output.push_back(addr & 0xFF);
				code_output.push_back(addr >> 8);
			}
		}
	}
	else if (mnemonic == "STB")
	{
		try
		{
			std::string src, dst;
			iss >> src >> dst;
			dst = clean_token(dst);
			src = clean_token(src);
			// std::cout << "STB: src=" << src << " comma=" << " dst=" << dst << "\n";
			if (src[0] == '[')
			{
				src = src.substr(1);
				src.pop_back();
				if (regs.count(src))
				{
					// STB [reg], reg
					code_output.push_back(0x1E);
					code_output.push_back(regs[src]);
					code_output.push_back(regs[dst]);
				}
				else
				{
					// STB [addr], reg
					uint16_t addr = safe_stoi(src);
					code_output.push_back(0x24); // новый опкод
					code_output.push_back(addr & 0xFF);
					code_output.push_back(addr >> 8);
					code_output.push_back(regs[dst]);
				}
			}
		}
		catch (std::exception &ex)
		{
			// std::cerr << "Exception: " << ex.what() << "\n";
		}
	}
	else if (mnemonic == "INT")
	{
		std::string arg;
		iss >> arg;
		uint8_t vector = safe_stoi(arg);
		code_output.push_back(0x1F);
		code_output.push_back(vector);
	}
	else if (mnemonic == "IRET")
	{
		code_output.push_back(0x20);
	}
	else if (mnemonic == "STI")
	{
		code_output.push_back(0x21);
	}
	else if (mnemonic == "CLI")
	{
		code_output.push_back(0x22);
	}

	else if (mnemonic == "JCS")
	{
		std::string arg;
		iss >> arg;
		int8_t offset;
		if (labels.count(arg))
		{
			offset = labels.find(arg)->second - (code_output.size() + 2);
		}
		else if (!current_global_label.empty() && labels.count(current_global_label + arg))
		{
			arg = current_global_label + arg;
			offset = labels.find(arg)->second - (code_output.size() + 2);
		}
		else
		{
			offset = safe_stoi(arg);
		}
		code_output.push_back(0x28);
		code_output.push_back(offset);
	}
	else if (mnemonic == "JCC")
	{
		std::string arg;
		iss >> arg;
		int8_t offset;
		if (labels.count(arg))
		{
			offset = labels.find(arg)->second - (code_output.size() + 2);
		}
		else if (!current_global_label.empty() && labels.count(current_global_label + arg))
		{
			arg = current_global_label + arg;
			offset = labels.find(arg)->second - (code_output.size() + 2);
		}
		else
		{
			offset = safe_stoi(arg);
		}
		code_output.push_back(0x29);
		code_output.push_back(offset);
	}
}

void tasm::assemble_file(const std::string &path)
{
	std::ifstream file(path);
	std::vector<std::string> lines;
	std::string line;

	while (std::getline(file, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		size_t comment = line.find(';');
		if (comment != std::string::npos)
			line = line.substr(0, comment);
		if (line.find_first_not_of(" \t") == std::string::npos)
			continue;

		if (line.find("%include") != std::string::npos)
		{
			size_t start = line.find('"');
			size_t end = line.rfind('"');
			if (start != std::string::npos && end != std::string::npos &&
				end > start)
			{
				std::string inc_path = line.substr(start + 1, end - start - 1);
				std::ifstream inc_file(inc_path);
				if (!inc_file)
				{
					// std::cerr << "ERROR: cannot open " << inc_path << "\n";
					continue;
				}
				std::string inc_line;
				int count = 0;
				while (std::getline(inc_file, inc_line))
				{
					if (!inc_line.empty() && inc_line.back() == '\r')
						inc_line.pop_back();
					lines.push_back(inc_line);
					count++;
					// std::cout << "INC: [" << inc_line << "]\n";
				}
				// std::cout << "Included " << count << " lines from " << inc_path <<
				// "\n";
			}
		}
		else
		{
			lines.push_back(line);
		}
	}
	collect_macros(lines);
	collect_equ(lines);

	// Раскрываем макросы
	std::vector<std::string> expanded_lines;
	for (const auto &ln : lines)
	{
		auto expanded = expand_macro(ln);
		expanded_lines.insert(expanded_lines.end(), expanded.begin(), expanded.end());
	}
	lines = expanded_lines;

	// Замена констант (теперь и в раскрытых макросах)
	for (auto &ln : lines)
	{
		ln = replace_equ(ln);
	}

	expand_times(lines);
	Section current = Section::None;
	uint16_t data_offset = 0;
	uint16_t code_offset = org_addr; // начинаем с org_addr

	// std::cout << "=== LINES AFTER EXPANSION ===\n";
	// for (auto &ln : lines)
	// {
	//	std::cout << ln << "\n";
	// }
	// Первый проход: собираем метки
	for (const auto &ln : lines)
	{
		if (ln.find("org ") == 0)
		{
			std::istringstream iss(ln);
			std::string org_str;
			uint16_t addr;
			iss >> org_str >> addr;
			org_addr = addr;
			code_offset = addr;
			continue;
		}
		if (ln == "section .data")
		{
			current = Section::Data;
			continue;
		}
		if (ln == "section .code")
		{
			current = Section::Code;
			continue;
		}

		std::string work = ln;
		size_t colon = work.find(':');
		if (colon != std::string::npos && colon > 0)
		{
			std::string label = work.substr(0, colon);
			label.erase(0, label.find_first_not_of(" \t"));
			label.erase(label.find_last_not_of(" \t") + 1);

			// Локальные метки (начинаются с точки)
			if (!label.empty() && label[0] == '.')
			{
				label = current_global_label + label;
			}
			else
			{
				current_global_label = label;
			}

			if (current == Section::Data)
				labels[label] = data_offset;
			else if (current == Section::Code)
				labels[label] = code_offset;

			work = work.substr(colon + 1);
			if (work.find_first_not_of(" \t") == std::string::npos)
				continue;
		}

		if (current == Section::Data)
		{
			data_offset += data_size(work);
		}
		else if (current == Section::Code)
		{
			if (current == Section::Code)
			{
				// std::cout << "  [" << work << "] size=" << instruction_size(work)
				//           << " offset before=" << code_offset;
				// code_offset += instruction_size(work);
				// std::cout << " after=" << code_offset << "\n";
			}
			int sz = instruction_size(work);
			// std::cout << "  INST: [" << work << "] size=" << sz
			// 		  << " code_offset=" << code_offset;
			code_offset += sz;
			// std::cout << " -> " << code_offset << "\n";
		}
	}

	// std::cout << "Labels: ";
	// for (auto &pair : labels)
	// {
	// 	std::cout << pair.first << "=" << pair.second << " ";
	// }
	// std::cout << "\n";

	// std::cout << "strlen.done=" << labels.find("strlen.done")->second
	// << " strlen.loop=" << labels.find("strlen.loop")->second<< "\n";
	// std::cout << "AFTER PASS1: strlen.done=" <<
	// labels.find("strlen.done")->second<< "\n"; std::cout << "Labels: "; for
	// (auto& pair : labels) {  std::cout << pair.first << "=" << pair.second <<
	// "
	// ";
	// }
	// std::cout << "\n";

	auto labels_copy = labels;
	// Второй проход: генерируем
	data_output.clear();
	code_output.clear();
	current = Section::None;

	for (const auto &ln : lines)
	{
		// std::cout << "start/assemble_file/strel.done output: "
		//           << labels_copy.find("strlen.done")->second << std::endl;
		if (ln == "section .data")
		{
			current = Section::Data;
			continue;
		}
		if (ln == "section .code")
		{
			current = Section::Code;
			continue;
		}

		std::string work = ln;
		size_t colon = work.find(':');
		if (colon != std::string::npos && colon > 0)
		{
			work = work.substr(colon + 1);
			if (work.find_first_not_of(" \t") == std::string::npos)
				continue;
		}

		if (current == Section::Data)
		{
			assemble_data(work);
		}
		else if (current == Section::Code)
		{
			assemble_line(work);
		}
	}
}
void tasm::write_toru(const std::string &path)
{
	std::ofstream file(path, std::ios::binary);
	uint16_t dsize = data_output.size();
	uint16_t csize = code_output.size();
	file.put(dsize & 0xFF);
	file.put(dsize >> 8);
	file.put(csize & 0xFF);
	file.put(csize >> 8);
	file.put(org_addr & 0xFF); // ← ДОБАВЬ ЭТО
	file.put(org_addr >> 8);   // ← И ЭТО

	for (uint8_t byte : data_output)
		file.put(byte);
	for (uint8_t byte : code_output)
		file.put(byte);
}
int tasm::data_size(const std::string &line)
{
	std::istringstream iss(line);
	std::string name, directive;
	iss >> name >> directive; // name: db/dw/ds

	if (directive == "db")
	{
		// db 0x48, 0x65, 0x00
		std::string rest;
		std::getline(iss, rest);
		return std::count(rest.begin(), rest.end(), ',') + 1;
	}
	if (directive == "dw")
	{
		// dw 0x1234, 0x5678
		std::string rest;
		std::getline(iss, rest);
		return (std::count(rest.begin(), rest.end(), ',') + 1) * 2;
	}
	if (directive == "ds")
	{
		std::string str;
		std::getline(iss, str); // читаем всё до конца строки
		str.erase(0, str.find_first_not_of(" \t"));
		str.erase(str.find_last_not_of(" \t") + 1);
		if (str.front() == '"')
			str = str.substr(1);
		if (str.back() == '"')
			str.pop_back();
		return str.size() + 1;
	}
	if (directive == "resb")
	{
		std::string count_str;
		iss >> count_str;
		return std::stoi(count_str, nullptr, 0);
	}
	if (directive == "resw")
	{
		std::string count_str;
		iss >> count_str;
		return std::stoi(count_str, nullptr, 0) * 2;
	}
	if (directive == "fill")
	{
		std::string count_str, value_str;
		iss >> count_str >> value_str;
		return safe_stoi(count_str);
	}
	return 0;
}

void tasm::assemble_data(const std::string &line)
{
	// std::cout << "start/assemble_data/strel.done output: "
	//           << labels.find("strlen.done")->second << std::endl;
	std::istringstream iss(line);
	std::string directive;
	iss >> directive;
	if (directive == "db")
	{
		std::string token;
		while (std::getline(iss, token, ','))
		{
			// убираем пробелы
			token.erase(0, token.find_first_not_of(" \t"));
			uint8_t val = std::stoi(token, nullptr, 0);
			data_output.push_back(val);
		}
	}
	else if (directive == "dw")
	{
		std::string token;
		while (std::getline(iss, token, ','))
		{
			token.erase(0, token.find_first_not_of(" \t"));
			// Убираем '#' если есть
			if (token[0] == '#')
				token = token.substr(1);
			uint16_t val = std::stoi(token, nullptr, 0);
			data_output.push_back(val & 0xFF);
			data_output.push_back(val >> 8);
		}
	}
	else if (directive == "ds")
	{
		std::string str;
		std::getline(iss, str);
		str.erase(0, str.find_first_not_of(" \t"));
		str.erase(str.find_last_not_of(" \t") + 1);
		if (str.front() == '"')
			str = str.substr(1);
		if (str.back() == '"')
			str.pop_back();
		for (char c : str)
			data_output.push_back(c);
		data_output.push_back(0); // терминатор
	}
	else if (directive == "resb")
	{
		std::string count_str;
		iss >> count_str;
		int count = std::stoi(count_str, nullptr, 0);
		for (int i = 0; i < count; i++)
		{
			data_output.push_back(0);
		}
	}
	else if (directive == "resw")
	{
		std::string count_str;
		iss >> count_str;
		int count = std::stoi(count_str, nullptr, 0);
		for (int i = 0; i < count * 2; i++)
		{
			data_output.push_back(0);
		}
	}
	else if (directive == "fill")
	{
		std::string count_str, value_str;
		iss >> count_str >> value_str;
		int count = safe_stoi(count_str);
		uint8_t val = safe_stoi(value_str);
		for (int i = 0; i < count; i++)
		{
			data_output.push_back(val);
		}
	}
	// std::cout << "end/assemble_data/strel.done output: " <<
	// labels.find("strlen.done")->second
	//           << std::endl;
}

int tasm::instruction_size(const std::string &line)
{
	std::istringstream iss(line);
	std::string mnemonic;
	iss >> mnemonic;

	if (mnemonic.empty() || mnemonic[0] == ';')
		return 0;
	if (mnemonic.back() == ':')
		return 0;

	if (mnemonic == "HLT")
		return 1;
	if (mnemonic == "MOV")
	{
		std::istringstream tmp(line);
		std::string m, dst, src;
		tmp >> m >> dst >> src;
		src = clean_token(src);
		if (regs.count(src))
			return 3; // MOV reg, reg
		return 4;	  // MOV reg, imm или MOV reg, label
	}
	if (mnemonic == "STOR")
		return 4;
	if (mnemonic == "LOAD")
		return 4;
	if (mnemonic == "JEQ")
		return 2;
	if (mnemonic == "JNE")
		return 2;
	if (mnemonic == "JMP")
		return 4;
	if (mnemonic == "PUSH")
		return 2;
	if (mnemonic == "POP")
		return 2;
	if (mnemonic == "INC")
		return 2;
	if (mnemonic == "DEC")
		return 2;
	if (mnemonic == "SHL")
		return 3;
	if (mnemonic == "SHR")
		return 3;
	if (mnemonic == "CALL")
		return 4;
	if (mnemonic == "RET")
		return 1;
	if (mnemonic == "ADD" || mnemonic == "SUB" || mnemonic == "CMP" ||
		mnemonic == "AND" || mnemonic == "OR" || mnemonic == "XOR")
	{
		if (line.find('#') != std::string::npos)
			return 4;
		return 3;
	}
	if (mnemonic == "org")
		return 0;
	if (mnemonic == "INT")
		return 2;
	if (mnemonic == "IRET")
		return 1;
	if (mnemonic == "STI")
		return 1;
	if (mnemonic == "CLI")
		return 1;
	if (mnemonic == "JCS")
		return 2;
	if (mnemonic == "JCC")
		return 2;
	if (mnemonic == "LDB")
	{
		// проверим, регистр или число внутри скобок
		std::string tmp = line;
		size_t br = tmp.find('[');
		if (br != std::string::npos)
		{
			std::string inside = tmp.substr(br + 1);
			size_t cl = inside.find(']');
			if (cl != std::string::npos)
				inside = inside.substr(0, cl);
			inside = clean_token(inside); // убрать запятую и пробелы
			if (regs.count(inside))
				return 3; // [reg] — 3 байта
		}
		return 4; // [addr] — 4 байта
	}
	if (mnemonic == "STB")
	{
		std::string tmp = line;
		size_t br = tmp.find('[');
		if (br != std::string::npos)
		{
			std::string inside = tmp.substr(br + 1);
			size_t cl = inside.find(']');
			if (cl != std::string::npos)
				inside = inside.substr(0, cl);
			inside = clean_token(inside);
			if (regs.count(inside))
				return 3;
		}
		return 4;
	}
	return 0;
}
void tasm::collect_macros(std::vector<std::string> &lines)
{
	std::vector<std::string> new_lines;
	bool in_macro = false;
	Macro current_macro;

	for (const auto &ln : lines)
	{
		std::istringstream iss(ln);
		std::string first_word;
		iss >> first_word;

		if (first_word == "%macro")
		{
			in_macro = true;
			current_macro.body.clear();
			iss >> current_macro.name >> current_macro.num_params;
		}
		else if (first_word == "%endmacro")
		{
			in_macro = false;
			macros[current_macro.name] = current_macro;
		}
		else if (!in_macro)
		{
			new_lines.push_back(ln);
		}
		else
		{
			current_macro.body.push_back(ln);
		}
	}

	lines = new_lines;
}

std::vector<std::string> tasm::expand_macro(const std::string &line)
{
	std::istringstream iss(line);
	std::string name;
	iss >> name;

	if (!macros.count(name))
		return {line};

	Macro &m = macros[name];
	std::vector<std::string> args;
	std::string arg;
	while (iss >> arg)
	{
		args.push_back(arg);
	}

	// std::cout << "MACRO " << name << " args: ";
	//for (auto &a : args)
	//	std::cout << "[" << a << "] ";
	// std::cout << "\n";

	std::vector<std::string> result;
	for (const auto &body_line : m.body)
	{
		std::string expanded = body_line;
		for (int i = 0; i < args.size(); i++)
		{
			std::string placeholder = "%" + std::to_string(i + 1);
			// std::cout << "  replacing " << placeholder << " with " << args[i] << " in " << expanded << "\n";
			size_t pos = expanded.find(placeholder);
			if (pos != std::string::npos)
			{
				expanded.replace(pos, placeholder.length(), args[i]);
			}
		}
		// std::cout << "  result: " << expanded << "\n";
		result.push_back(expanded);
	}
	return result;
}

void tasm::collect_equ(std::vector<std::string> &lines)
{
	std::vector<std::string> new_lines;

	for (const auto &ln : lines)
	{
		std::istringstream iss(ln);
		std::string name, equ_directive, value;
		iss >> name >> equ_directive;

		if (equ_directive == "equ")
		{
			std::getline(iss, value);
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t") + 1);
			equ_constants[name] = value;
		}
		else
		{
			new_lines.push_back(ln);
		}
	}

	lines = new_lines;
}

std::string tasm::replace_equ(const std::string &line)
{
	std::string result = line;
	for (auto &pair : equ_constants)
	{
		const std::string &name = pair.first;
		const std::string &value = pair.second;
		size_t pos = 0;
		// ОТЛАДКА
		if (result.find(name) != std::string::npos)
		{
			// std::cout << "  EQU found '" << name << "' in '" << result << "' -> '" << value << "'\n";
		}
		while ((pos = result.find(name, pos)) != std::string::npos)
		{
			bool valid_start = (pos == 0 || !isalnum(result[pos - 1]));
			bool valid_end = (pos + name.length() >= result.length() || !isalnum(result[pos + name.length()]));

			if (valid_start && valid_end)
			{
				result.replace(pos, name.length(), value);
				pos += value.length();
			}
			else
			{
				pos += name.length();
			}
		}
	}
	return result;
}

void tasm::expand_times(std::vector<std::string> &lines)
{
	std::vector<std::string> new_lines;

	for (const auto &ln : lines)
	{
		std::istringstream iss(ln);
		std::string first_word;
		iss >> first_word;

		if (first_word == "times")
		{
			int count;
			iss >> count;

			// получить оставшуюся часть строки (инструкцию)
			std::string rest;
			std::getline(iss, rest);
			rest.erase(0, rest.find_first_not_of(" \t"));

			for (int i = 0; i < count; i++)
			{
				new_lines.push_back(rest);
			}
		}
		else
		{
			new_lines.push_back(ln);
		}
	}

	lines = new_lines;
}