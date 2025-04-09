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
