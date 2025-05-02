#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#define MATRIX_SIZE 3 
#define MAX_KEYS 10

void list_stored_keys();
char* inputword_process(char input[]);
void encrypt();  
void decrypt();  
int find_key_by_code(const char *code);
char* prompt_and_get_input(const char* prompt_message); 
void generate_random_key(int key[MATRIX_SIZE][MATRIX_SIZE]);
void hill_cipher_EnCrypt_Dycrypt(char *text, int group_size, int key[MATRIX_SIZE][MATRIX_SIZE], int is_decrypt);
void print_matrix(int matrix[MATRIX_SIZE][MATRIX_SIZE]);
int stored_keys[MAX_KEYS][MATRIX_SIZE][MATRIX_SIZE]; 
int stored_key_count = 0; 
char *stored_random_code[MAX_KEYS];
int calculate_determinant(int matrix[MATRIX_SIZE][MATRIX_SIZE]);
int mod_inverse(int a, int m);
void calculate_adjMatrix(int matrix[MATRIX_SIZE][MATRIX_SIZE], int adj[MATRIX_SIZE][MATRIX_SIZE]);
int calculate_inverse_matrix(int matrix[MATRIX_SIZE][MATRIX_SIZE], int inverse[MATRIX_SIZE][MATRIX_SIZE]);

int main() {
    char choose;

    do {
        printf("選擇加密 (e)、解密 (d) 或列出密鑰 (l) 或清空視窗 (c)：");
        if (scanf(" %c", &choose) != 1) {  
            printf("讀取選項時發生錯誤。\n");
            return 1;
        }

        int c;
        while ((c = getchar()) != '\n' && c != EOF); 

        if (choose == 'e' || choose == 'E') {
            encrypt();
        } else if (choose == 'd' || choose == 'D') {
            decrypt();
        } else if (choose == 'l' || choose == 'L') {
            list_stored_keys();
        } else if (choose == 'c' || choose == 'C') {
            #ifdef _WIN32
                system("cls"); 
            #endif
        } else {
            printf("選項無效，請輸入 'e'、'd'、'l' 或 'c'。\n");
        }

        printf("是否繼續使用程式？(y/n)：");
        char continue_choice;
        while (scanf(" %c", &continue_choice) != 1 || 
       (continue_choice != 'y' && continue_choice != 'Y' && continue_choice != 'n' && continue_choice != 'N')) {
        printf("錯誤輸入，是否繼續使用程式？(y/n)：");
        while ((c = getchar()) != '\n' && c != EOF); 
}

    if (continue_choice == 'n' || continue_choice == 'N') {
        printf("程式結束。\n");
        break; 
}
        while ((c = getchar()) != '\n' && c != EOF); 
    } while (1);
    system("pause"); 
    return 0; 
}

char* prompt_and_get_input(const char* prompt_message) {
    char temp[100]; 
    printf("%s", prompt_message); 
   
    if (fgets(temp, sizeof(temp), stdin) == NULL) {
        if (feof(stdin)) {
            printf("\n輸入意外結束。\n");
        } else {
            printf("輸入錯誤！\n");
        }
        return NULL; 
    }
    return inputword_process(temp);
}

char* inputword_process(char word_input[]) {
    char *dynamic_input;
    size_t length;

    word_input[strcspn(word_input, "\n")] = '\0';
    length = strlen(word_input);

    dynamic_input = (char *)malloc((length + 1) * sizeof(char));
    if (dynamic_input == NULL) {
        printf("記憶體配置失敗！\n");
        return NULL;
    }

    for (size_t i = 0; i < length; i++) {
        dynamic_input[i] = toupper(word_input[i]);
    }
    dynamic_input[length] = '\0';

    return dynamic_input;
}

void encrypt() {
    char *text_to_encrypt = prompt_and_get_input("請輸入要加密的文字：");
    if (text_to_encrypt == NULL) {
        return;
    }

    printf("原始文字：%s\n", text_to_encrypt);

    int key[MATRIX_SIZE][MATRIX_SIZE];
    generate_random_key(key);
    
    //printf("隨機生成的加密矩陣 K：\n");
    //print_matrix(key);
    
    char *original_text = strdup(text_to_encrypt); 
    
    hill_cipher_EnCrypt_Dycrypt(text_to_encrypt, MATRIX_SIZE, key, 0);
    
    printf("加密後文字：%s\n", text_to_encrypt);
    
    strcpy(text_to_encrypt, original_text); 
    free(original_text);
    free(text_to_encrypt);
}

