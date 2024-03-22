/**
  ******************************************************************************
  * @file           : scanner.c
  * @author         : Troy
  * @brief          : None
  * @attention      : None
  * @date           : 2024/3/11
  ******************************************************************************
  */
#include <ctype.h>
#include "scanner.h"


#define TAG                 "[scanner]"

/* ����flood�Ļ�ȡ��һ��token���� */
extern void flood_get_next_token(void* scanner);

/* �ִ���ģʽ�ַ������� */
static const char* scanner_mode_str[] = {
    "flood",
//    "dfa"
};

/* �ִ�����ȡ��һ��token�ĺ�����Ӧ����scanner_mode_str��˳��һ�� */
static scanner_get_next_token_t get_next_token_func[] = {
    flood_get_next_token,
};

#ifdef ALL_STEPS_INDEPENDENCE

/**
 * @brief ��token�������
 * @param scanner
 * @note �ú��������token�������Զ�ѡ���ʶ������߳�����
 *       from __insert_table����� �����Լ��Ľ��� ���ù���
 */
static void __insert_table(scanner_t* scanner) {
    char buf[128] = {0};
    token_t *token = &scanner->cur_token;
    /* buf�洢��token���ַ��� */
    memcpy(buf, token->start, token->len);
    if(token->type == TOKEN_ID) {
        /* ����������û���ʶ���Ļ� ��洢����ʶ�������� */
        if(bsst_search(scanner->user_id_table_tree, buf) == NULL) {
            user_id_table_t *temp = (user_id_table_t*)malloc(sizeof(user_id_table_t));
            ASSERT(temp != NULL, "�����ʱ�����ڵ�ʧ��");
            bsst_insert(&scanner->user_id_table_tree, buf, &temp);
        }
    } else if(token->type == TOKEN_NUM) {
        /* ��������ǳ����Ļ� ��洢������������ */
        if(bsst_search(scanner->constant_table_tree, buf) == NULL) {
            constant_table_t *temp = (constant_table_t*)malloc(sizeof(constant_table_t));
            ASSERT(temp != NULL, "�����ʱ�����ڵ�ʧ��");
            bsst_insert(&scanner->constant_table_tree, buf, &temp);
        }
    }
}
/**
 * @brief ����tokenΪ��Ԫʽ
 * @param token ��Ҫ������token
 */
static void __export_token(token_t* token) {
    char buf1[128] = {0}, buf2[128] = {0};
    /* buf2�洢��token���ַ��� */
    memcpy(buf2, token->start, token->len);
    /* ����Ϊ������Ԫʽ��buf1 */
    sprintf(buf1, "(");
    if(token->type >= TOKEN_INT && token->type <= TOKEN_END) {
        sprintf(buf1, "%s%s,-)", buf1, buf2);
    } else if(token->type == TOKEN_ID) {
        sprintf(buf1, "%sid,%s)", buf1, buf2);
    } else if(token->type == TOKEN_NUM) {
        sprintf(buf1, "%snum,%s)", buf1, buf2);
    } else {
        sprintf(buf1, "%s%s,-)", buf1, buf2);
    }
    /* ׷��д���Ԫʽ�����ļ�����
     * ��ÿ�ηִ��������׶Σ����������Ŀ¼�������ļ�
     * ���Կ���ֱ����׷�ӵ���ʽ */
    utils_write_file("../build/scanner/test.txt", buf1, "a");
}
/**
 * @brief ��ȡ����token
 * @param arg �ִ���ָ��
 */
