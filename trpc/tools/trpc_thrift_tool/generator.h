/*
  Copyright (c) 2020 Sogou, Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef __RPC_GENERATOR_H__
#define __RPC_GENERATOR_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include <list>
#include <string>
#include <algorithm>

#include "printer.h"
#include "parser.h"

struct GeneratorParams
{
	const char *out_dir;
	bool generate_skeleton;
	std::string idl_file;
	std::string input_dir;

	GeneratorParams() : out_dir(NULL), generate_skeleton(true) { }
};

class Generator
{
public:
	Generator(bool is_thrift):
		parser(is_thrift),
		printer(is_thrift)
	{
		this->suffix = ".trpc.";
		this->thrift_suffix = ".thrift.";
		this->skeleton_suffix = ".skeleton.";
		this->is_thrift = is_thrift;
	}

	bool generate(struct GeneratorParams& params);

protected:
	virtual bool generate_server_cpp_file(const idl_info& cur_info,
										  const std::string& idle_file_name);
	virtual bool generate_client_cpp_file(const idl_info& cur_info,
										  const std::string& idle_file_name);

	std::string server_cpp_file;
	std::string client_cpp_file;

private:
	bool generate_header(idl_info& cur_info, struct GeneratorParams& params);
	void generate_skeleton(const std::string& idl_file);

	bool generate_trpc_thrift_file(const idl_info& cur_info);
	
	bool generate_thrift_type_file(idl_info& cur_info);
	void thrift_replace_include(const idl_info& cur_info,
								std::vector<rpc_param>& params);

	bool init_file_names(const std::string& idl_file, const char *out_dir);

	Parser parser;
	Printer printer;
	idl_info info;
	std::string out_dir;
	std::string suffix;
	std::string thrift_suffix;
	std::string skeleton_suffix;
	std::string prefix;
	std::string trpc_header_file;
	std::string trpc_cc_file;
	std::string thrift_type_file;
	bool is_thrift;
};

#endif
