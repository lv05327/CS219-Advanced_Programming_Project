#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#define Precision 5 //set the needed precision of divsion

//error handle
void handle_error(const char *message, int interactive)
{
    if (interactive)
    {
        printf("Error: %s\n", message);
    }
    else
    {
        fprintf(stderr, "%s\n", message);
        exit(EXIT_FAILURE);
    }
}

//检查输入的数字是否合法
bool check_number(char *str)
{
    int has_point = 0;
    int i = 0;
    while (str[i] != '\0')
    {
        if (str[i] == '.')
        {
            if (has_point)
            {
                return false;
            }
            has_point = 1;
            if (!isdigit(str[i + 1]))
            {
                return false;
            }
        }
        else if (!isdigit(str[i]))
        {
            if (str[i] == '-' && i == 0)
            {
                i++;
                continue;
            }
            return false;
        }
        i++;
    }
    if ((str[0] == '-' && str[1] == '0' && strlen(str) == 2) || (str[0] == '+' && str[1] == '0' && strlen(str) == 2))
    {
        str++;
    }
    if (str[0] == '-' && strlen(str) == 1)
    {
        return false;
    }
    return true;
}

//检查操作符的合法性
bool check_op(char *str)
{
    if (strlen(str) != 1)
    {
        return false;
    }
    else
    {
        switch (str[0])
        {
        case '+':
            return true;
        case '-':
            return true;
        case 'x':
            return true;
        case '/':
            return true;
        default:
            return false;
        }
    }
    return false;
}
// 反转字符串，用于从低位开始做运算
void reverse(char *str)
{
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++)
    {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

// 补齐小数部分
void pad_decimal(char *num1, char *num2)
{
    int len1 = strlen(num1);
    int len2 = strlen(num2);

    // 找到小数点的位置
    char *dot1 = strchr(num1, '.');
    char *dot2 = strchr(num2, '.');

    // 如果没有小数点，补上 ".0"
    if (!dot1)
    {
        strcat(num1, ".0");
        dot1 = strchr(num1, '.');
    }
    if (!dot2)
    {
        strcat(num2, ".0");
        dot2 = strchr(num2, '.');
    }

    // 补齐小数部分的长度
    int decimal_len1 = strlen(dot1 + 1);
    int decimal_len2 = strlen(dot2 + 1);
    if (decimal_len1 < decimal_len2)
    {
        for (int i = 0; i < decimal_len2 - decimal_len1; i++)
        {
            strcat(num1, "0");
        }
    }
    else if (decimal_len1 > decimal_len2)
    {
        for (int i = 0; i < decimal_len1 - decimal_len2; i++)
        {
            strcat(num2, "0");
        }
    }
}
// 大数加法(整数部分)
void add_integer(char *num1, char *num2, char *result, int *carry)
{
    int len1 = strlen(num1);
    int len2 = strlen(num2);
    int maxLen = len1 > len2 ? len1 : len2;

    reverse(num1);
    reverse(num2);

    for (int i = 0; i < maxLen; i++)
    {
        int digit1 = i < len1 ? num1[i] - '0' : 0;
        int digit2 = i < len2 ? num2[i] - '0' : 0;
        int sum = digit1 + digit2 + *carry;
        result[i] = (sum % 10) + '0';
        *carry = sum / 10;
    }

    if (*carry)
    {
        result[maxLen] = *carry + '0';
        result[maxLen + 1] = '\0';
    }
    else
    {
        result[maxLen] = '\0';
    }

    reverse(result);
}
// 大数加法（小数部分）
void add_decimal(char *num1, char *num2, char *result, int *carry)
{
    int len1 = strlen(num1);
    int len2 = strlen(num2);
    int maxLen = len1 > len2 ? len1 : len2;

    reverse(num1);
    reverse(num2);

    for (int i = 0; i < maxLen; i++)
    {
        int digit1 = i < len1 ? num1[i] - '0' : 0;
        int digit2 = i < len2 ? num2[i] - '0' : 0;
        int sum = digit1 + digit2 + *carry;
        result[i] = (sum % 10) + '0';
        *carry = sum / 10;
    }
    result[maxLen] = '\0';
    reverse(result);
}
// 大数加法
void add(char *num1, char *num2)
{
    if (num1[0] == '-' || num2[0] == '-')
    {
        printf("= -");
        if (num1[0] == '-')
        {
            num1++;
        }
        if (num2[0] == '-')
        {
            num2++;
        }
    }
    else
    {
        printf("=");
    }

    pad_decimal(num1, num2);

    // 移除小数点
    char *dot1 = strchr(num1, '.');
    char *dot2 = strchr(num2, '.');
    *dot1 = *dot2 = '\0';

    // 合并整数部分和小数部分
    char integer1[strlen(num1)], integer2[strlen(num2)];
    char decimal1[strlen(dot1 + 1)], decimal2[strlen(dot2 + 1)];
    strcpy(integer1, num1);
    strcpy(integer2, num2);
    strcpy(decimal1, dot1 + 1);
    strcpy(decimal2, dot2 + 1);

    // 计算小数部分
    char decimal_result[strlen(dot1 + 1) > strlen(dot2 + 1) ? strlen(dot1 + 1) : strlen(dot2 + 1)];
    int carry = 0;
    add_decimal(decimal1, decimal2, decimal_result, &carry);

    // 计算整数部分
    char integer_result[strlen(num1) > strlen(num2) ? strlen(num1) + 1 : strlen(num2) + 1];
    add_integer(integer1, integer2, integer_result, &carry);

    char result[(strlen(dot1 + 1) > strlen(dot2 + 1) ? strlen(dot1 + 1) : strlen(dot2 + 1)) + (strlen(num1) > strlen(num2) ? strlen(num1) + 1 : strlen(num2) + 1) + 1];
    // 合并结果
    sprintf(result, "%s.%s", integer_result, decimal_result);
    printf("%s\n", result);
}

// 大数减法（整数部分）
void subtract_integer(char *num1, char *num2, char *result, int *borrow)
{
    int len1 = strlen(num1);
    int len2 = strlen(num2);
    int maxLen = len1 > len2 ? len1 : len2;

    reverse(num1);
    reverse(num2);

    for (int i = 0; i < maxLen; i++)
    {
        int digit1 = i < len1 ? num1[i] - '0' : 0;
        int digit2 = i < len2 ? num2[i] - '0' : 0;
        int diff = digit1 - digit2 - *borrow;

        if (diff < 0)
        {
            diff += 10;
            *borrow = 1;
        }
        else
        {
            *borrow = 0;
        }

        result[i] = diff + '0';
    }

    result[maxLen] = '\0';
    reverse(result);

    // 移除前导零
    int i = 0;
    while (result[i] == '0' && result[i + 1] != '\0')
    {
        i++;
    }
    memmove(result, result + i, strlen(result) - i + 1);
}
// 大数减法（小数部分）
void subtract_decimal(char *num1, char *num2, char *result, int *borrow)
{
    int len1 = strlen(num1);
    int len2 = strlen(num2);
    int maxLen = len1 > len2 ? len1 : len2;

    reverse(num1);
    reverse(num2);

    for (int i = 0; i < maxLen; i++)
    {
        int digit1 = i < len1 ? num1[i] - '0' : 0;
        int digit2 = i < len2 ? num2[i] - '0' : 0;
        int diff = digit1 - digit2 - *borrow;

        if (diff < 0)
        {
            diff += 10;
            *borrow = 1;
        }
        else
        {
            *borrow = 0;
        }

        result[i] = diff + '0';
    }

    result[maxLen] = '\0';
    reverse(result);

    // 移除前导零
    int i = 0;
    while (result[i] == '0' && result[i + 1] != '\0')
    {
        i++;
    }
    memmove(result, result + i, strlen(result) - i + 1);
}
// 比较两个数的大小(减法)
bool compare_sub(char *integer1, char *integer2, char *decimal1, char *decimal2)
{
    if (strlen(integer1) > strlen(integer2))
    {
        return true;
    }
    else if (strlen(integer1) < strlen(integer2))
    {
        return false;
    }

    // 整数部分长度相同，逐字符比较
    for (int i = 0; i < strlen(integer1); i++)
    {
        if (integer1[i] > integer2[i])
        {
            return true;
        }
        else if (integer1[i] < integer2[i])
        {
            return false;
        }
    }

    // 整数部分相同，比较小数部分
    int decimal_len1 = strlen(decimal1);
    int decimal_len2 = strlen(decimal2);

    for (int i = 0; i < decimal_len1 && i < decimal_len2; i++)
    {
        if (decimal1[i] > decimal2[i])
        {
            return true;
        }
        else if (decimal1[i] < decimal2[i])
        {
            return false;
        }
    }
    // 小数部分也相同
    return true;
}
// 大数减法
void subtract(char *num1, char *num2)
{
    printf("=");
    pad_decimal(num1, num2);

    // 移除小数点
    char *dot1 = strchr(num1, '.');
    char *dot2 = strchr(num2, '.');
    *dot1 = *dot2 = '\0';

    // 合并整数部分和小数部分
    char integer1[strlen(num1)], integer2[strlen(num2)];
    char decimal1[strlen(dot1 + 1)], decimal2[strlen(dot2 + 1)];
    strcpy(integer1, num1);
    strcpy(integer2, num2);
    strcpy(decimal1, dot1 + 1);
    strcpy(decimal2, dot2 + 1);

    // 判断 num1 是否小于 num2
    bool is_negative = false;
    if (!compare_sub(integer1, integer2, decimal1, decimal2))
    {
        is_negative = true;
    }
    // 计算小数部分
    char decimal_result[strlen(dot1 + 1) > strlen(dot2 + 1) ? strlen(dot1 + 1) : strlen(dot2 + 1)];
    char integer_result[strlen(num1) > strlen(num2) ? strlen(num1) : strlen(num2)];
    char result[(strlen(dot1 + 1) > strlen(dot2 + 1) ? strlen(dot1 + 1) : strlen(dot2 + 1)) + (strlen(num1) > strlen(num2) ? strlen(num1) : strlen(num2)) + 1];
    int borrow = 0;
    if (!is_negative)
    {
        subtract_decimal(decimal1, decimal2, decimal_result, &borrow);
        subtract_integer(integer1, integer2, integer_result, &borrow);
        sprintf(result, "%s.%s", integer_result, decimal_result);
    }
    else
    {
        subtract_decimal(decimal2, decimal1, decimal_result, &borrow);
        subtract_integer(integer2, integer1, integer_result, &borrow);
        sprintf(result, "-%s.%s", integer_result, decimal_result);
    }
    printf("%s\n", result);
}

// 大数乘法(统一化为整数)
void multiply_integer(char *num1, char *num2, char *result)
{
    int len1 = strlen(num1);
    int len2 = strlen(num2);
    int *tempResult = (int *)calloc(len1 + len2, sizeof(int));

    reverse(num1);
    reverse(num2);

    for (int i = 0; i < len1; i++)
    {
        for (int j = 0; j < len2; j++)
        {
            tempResult[i + j] += (num1[i] - '0') * (num2[j] - '0');
            tempResult[i + j + 1] += tempResult[i + j] / 10;
            tempResult[i + j] %= 10;
        }
    }

    int resultLen = len1 + len2;
    while (resultLen > 1 && tempResult[resultLen - 1] == 0)
    {
        resultLen--;
    }

    for (int i = 0; i < resultLen; i++)
    {
        result[i] = tempResult[i] + '0';
    }
    result[resultLen] = '\0';

    reverse(result);
    free(tempResult);
}
// 大数乘法（支持小数）
void multiply(char *num1, char *num2)
{
    int t = 0;
    if (num1[0] == '-')
    {
        t++;
        num1++;
    }
    if (num2[0] == '-')
    {
        t++;
        num2++;
    }
    if (t == 0 || t == 2)
    {
        printf("=");
    }
    else
    {
        printf("= -");
    }

    pad_decimal(num1, num2);

    // 移除小数点
    char *dot1 = strchr(num1, '.');
    char *dot2 = strchr(num2, '.');
    *dot1 = *dot2 = '\0';

    // 合并整数部分和小数部分
    char integer1[strlen(num1)], integer2[strlen(num2)];
    char decimal1[strlen(dot1 + 1)], decimal2[strlen(dot2 + 1)];
    strcpy(integer1, num1);
    strcpy(integer2, num2);
    strcpy(decimal1, dot1 + 1);
    strcpy(decimal2, dot2 + 1);

    // 计算总的小数位数
    int decimal_places = strlen(decimal1) + strlen(decimal2);

    // 合并整数和小数部分
    char full_num1[strlen(num1) + strlen(dot1 + 1)], full_num2[strlen(num2) + strlen(dot2 + 1)];
    sprintf(full_num1, "%s%s", integer1, decimal1);
    sprintf(full_num2, "%s%s", integer2, decimal2);

    // 计算乘法
    char result[strlen(num1) + strlen(dot1 + 1) + strlen(num2) + strlen(dot2 + 1) + 1];
    char temp_result[strlen(num1) + strlen(dot1 + 1) + strlen(num2) + strlen(dot2 + 1)];
    multiply_integer(full_num1, full_num2, temp_result);

    // 插入小数点
    int len = strlen(temp_result);
    memmove(temp_result + len - decimal_places + 1, temp_result + len - decimal_places, decimal_places + 1);
    temp_result[len - decimal_places] = '.';
    strcpy(result, temp_result);
    printf("%s\n", result);
}

// 比较两个数的大小(除法)
bool compare_devi(char integer1[], char integer2[])
{
    if (strlen(integer1) > strlen(integer2))
    {
        return true;
    }
    else if (strlen(integer1) < strlen(integer2))
    {
        return false;
    }

    // 整数部分长度相同，逐字符比较
    for (int i = 0; i < strlen(integer1); i++)
    {
        if (integer1[i] > integer2[i])
        {
            return true;
        }
        else if (integer1[i] < integer2[i])
        {
            return false;
        }
    }
    return true;
}

// 减法
void subtract_for_divide(char *num1, char *num2)
{
    int l = strlen(num2);
    int borrow = 0;
    for (int i = l - 1; i >= 0; i--)
    {
        int digit1 = num1[i] - '0';
        int digit2 = num2[i] - '0';
        int diff = digit1 - digit2 - borrow;

        if (diff < 0)
        {
            diff += 10;
            borrow = 1;
        }
        else
        {
            borrow = 0;
        }

        num1[i] = diff + '0';
    }
}
// 去除前导零
void remove_leading_zeros(char *result)
{
    int i = 0;
    int len = strlen(result);

    // 找到第一个非零字符的位置
    while (i < len && result[i] == '0')
    {
        i++;
    }

    // 如果全部是零，保留一个零
    if (i == len)
    {
        result[0] = '0';
        result[1] = '\0';
        return;
    }

    // 如果遇到小数点，则保留一个零
    if (result[i] == '.')
    {
        i--;
    }

    // 移除前导零
    if (i > 0)
    {
        memmove(result, result + i, len - i + 1);
    }
}
// 大数除法
void devide(char *num1, char *num2)
{
    int t = 0;
    if (num1[0] == '-')
    {
        t++;
        num1++;
    }
    if (num2[0] == '-')
    {
        t++;
        num2++;
    }
    if (t == 0 || t == 2)
    {
        printf("=");
    }
    else
    {
        printf("= -");
    }

    if (strcmp(num1, num2) == 0)
    {
        char r[7];
        r[0] = '1';
        r[1] = '.';
        for (size_t i = 2; i < Precision + 2; i++)
        {
            r[i] = '0';
        }
        printf("%s\n", r);
    }
    else
    {
        pad_decimal(num1, num2);

        // 移除小数点
        char *dot1 = strchr(num1, '.');
        char *dot2 = strchr(num2, '.');
        *dot1 = *dot2 = '\0';

        // 合并整数部分和小数部分
        char integer1[strlen(num1)], integer2[strlen(num2)];
        char decimal1[strlen(dot1 + 1)], decimal2[strlen(dot2 + 1)];
        strcpy(integer1, num1);
        strcpy(integer2, num2);
        strcpy(decimal1, dot1 + 1);
        strcpy(decimal2, dot2 + 1);

        // 合并整数和小数部分
        char full_num1[strlen(num1) + strlen(dot1 + 1) + Precision + 1];
        char full_num2[strlen(num2) + strlen(dot2 + 1) + 1];
        sprintf(full_num1, "%s%s", integer1, decimal1);
        sprintf(full_num2, "0%s%s", integer2, decimal2);
        for (int i = strlen(num1) + strlen(dot1 + 1); i < strlen(num1) + strlen(dot1 + 1) + Precision; i++)
        {
            full_num1[i] = '0';
        }
        full_num1[strlen(num1) + strlen(dot1 + 1) + Precision] = '\0';
        // 除法
        char temp[strlen(num2) + strlen(dot2 + 1) + 2];
        for (int i = 0; i < strlen(num2) + strlen(dot2 + 1) + 1; i++)
        {
            temp[i] = '0';
        }
        temp[strlen(num2) + strlen(dot2 + 1) + 1] = '\0';
        char result[strlen(num1) + strlen(dot1 + 1) + Precision + strlen(num2) + strlen(dot2 + 1)];
        int k = 0;
        int x = 0;
        int y = strlen(full_num2) - 1;
        int q = 0;
        bool afterbool = false;
        if (!compare_sub(integer1, integer2, decimal1, decimal2))
        {
            result[q++] = '0';
            result[q++] = '.';
            afterbool = true;
            int u = strlen(full_num2) - strlen(integer1) - strlen(decimal1) - 1;
            for (int i = 0; i < u && k < Precision; i++)
            {
                result[q++] = '0';
                k++;
            }
        }
        while (k < Precision)
        {
            int p = 0;
            if (full_num1[x] == '0')
            {
                x++;
            }
            int j = strlen(temp) - 1;
            for (int i = y; i >= x; i--)
            {
                temp[j--] = full_num1[i];
            }
            if (compare_devi(temp, full_num2))
            {
                while (compare_devi(temp, full_num2))
                {
                    subtract_for_divide(temp, full_num2);
                    p++;
                }
                int r = 0;
                for (int i = x; i <= y; i++)
                {
                    full_num1[i] = temp[r++];
                }
                if (p >= 10)
                {
                    int n = p / 10;
                    int g = p % 10;
                    result[q++] = n + '0';
                    result[q++] = g + '0';
                }
                else
                {
                    result[q++] = p + '0';
                }
                if (afterbool)
                {
                    k++;
                }
                y++;
                if (y == strlen(num1) + strlen(dot1 + 1))
                {
                    result[q++] = '.';
                    afterbool = true;
                }
            }
            else
            {
                y++;
                if (y == strlen(num1) + strlen(dot1 + 1))
                {
                    result[q++] = '0';
                    result[q++] = '.';
                    afterbool = true;
                }
                else
                {
                    result[q++] = '0';
                    if (afterbool)
                    {
                        k++;
                    }
                }
            }
        }
        result[q] = '\0';
        remove_leading_zeros(result);
        printf("%s\n", result);
    }
}

/* 执行基本运算 */
void calculate(char *a, char op, char *b, int interactive)
{
    // 预处理解决负数加减法问题
    if (op == '+')
    {
        if (a[0] != '-' && b[0] == '-')
        {
            b++;
            op = '-';
        }
        if (a[0] == '-' && b[0] != '-')
        {
            a++;
            char *temp;
            temp = a;
            a = b;
            b = temp;
            op = '-';
        }
    }
    if (op == '-')
    {
        if (a[0] == '-' && b[0] != '-')
        {
            op = '+';
        }
        if (a[0] != '-' && b[0] == '-')
        {
            op = '+';
            b++;
        }
        if (a[0] == '-' && b[0] == '-')
        {
            a++;
            b++;
            char *temp;
            temp = a;
            a = b;
            b = temp;
        }
    }
    switch (op)
    {
    case '+':
        add(a, b);
        break;
    case '-':
        subtract(a, b);
        break;
    case 'x':
        multiply(a, b);
        break;
    case '/':
        if (b[0] == '0')
        {
            printf("A number cannot be divided by zero.\n");
            break;
        }
        devide(a, b);
        break;
    }
}

/*命令行模式处理 */
void command_line_mode(int argc, char *argv[])
{
    if (argc != 4)
    {
        handle_error("Usage: ./calculator [num1] [op] [num2]", 0);
    }
    if (!check_number(argv[1]) || !check_number(argv[3]))
    {
        handle_error("number invalid!", 0);
    }
    if (!check_op(argv[2]))
    {
        handle_error("operator invalid!", 0);
    }

    char *a = argv[1];
    char op = argv[2][0];
    char *b = argv[3];

    char a_real[sizeof(a) + sizeof(b)];
    char b_real[sizeof(a) + sizeof(b)];
    strcpy(a_real, a);
    strcpy(b_real, b);

    calculate(a, op, b, 0);
}

// 动态读取输入
char *read_input()
{
    size_t size = 256;
    char *input = (char *)malloc(size * sizeof(char));
    if (!input)
    {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    size_t len = 0;
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        input[len++] = ch;

        // 如果缓冲区已满，重新分配更大的内存
        if (len == size - 1)
        {
            size *= 2;
            input = (char *)realloc(input, size * sizeof(char));
            if (!input)
            {
                perror("Failed to reallocate memory");
                exit(EXIT_FAILURE);
            }
        }
    }
    input[len] = '\0'; // 添加字符串结束符
    return input;
}

void interactive_mode()
{
    printf("Interactive Calculator Mode (type 'quit' to exit)\n");

    while (1)
    {
        printf("> ");
        char *input = read_input();

        if (strcmp(input, "quit") == 0)
        {
            free(input);
            break;
        }

        // 解析普通表达式
        char *tokens[3];
        int token_count = 0;
        char *token = strtok(input, " ");

        while (token && token_count < 3)
        {
            tokens[token_count++] = token;
            token = strtok(NULL, " ");
        }

        if (token_count != 3)
        {
            handle_error("Invalid expression format! Use: num op num", 1);
            free(input); // 释放内存
            continue;
        }
        if (!check_number(tokens[0]) || !check_number(tokens[2]))
        {
            handle_error("number invalid!", 1);
        }
        if (!check_op(tokens[1]))
        {
            handle_error("operator invalid!", 1);
        }

        char *a;
        char op;
        char *b;

        a = tokens[0];
        op = tokens[1][0];
        b = tokens[2];

        char a_real[sizeof(a) + sizeof(b)];
        char b_real[sizeof(a) + sizeof(b)];
        strcpy(a_real, a);
        strcpy(b_real, b);
        calculate(a_real, op, b_real, 1);

        free(input); // 释放内存
    }
}
int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        interactive_mode();
    }
    else
    {
        command_line_mode(argc, argv);
    }
    return 0;
}
