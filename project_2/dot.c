#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    INT,
    FLOAT,
    DOUBLE,
    SHORT,
    SIGNEDCHAR,
    UNKNOWN
} DataType;

// 获取数据类型
DataType getDataType(const char *datatype)
{
    if (strcmp(datatype, "int") == 0)
        return INT;
    if (strcmp(datatype, "float") == 0)
        return FLOAT;
    if (strcmp(datatype, "double") == 0)
        return DOUBLE;
    if (strcmp(datatype, "short") == 0)
        return SHORT;
    if (strcmp(datatype, "signed char") == 0)
        return SIGNEDCHAR;
    return UNKNOWN;
}

int main()
{
    char filename[256];
    scanf("%s", filename);

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Error opening file!\n");
        return 1;
    }

    // 读取数据类型
    char datatype[50];
    if (!fgets(datatype, sizeof(datatype), file))
    {
        printf("Error reading datatype!\n");
        fclose(file);
        return 1;
    }
    // 去掉换行符
    datatype[strcspn(datatype, "\n")] = '\0';

    // 读取向量大小
    int size;
    if (fscanf(file, "%d\n", &size) != 1)
    {
        printf("Error reading vector size!\n");
        fclose(file);
        return 1;
    }

    // 读取两行向量
    char line1[1024], line2[1024];
    if (!fgets(line1, sizeof(line1), file) || !fgets(line2, sizeof(line2), file))
    {
        printf("Error reading vectors!\n");
        fclose(file);
        return 1;
    }
    fclose(file);

    // 去掉换行符
    line1[strcspn(line1, "\n")] = '\0';
    line2[strcspn(line2, "\n")] = '\0';

    // 根据数据类型处理
    switch (getDataType(datatype))
    {
    case INT:
    case SHORT:
    {
        long *vector1 = (long *)malloc(size * sizeof(long));
        long *vector2 = (long *)malloc(size * sizeof(long));
        if (!vector1 || !vector2)
        {
            printf("Memory allocation failed!\n");
            free(vector1);
            free(vector2);
            return 1;
        }

        // 解析向量
        char *token;
        token = strtok(line1, ",");
        for (int i = 0; i < size && token; i++)
        {
            vector1[i] = strtol(token, NULL, 10);
            token = strtok(NULL, ",");
        }
        token = strtok(line2, ",");
        for (int i = 0; i < size && token; i++)
        {
            vector2[i] = strtol(token, NULL, 10);
            token = strtok(NULL, ",");
        }

        // 计算点积
        long dot_product = 0;
        for (int i = 0; i < size; i++)
        {
            dot_product += vector1[i] * vector2[i];
        }
        printf("Dot product: %ld\n", dot_product);

        free(vector1);
        free(vector2);
        break;
    }
    case FLOAT:
    case DOUBLE:
    {
        double *vector1 = (double *)malloc(size * sizeof(double));
        double *vector2 = (double *)malloc(size * sizeof(double));
        if (!vector1 || !vector2)
        {
            printf("Memory allocation failed!\n");
            free(vector1);
            free(vector2);
            return 1;
        }

        // 解析向量
        char *token;
        token = strtok(line1, ",");
        for (int i = 0; i < size && token; i++)
        {
            vector1[i] = strtod(token, NULL);
            token = strtok(NULL, ",");
        }
        token = strtok(line2, ",");
        for (int i = 0; i < size && token; i++)
        {
            vector2[i] = strtod(token, NULL);
            token = strtok(NULL, ",");
        }

        // 计算点积
        double dot_product = 0;
        for (int i = 0; i < size; i++)
        {
            dot_product += vector1[i] * vector2[i];
        }
        printf("Dot product: %.2f\n", dot_product);

        free(vector1);
        free(vector2);
        break;
    }
    case SIGNEDCHAR:
    {
        long *vector1 = (long *)malloc(size * sizeof(long));
        long *vector2 = (long *)malloc(size * sizeof(long));
        if (!vector1 || !vector2)
        {
            printf("Memory allocation failed!\n");
            free(vector1);
            free(vector2);
            return 1;
        }

        // 解析向量1
        char temp_char;
        char *token = strtok(line1, ",");
        for (int i = 0; i < size && token; i++)
        {
            sscanf(token, " '%c'", &temp_char); // 提取单引号中的字符
            vector1[i] = (long)temp_char;       // 将字符转换为 ASCII 值并存储为 long
            token = strtok(NULL, ",");
        }

        // 解析向量2
        token = strtok(line2, ",");
        for (int i = 0; i < size && token; i++)
        {
            sscanf(token, " '%c'", &temp_char); // 提取单引号中的字符
            vector2[i] = (long)temp_char;       // 将字符转换为 ASCII 值并存储为 long
            token = strtok(NULL, ",");
        }

        // 计算点积
        int dot_product = 0;
        for (int i = 0; i < size; i++)
        {
            dot_product += vector1[i] * vector2[i];
        }
        printf("Dot product: %d\n", dot_product);

        free(vector1);
        free(vector2);
        break;
    }
    default:
        printf("Unsupported data type: %s\n", datatype);
        return 1;
    }

    return 0;
}