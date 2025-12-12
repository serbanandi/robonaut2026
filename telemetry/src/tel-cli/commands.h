#ifndef COMMANDS_H
#define COMMANDS_H

int cmd_logs(void* ctx, int argc, char* argv[]);

int cmd_show_vars(void* ctx);

int cmd_ls(void* ctx);

int cmd_read_var(void* ctx, const char* varname);

int cmd_write_var(void* ctx, const char* name, const char* val);

int cmd_text(void* ctx, int argc, char* argv[]);

#endif // COMMANDS_H
