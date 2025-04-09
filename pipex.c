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
