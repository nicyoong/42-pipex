#ifndef PIPEX_H
#define PIPEX_H

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include "libft/libft.h"

#define READ_END 0
#define WRITE_END 1

char    **split_command(char *cmd);
char    *get_cmd_path(char *cmd, char **envp);
char    *get_env_value(char *var, char **envp);
int     handle_here_doc(char *limiter, int *in_fd);

#endif