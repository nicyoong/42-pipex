#include "pipex.h"

char *ft_strjoin3(char const *s1, char const *s2, char const *s3)
{
    char    *temp;
    char    *result;

    if (!s1 || !s2 || !s3)
        return (NULL);
    temp = ft_strjoin(s1, s2);
    if (!temp)
        return (NULL);
    result = ft_strjoin(temp, s3);
    free(temp);
    return (result);
}

int ft_strcmp(const char *s1, const char *s2)
{
    size_t i;

    i = 0;
    while (s1[i] && s2[i] && s1[i] == s2[i])
        i++;
    return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

void    ft_free_split(char **split)
{
    int i;

    if (!split)
        return;
    i = 0;
    while (split[i])
    {
        free(split[i]);
        i++;
    }
    free(split);
}

char    **split_command(char *cmd)
{
    return (ft_split(cmd, ' '));
}

char    *get_env_value(char *var, char **envp)
{
    size_t var_len;

    if (!var || !envp)
        return (NULL);
    var_len = ft_strlen(var);
    while (*envp)
    {
        if (ft_strncmp(*envp, var, var_len) == 0 && (*envp)[var_len] == '=')
            return (&(*envp)[var_len + 1]);
        envp++;
    }
    return (NULL);
}

char    *get_cmd_path(char *cmd, char **envp)
{
    char    *path_env;
    char    **dirs;
    char    *full_path;
    int     i;

    if (ft_strchr(cmd, '/') != NULL)
    {
        if (access(cmd, X_OK) == 0)
            return (ft_strdup(cmd));
        return (NULL);
    }
    path_env = get_env_value("PATH", envp);
    if (!path_env)
        return (NULL);
    dirs = ft_split(path_env, ':');
    if (!dirs)
        return (NULL);
    i = 0;
    while (dirs[i])
    {
        full_path = ft_strjoin3(dirs[i], "/", cmd);
        if (!full_path)
        {
            ft_free_split(dirs);
            return (NULL);
        }
        if (access(full_path, X_OK) == 0)
        {
            ft_free_split(dirs);
            return (full_path);
        }
        free(full_path);
        i++;
    }
    ft_free_split(dirs);
    return (NULL);
}

int handle_here_doc(char *limiter, int *in_fd)
{
    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) == -1)
        return (-1);
    pid = fork();
    if (pid == -1)
        return (-1);
    if (pid == 0)
    {
        char *line;
        size_t limiter_len = ft_strlen(limiter);
        close(pipefd[READ_END]);
        while (1)
        {
            ft_putstr_fd("heredoc> ", STDOUT_FILENO);
            line = get_next_line(STDIN_FILENO);
            if (!line)
                break;
            if (ft_strncmp(line, limiter, limiter_len) == 0
                && (line[limiter_len] == '\n' || line[limiter_len] == '\0'))
            {
                free(line);
                break;
            }
            write(pipefd[WRITE_END], line, ft_strlen(line));
            free(line);
        }
        close(pipefd[WRITE_END]);
        exit(0);
    }
    else
    {
        close(pipefd[WRITE_END]);
        waitpid(pid, NULL, 0);
        *in_fd = pipefd[READ_END];
    }
    return (0);
}
