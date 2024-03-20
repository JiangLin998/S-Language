/**
  ******************************************************************************
  * @file           : scanner.c
  * @author         : Troy
  * @brief          : None
  * @attention      : None
  * @date           : 2024/3/11
  ******************************************************************************
  */
#include "scanner.h"

#define TAG                 "[scanner]"

extern void flood_get_next_token(void* scanner);

static const char* scanner_mode_str[] = {
    "flood",
//    "dfa"
};

static scanner_get_next_token_t get_next_token_func[] = {
    flood_get_next_token,
};

#ifdef ALL_STEPS_INDEPENDENCE
/**
 * @brief 获取所有token
 * @param arg 分词器指针
 */
static void _get_all_token(void* arg) {
    scanner_t* scanner = (scanner_t*)arg;
    scanner->get_next_token(scanner);
    while(scanner->cur_token.type != TOKEN_EOF){
        char buf[256] = {0};
        memcpy(buf, scanner->cur_token.start, scanner->cur_token.len);
        printf("%d %s\n", scanner->cur_token.type, buf);
        scanner->get_next_token(scanner);
    }
}
#endif
/**
 * @brief 创建分词器
 * @param scanner_mode 选择分词器模式，支持模式见scanner_mode_str数组
 * @param file_path 需要分词的文件目录
 * @return 分词器指针
 */
scanner_t* scanner_new(const char* scanner_mode, const char* file_path) {
    scanner_t* scanner = malloc(sizeof(scanner_t));
    if(!scanner){
        return NULL;
    }

    scanner->file_path = file_path;
    utils_read_file(file_path, &scanner->source_code);
    if(scanner->source_code == NULL){
        S_LOGE(TAG, "Memory request failed");
        free(scanner);
        return NULL;
    }
    S_LOGI(TAG, "\n%s", scanner->source_code);
    scanner->cur_char = *scanner->source_code;
    scanner->next_char_ptr = scanner->source_code + 1;
    scanner->cur_token.line_num = 1;
    scanner->cur_token.type = TOKEN_UNKNOWN;
    scanner->cur_token.start = NULL;
    scanner->cur_token.len = 0;
    scanner->prev_token = scanner->cur_token;
    /* 设置获取下一个token的函数 */
    if(scanner_mode == NULL){
        /* 默认模式为flood模式 */
        scanner->get_next_token = get_next_token_func[0];
    }else{
        uint8_t _i, _scanner_mode_num = sizeof(scanner_mode_str) / sizeof(scanner_mode_str[0]);
        for(_i = 0; _i < _scanner_mode_num; _i++) {
            if(strcmp(scanner_mode_str[_i], scanner_mode) == 0){
                break;
            }
        }
        /* 未匹配到的模式 */
        if(_i == _scanner_mode_num){
            scanner->get_next_token = get_next_token_func[0];
        }else {
            scanner->get_next_token = get_next_token_func[_i];
        }
    }
#ifdef ALL_STEPS_INDEPENDENCE
    scanner->get_all_token = _get_all_token;
#endif

    return scanner;
}
/**
 * @brief 删除分词器
 * @param scanner 分词器指针
 */
void scanner_delete(scanner_t* scanner) {
    free(scanner->source_code);
    free(scanner);
}