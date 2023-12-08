#include "utils.h"
#include <filesystem>
#include <string>

bool create_parents_dir(std::string& path){
    std::filesystem::path p=std::filesystem::absolute(path);
	bool ret=std::filesystem::create_directories(p);
    return ret;
}

std::string replaceAll(std::string &str, std::string oldStr, std::string newStr){
    std::string::size_type pos = str.find(oldStr);
    while(pos != std::string::npos){
        str.replace(pos, oldStr.size(), newStr);
        pos = str.find(oldStr);
    }
    return str;
}


std::string content_template_replace(std::string template_content,std::map<std::string,std::string>& mp){
    for(auto& [k,v]:mp){
        replaceAll(template_content, "%"+k+"%", v);
    }
    return template_content;
}