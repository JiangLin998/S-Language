/**
  ******************************************************************************
  * @file           : flood.c
  * @author         : Troy
  * @brief          : None
  * @attention      : None
  * @date           : 2024/3/11
  ******************************************************************************
  */
#include <ctype.h>
#include "scanner.h"

#define TAG                 "[flood]"

/**
 * @brief ��ȡ��һ���ַ����������·ִ���
 * @param scanner �ִ���ָ��
 * @return
 */
static char _look_next_char(scanner_t* scanner){
    return *scanner->next_char_ptr;
}
/**
 * @brief ��ȡ��һ���ַ����Ҳ����·ִ���
 * @param scanner �ִ���ָ��
 * @return
 */
static void _get_next_char(scanner_t* scanner) {
    scanner->cur_char = *scanner->next_char_ptr++;
}
/**
 * @brief ƥ����һ���ַ�
 * @param scanner �ִ���ָ��
 * @param next_char ��������һ���ַ�
 * @return ƥ��ɹ�����1 ƥ��ʧ�ܷ���0
 */
static char _match_next_char(scanner_t* scanner, char next_char){
    if(_look_next_char(scanner) == next_char){
        _get_next_char(scanner);
        return 1;
    }
    return 0;
}
/**
 * @brief ����һ�У���Ҫ����//ע���ж�
 * @param scanner �ִ���ָ��
 */
static void _skip_line(scanner_t* scanner) {
    while(scanner->cur_char != '\n') {
        _get_next_char(scanner);
    }
    /* ����\n */
    _get_next_char(scanner);
    scanner->cur_token.line_num ++;
}
/**
 * @brief �����ո�
 * @param scanner �ִ���ָ��
 */
static void _skip_blanks(scanner_t* scanner) {
    while(isspace(scanner->cur_char)){
        if(scanner->cur_char == '\n'){
            scanner->cur_token.line_num ++;
        }
        _get_next_char(scanner);
    }
}

#include "symbols.inc"
#include "keywords.inc"
/**
 * @brief �����û���ʶ����ؼ���
 * @param scanner �ִ���ָ��
 * @param type token�����ͣ���ȷ��token������дTOKEN_UNKNOWN
 */
static void _parse_id_or_keyword(scanner_t* scanner, token_type_t type) {
    while(isalnum(scanner->cur_char) || scanner->cur_char == '_') {
        _get_next_char(scanner);
    }

    uint32_t _len = (uint32_t)(scanner->next_char_ptr - scanner->cur_token.start - 1);
    if(type != TOKEN_UNKNOWN) {
        scanner->cur_token.type = type;
    }else {
        uint32_t _i, _keyword_num = sizeof(keywords) / sizeof(keywords[0]);
        for(_i = 0; _i < _keyword_num; _i++) {
            if(_len == keywords[_i].len &&
                memcmp(keywords[_i].keyword, scanner->cur_token.start, _len) == 0){
                break;
            }
        }
        if(_i == _keyword_num) {
            scanner->cur_token.type = TOKEN_ID;
        }else {
            scanner->cur_token.type = keywords[_i].type;
        }
        scanner->cur_token.len = _len;
    }
}
/**
 * @brief ��������
 * @param scanner �ִ���ָ��
 */
static void _parse_number(scanner_t* scanner){
    while(isdigit(scanner->cur_char)) {
        _get_next_char(scanner);
    }
    if(scanner->cur_char == '.') {
        _get_next_char(scanner);
        while(isdigit(scanner->cur_char)) {
            _get_next_char(scanner);
        }
    }
    scanner->cur_token.len = (uint32_t)(scanner->next_char_ptr - scanner->cur_token.start - 1);
    scanner->cur_token.type = TOKEN_NUM;
}
/**
 * @brief ��������
 * @param scanner �ִ���ָ��
 * @return
 */
static int _parse_symbol(scanner_t* scanner) {
    int _symbol_size = sizeof(symbols) / sizeof(symbols[0]);
    int _i;
    for(_i = 0; _i < _symbol_size; _i++) {
        if(scanner->cur_char == symbols[_i].symbol){
            break;
        }
    }
    if(_i == _symbol_size) {
        SCANNER_REPORT("error",
                     scanner,
                     "�ַ�%c��֧��",
                     scanner->cur_char);
        _get_next_char(scanner);
        scanner->get_next_token_init(scanner);
        return 0;
    }else {
        return symbols[_i].func(scanner);
    }
}



/**
 * @brief ��ȡ��һ��token
 * @param arg �ִ���ָ��
 */
void flood_get_next_token(void* arg) {
    scanner_t* scanner = (scanner_t*)arg;
    scanner->get_next_token_init(arg);
    while(scanner->cur_char != '\0') {
        if(scanner->cur_char == '_' || isalpha(scanner->cur_char)) {
            _parse_id_or_keyword(scanner, TOKEN_UNKNOWN);
            return;
        }else if(isdigit(scanner->cur_char)) {
            /* ��������� */
            _parse_number(scanner);
            return;
        }else {
            if(_parse_symbol(scanner) == 0) {
                continue;
            }
        }
        scanner->cur_token.len = (uint32_t)(scanner->next_char_ptr - scanner->cur_token.start);
        _get_next_char(scanner);
        return;
    }
}