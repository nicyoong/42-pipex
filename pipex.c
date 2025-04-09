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

int main(int argc, char **argv, char **envp)
{
    int in_fd;
    int out_fd;
    int here_doc = 0;
    int num_cmds;
    pid_t *child_pids;
    int i;

    if (argc < 5)
    {
        ft_putstr_fd("Error: too few arguments\n", STDERR_FILENO);
        exit(1);
    }
    if (ft_strcmp(argv[1], "here_doc") == 0)
    {
        here_doc = 1;
        if (argc < 6)
        {
            ft_putstr_fd("Error: here_doc requires at least 4 arguments\n", STDERR_FILENO);
            exit(1);
        }
        if (handle_here_doc(argv[2], &in_fd) == -1)
        {
            perror("pipex");
            exit(1);
        }
        out_fd = open(argv[argc - 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
        num_cmds = argc - 4;
    }
    else
    {
        in_fd = open(argv[1], O_RDONLY);
        if (in_fd == -1)
        {
            perror("pipex");
            exit(1);
        }
        out_fd = open(argv[argc - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_fd == -1)
        {
            perror("pipex");
            exit(1);
        }
        num_cmds = argc - 3;
    }
    child_pids = malloc(num_cmds * sizeof(pid_t));
    if (!child_pids)
    {
        perror("pipex");
        exit(1);
    }
    int prev_pipe_read = in_fd;
    for (i = 0; i < num_cmds; i++)
    {
        int pipefd[2];
        if (i < num_cmds - 1 && pipe(pipefd) == -1)
        {
            perror("pipex");
            exit(1);
        }
        child_pids[i] = fork();
        if (child_pids[i] == -1)
        {
            perror("pipex");
            exit(1);
        }
        if (child_pids[i] == 0)
        {
            dup2(prev_pipe_read, STDIN_FILENO);
            close(prev_pipe_read);
            if (i == num_cmds - 1)
            {
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }
            else
            {
                dup2(pipefd[WRITE_END], STDOUT_FILENO);
                close(pipefd[WRITE_END]);
                close(pipefd[READ_END]);
            }
            char **cmd_args = split_command(argv[i + 2 + here_doc * 2]);
            if (!cmd_args || !cmd_args[0])
            {
                ft_putstr_fd("pipex: command not found\n", STDERR_FILENO);
                exit(127);
            }
            char *path = get_cmd_path(cmd_args[0], envp);
            if (!path)
            {
                ft_putstr_fd("pipex: command not found: ", STDERR_FILENO);
                ft_putstr_fd(cmd_args[0], STDERR_FILENO);
                ft_putstr_fd("\n", STDERR_FILENO);
                ft_free_split(cmd_args);
                exit(127);
            }
            execve(path, cmd_args, envp);
            perror("pipex");
            free(path);
            ft_free_split(cmd_args);
            exit(126);
        }
        else
        {
            if (i < num_cmds - 1)
            {
                close(pipefd[WRITE_END]);
                if (prev_pipe_read != in_fd)
                    close(prev_pipe_read);
                prev_pipe_read = pipefd[READ_END];
            }
        }
    }
    close(prev_pipe_read);
    close(out_fd);
    close(in_fd);
    int status;
    int exit_status = 0;
    for (i = 0; i < num_cmds; i++)
    {
        waitpid(child_pids[i], &status, 0);
        if (i == num_cmds - 1)
            exit_status = WEXITSTATUS(status);
    }
    free(child_pids);
    return (exit_status);
}