void decrypt() {
    if (stored_key_count == 0) {
        printf("目前沒有儲存的密鑰。\n");
        return;
    }

    char *text_to_decrypt = prompt_and_get_input("請輸入要解密的文字：");
    if (text_to_decrypt == NULL) {
        return;
    }

    printf("正在解密：%s\n", text_to_decrypt);

    printf("可用的密鑰代碼：\n");
    for (int i = 0; i < stored_key_count; i++) {
        printf("%d. 密鑰代碼：%s\n", i + 1, stored_random_code[i]);
    }
    
    char key_code[5];
    printf("請輸入要使用的密鑰代碼：");
    if (scanf("%4s", key_code) != 1) {
        printf("輸入錯誤。\n");
        free(text_to_decrypt);
        return;
    }
    int key_index = find_key_by_code(key_code);
    if (key_index == -1) {
        printf("無效的密鑰代碼。\n");
        free(text_to_decrypt);
        return;
    }

    int inverse[MATRIX_SIZE][MATRIX_SIZE];
    if (!calculate_inverse_matrix(stored_keys[key_index], inverse)) {
        free(text_to_decrypt);
        return;
    }

    //printf("使用的反矩陣 K^-1：\n");
    //print_matrix(inverse);

    hill_cipher_EnCrypt_Dycrypt(text_to_decrypt, MATRIX_SIZE, inverse, 1);
    
    
    int len = strlen(text_to_decrypt);
    while (len > 0 && text_to_decrypt[len-1] == 'X') {  
        text_to_decrypt[len-1] = '\0';
        len--;
    }
    
    printf("解密後文字：%s\n", text_to_decrypt);

    free(text_to_decrypt);
}

void generate_random_key(int key[MATRIX_SIZE][MATRIX_SIZE]) {
    srand(time(NULL)); 
    while (1) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                key[i][j] = rand() % 26; 
            }
        }

        int det = calculate_determinant(key);
        det = (det % 26 + 26) % 26; 

        if (mod_inverse(det, 26) != -1) {
            break; 
        }
    }
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            stored_keys[stored_key_count][i][j] = key[i][j]; 
        }
    }
    char random_code[5];
    for (int i = 0; i < 4; i++) {
        random_code[i] = 'A' + (rand() % 26); 
    }
    random_code[4] = '\0'; 

    stored_random_code[stored_key_count] = (char *)malloc(5 * sizeof(char));
    if (stored_random_code[stored_key_count] == NULL) {
        printf("記憶體分配失敗！\n");
        return;
    }
    strcpy(stored_random_code[stored_key_count], random_code);

    stored_key_count++;
    printf("密鑰已成功儲存，本次生成的密鑰代碼是：%s\n", random_code);
    printf("您已儲存 %d 個代碼\n", stored_key_count);
}
void hill_cipher_EnCrypt_Dycrypt(char *text, int group_size, int key[MATRIX_SIZE][MATRIX_SIZE], int is_decrypt) {
    int length = strlen(text);
    int actual_chars = 0;
    

    for (int i = 0; i < length; i++) {
        if (text[i] != ' ') actual_chars++;
    }


    if (!is_decrypt) {
        int remainder = actual_chars % group_size;
        if (remainder != 0) {
            int padding_needed = group_size - remainder;

            for (int i = 0; i < padding_needed; i++) {
                text[length + i] = 'X';
            }
            length += padding_needed;
            text[length] = '\0';
            actual_chars += padding_needed; 
        }
    }


    char *temp_text = (char *)malloc(actual_chars + 1);
    int temp_index = 0;
    for (int i = 0; i < length; i++) {
        if (text[i] != ' ') {
            temp_text[temp_index++] = text[i];
        }
    }
    temp_text[actual_chars] = '\0';


    for (int i = 0; i < actual_chars; i += group_size) {
        int vector[MATRIX_SIZE] = {0};
        int result[MATRIX_SIZE] = {0};
        int chars_in_group = 0;


        for (int j = 0; j < group_size && (i + j) < actual_chars; j++) {
            vector[j] = temp_text[i + j] - 'A';
            chars_in_group++;
        }


        if (chars_in_group < group_size && is_decrypt) {
            break;
        }


        for (int col = 0; col < group_size; col++) {
            for (int row = 0; row < group_size; row++) {
                if (row < chars_in_group) {
                    result[col] += vector[row] * key[row][col];
                }
            }
            result[col] %= 26;
            if (result[col] < 0) {
                result[col] += 26;
            }
        }


        for (int j = 0; j < chars_in_group && (i + j) < actual_chars; j++) {
            temp_text[i + j] = result[j] + 'A';
        }
    }


    temp_index = 0;
    for (int i = 0; i < length; i++) {
        if (text[i] != ' ') {
            if (temp_index < actual_chars) {
                text[i] = temp_text[temp_index++];
            }
        }
    }

    free(temp_text);


    if (is_decrypt) {
        int last_char = length - 1;
        while (last_char >= 0 && text[last_char] == 'X') {
            text[last_char] = '\0';
            last_char--;
        }
    }
}
void print_matrix(int matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            printf("%2d ", matrix[i][j]);
        }
        printf("\n");
    }
}

