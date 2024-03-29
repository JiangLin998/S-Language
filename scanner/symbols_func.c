/**
  ******************************************************************************
  * @file           : symbols_func.c
  * @author         : Troy
  * @brief          : None
  * @attention      : None
  * @date           : 2024/3/11
  ******************************************************************************
  */
#include "scanner.h"
#include "utils.h"

#define TAG                 "[symbols_func]"

#define SYMBOL_GENERAL_METHOD(func_name, token_name)       \
static int _symbol_##func_name##_func(scanner_t* scanner){ \
    scanner->cur_token.type = TOKEN_##token_name;           \
    scanner->cur_token.len = 1;                             \
    return 1;                                               \
}


#define SYMBOL_DOUBLE_GENERAL_METHOD(func_name, next_char, token_name)       \
static int _symbol_##func_name##_func(scanner_t* scanner){              \
    if(next_char != ' ' && _match_next_char(scanner, next_char)) {     \
        scanner->cur_token.type = TOKEN_##token_name##_EQUAL;           \
        scanner->cur_token.len = 2;                         \
    }else {                                                 \
        scanner->cur_token.type = TOKEN_##token_name;       \
        scanner->cur_token.len = 1;                         \
    }                                                       \
    return 1;                                               \
}
SYMBOL_DOUBLE_GENERAL_METHOD(add, '=', ADD)
SYMBOL_DOUBLE_GENERAL_METHOD(sub, '=', SUB)
SYMBOL_DOUBLE_GENERAL_METHOD(mul, '=', MUL)
SYMBOL_DOUBLE_GENERAL_METHOD(mod, '=', MOD)
SYMBOL_DOUBLE_GENERAL_METHOD(not, '=', NOT)
SYMBOL_DOUBLE_GENERAL_METHOD(less, '=', LESS)
SYMBOL_DOUBLE_GENERAL_METHOD(greater, '=', GREATER)
SYMBOL_DOUBLE_GENERAL_METHOD(equal, '=', EQUAL)
SYMBOL_GENERAL_METHOD(comma, COMMA)
SYMBOL_GENERAL_METHOD(semicolon, SEMICOLON)
SYMBOL_GENERAL_METHOD(left_paren, LEFT_PAREN)
SYMBOL_GENERAL_METHOD(right_paren, RIGHT_PAREN)
SYMBOL_GENERAL_METHOD(left_bracket, LEFT_BRACKET)
SYMBOL_GENERAL_METHOD(right_bracket, RIGHT_BRACKET)
SYMBOL_GENERAL_METHOD(or, LOGIC_ORR)
SYMBOL_GENERAL_METHOD(and, LOGIC_AND)
/**
 * @brief 对于/以及以/开头的符号进行处理
 * @param scanner 分词器指针
 * @return 作为token返回1 不作为token返回0
 */
static int _symbol_div_func(scanner_t* scanner) {
    if(_match_next_char(scanner, '=')) {
        scanner->cur_token.type = TOKEN_DIV_EQUAL;
        scanner->cur_token.len = 2;
        return 1;
    }else if(_match_next_char(scanner, '*')) {
        _get_next_char(scanner);
        /* 查找注释对 */
        while(scanner->cur_char != '*') {
            if(scanner->cur_char == '\n') {
                scanner->cur_token.line_num ++;
            }
            _get_next_char(scanner);
        }
        /* 如果仅存在星号不存在斜杠则报错注释对不全 */
        if(!_match_next_char(scanner, '/')) {
            SCANNER_REPORT("error",
                         scanner,
                         "缺少对应的注释对");
            _get_next_char(scanner);
        }else{
            /* 跳过'/' */
            _get_next_char(scanner);
            _skip_blanks(scanner);
            scanner->cur_token.start = scanner->next_char_ptr - 1;
        }
        return 0;
    }else if(_match_next_char(scanner, '/')) {
        _skip_line(scanner);
        scanner->cur_token.start = scanner->next_char_ptr - 1;
        return 0;
    }
    scanner->cur_token.type = TOKEN_DIV;
    return 1;
}

/**
 * @brief 换行解析
 * @param scanner 分词器指针
 * @return 默认返回0跳过该字符 不作为token
 */
static int _symbol_enter_func(scanner_t* scanner){
    scanner->cur_token.line_num ++;
    _get_next_char(scanner);
    scanner->cur_token.start = scanner->next_char_ptr - 1;
    return 0;
}
