#pragma once

#include <string>
#include <vector>

namespace gitx {

int cmd_init(const std::vector<std::string>& args);
int cmd_hash_object(const std::vector<std::string>& args);
int cmd_cat_object(const std::vector<std::string>& args);
int cmd_add(const std::vector<std::string>& args);
int cmd_status(const std::vector<std::string>& args);
int cmd_write_tree(const std::vector<std::string>& args);
int cmd_commit(const std::vector<std::string>& args);
int cmd_log(const std::vector<std::string>& args);
int cmd_revert(const std::vector<std::string>& args);
int cmd_help(const std::vector<std::string>& args);

} // namespace gitx