void list_stored_keys() {
    if (stored_key_count == 0) {
        printf("目前沒有儲存的密鑰。\n");
        return;
    }

    printf("已儲存的密鑰矩陣：\n");
    for (int k = 0; k < stored_key_count; k++) {
        printf("密鑰 #%d:\n", k + 1);
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                printf("%2d ", stored_keys[k][i][j]);
            }
            printf("\n");
        }
    }
}

int calculate_determinant(int matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    int det = 0;
    det += matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]);
    det -= matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0]);
    det += matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]);
    return det;
}

int mod_inverse(int a, int m) {
    a = (a % m + m) % m;
    for (int x = 1; x < m; x++) {
        if ((a * x) % m == 1) {
            return x; 
        }
    }
    return -1;
}

void calculate_adjMatrix(int matrix[MATRIX_SIZE][MATRIX_SIZE], int adj[MATRIX_SIZE][MATRIX_SIZE]) {
    adj[0][0] = (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]);
    adj[0][1] = -(matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0]);
    adj[0][2] = (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]);

    adj[1][0] = -(matrix[0][1] * matrix[2][2] - matrix[0][2] * matrix[2][1]);
    adj[1][1] = (matrix[0][0] * matrix[2][2] - matrix[0][2] * matrix[2][0]);
    adj[1][2] = -(matrix[0][0] * matrix[2][1] - matrix[0][1] * matrix[2][0]);

    adj[2][0] = (matrix[0][1] * matrix[1][2] - matrix[0][2] * matrix[1][1]);
    adj[2][1] = -(matrix[0][0] * matrix[1][2] - matrix[0][2] * matrix[1][0]);
    adj[2][2] = (matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0]);

    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            adj[i][j] = (adj[i][j] % 26 + 26) % 26;
        }
    }
}

int calculate_inverse_matrix(int matrix[MATRIX_SIZE][MATRIX_SIZE], int inverse[MATRIX_SIZE][MATRIX_SIZE]) {
    int det = calculate_determinant(matrix);
    int det_mod_26 = (det % 26 + 26) % 26;
    int det_inv = mod_inverse(det_mod_26, 26);
    if (det_inv == -1) {
        printf("矩陣不可逆，行列式 Det(K) mod 26 = %d，與 26 不互質。\n", det_mod_26);
        
        return 0;
    }
    int cofactor[MATRIX_SIZE][MATRIX_SIZE]; 
    calculate_adjMatrix(matrix, cofactor); 
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            inverse[i][j] = (det_inv * cofactor[j][i]) % 26;
            inverse[i][j] = (inverse[i][j] + 26) % 26; 
        }
    }
    return 1;
}

int find_key_by_code(const char *code) {  
    for (int i = 0; i < stored_key_count; i++) {
        if (strcmp(stored_random_code[i], code) == 0) {
            return i; 
        }
    }
    return -1; 
} 