static void _get_all_token(void* arg) {
    scanner_t* scanner = (scanner_t*)arg;
    /* ���Ȼ�ȡһ��token�ж��ǲ���eof���͵ģ���ֹ�ļ�Ϊ�� */
    scanner->get_next_token(scanner);
    while(scanner->cur_token.type != TOKEN_EOF){
        char buf[256] = {0};
        /* ��tokenָ���ַ������Ƶ�buf���� */
        memcpy(buf, scanner->cur_token.start, scanner->cur_token.len);
        S_LOGD(TAG, "%d %s", scanner->cur_token.type, buf);
        /* �˴�Ӧ�����ɱ� �ҵ�����Ԫʽ */
        __insert_table(scanner);
        __export_token(&scanner->cur_token);
        scanner->get_next_token(scanner);
    }
    /* ����һ�¶����� �����ɵı��ӡ���� */
    printf("---------------�û���ʶ����---------------\n");
    bsst_inorder(scanner->user_id_table_tree);
    printf("----------------------------------------\n");
    printf("------------------������-----------------\n");
    bsst_inorder(scanner->constant_table_tree);
    printf("----------------------------------------\n");
    printf("%d errors, %d warnings", scanner->errors_num, scanner->warnings_num);
}
#endif

static void _get_next_token_init(void* arg) {
    scanner_t *scanner = (scanner_t*)arg;
    token_t* token = &scanner->cur_token;
    /* �����ո� */
    while(isspace(scanner->cur_char)){
        if(scanner->cur_char == '\n'){
            scanner->cur_token.line_num ++;
        }
        scanner->cur_char = *scanner->next_char_ptr++;
    }
    /* ��token���浽��һ��token֮��Ϳ��Կ�ʼ��ȡ��һ��token�� */
    scanner->prev_token = *token;
    token->type = TOKEN_EOF;
    /* ����token����ʼ��ַ */
    token->start = scanner->next_char_ptr - 1;
    token->len = 0;
}

/**
 * @brief �����ִ���
 * @param scanner_mode ѡ��ִ���ģʽ��֧��ģʽ��scanner_mode_str����
 * @param file_path ��Ҫ�ִʵ��ļ�Ŀ¼
 * @return �ִ���ָ��
 */
scanner_t* scanner_new(const char* scanner_mode, const char* file_path) {
    scanner_t* scanner = malloc(sizeof(scanner_t));
    if(!scanner){
        return NULL;
    }
    /* ������Ҫ�ִʵ��ļ�·�� */
    scanner->file_path = file_path;
    /* ��ȡԴ���� */
    utils_read_file(file_path, &scanner->source_code);
    /* �����ȡʧ�� */
    if(scanner->source_code == NULL){
        S_LOGE(TAG, "Memory request failed");
        /* �������ͷŷִ��� ��ֹ�ڴ�й¶ */
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
    /* ���û�ȡ��һ��token�ĺ��� */
    if(scanner_mode == NULL){
        /* Ĭ��ģʽΪfloodģʽ */
        scanner->get_next_token = get_next_token_func[0];
    }else{
        uint8_t _i, _scanner_mode_num = sizeof(scanner_mode_str) / sizeof(scanner_mode_str[0]);
        for(_i = 0; _i < _scanner_mode_num; _i++) {
            if(strcmp(scanner_mode_str[_i], scanner_mode) == 0){
                break;
            }
        }
        if(_i == _scanner_mode_num){
            /* δƥ�䵽��ģʽ Ĭ��ģʽ0 Ҳ����floodģʽ*/
            scanner->get_next_token = get_next_token_func[0];
        }else {
            /* ƥ�䵽ģʽ�� �趨Ϊ��ģʽ��Ӧ�ĺ��� */
            scanner->get_next_token = get_next_token_func[_i];
        }
    }
    scanner->get_next_token_init = _get_next_token_init;
#ifdef ALL_STEPS_INDEPENDENCE
    scanner->get_all_token = _get_all_token;
#endif

    return scanner;
}
/**
 * @brief ɾ���ִ���
 * @param scanner �ִ���ָ��
 */
void scanner_delete(scanner_t* scanner) {
    free(scanner->source_code);
    free(scanner);
#ifdef ALL_STEPS_INDEPENDENCE
    bsst_destory(scanner->user_id_table_tree, NULL);
    bsst_destory(scanner->constant_table_tree, NULL);
#endif
}