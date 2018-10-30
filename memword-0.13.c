#include <stdio.h>	
#include <string.h>	
#include <unistd.h>	
#include <dirent.h>	// DIR, struct dirent
			/* struct dirent {
			  	 ino_t          d_ino;       // inode 番号
			  	 off_t          d_off;       // オフセットではない; 注意を参照
			  	 unsigned short d_reclen;    // このレコードの長さ
			  	 unsigned char  d_type;      // ファイル種別。全ファイルシステム
                               	 			     // でサポートされているわけではない
 			  	 char           d_name[256]; // ファイル名 
			  	 };
			*/
#include <stdlib.h>
#include <time.h>
/*	define		*/
#define PATH_MAX	4096
#define NAME_MAX	255
#define FILES_DIR_NAME	"/Files/"	// 出題ファイル格納ディレクトリ
#define FILES_MAX	256			// 取り扱う出題ファイル数の最大
#define STRINGS_MAX	1024		// 一つの文字列の最大
#define QUESTION_MAX	2000		// 一つのファイルの最大出題数
#define SUCCESS		0
#define ERROR		1
#define EDITER		"vi "
/*	struct	*/
struct filelist_struct {		// 番号とファイル名の組み合わせ
	unsigned char file_number;
	char file_name[NAME_MAX];
};
struct answer_and_question {
	unsigned long	number;
	unsigned long	rand_key;
	char answer[STRINGS_MAX];
	char question[STRINGS_MAX];
};
/*	function	*/
void scanf_and_errorcheck(const char *format, const void *variable_p) {
	if(EOF == scanf(format, variable_p)) {
		printf("ユーザーからの入力の受取に失敗しました。\n");
		exit(ERROR);
	}
};
int main(int argc,char** argv)
{
	/*	variable	*/
	DIR *files_dir;					// Files ディレクトリ
	FILE *reading_fp;				// 出題ファイルを格納
	struct dirent *files_dp;				// ディレクトリのデータを扱う構造体
	struct filelist_struct filelist_s[FILES_MAX];	// 出題ファイル一覧の構造体
	struct answer_and_question answer_and_question_s[QUESTION_MAX], // 出題番号,解答,問題,の構造体
		answer_and_question_tmp;		// sort用
	char files_dir_path[PATH_MAX], 			// 出題ファイルのパス
	     user_input_y_or_n,			// ユーザ入力の y か n を格納する
	     user_input_answer[STRINGS_MAX],		// ユーザの解答を格納
	     command_line_str[strlen(EDITER) + NAME_MAX];	// vi起動用
	unsigned char cnt, cnt_of_question, cnt1, cnt2, 			// ループ制御用変数
		      number_of_files;			// 出題ファイル数
	unsigned short user_input_num = 0,			// ユーザーの入力した数値
		       number_of_start_question = 0,	// 開始時の出題の行番号
		       number_of_end_question = 0;		// 最後の出題の行番号
	unsigned int question_max = 0;			// 最大出題数 (あとで sizeof の割り算に変更)
	/*	method		*/
	strcat(command_line_str, EDITER);
	if (NULL == getcwd(files_dir_path, PATH_MAX)) {
		printf("カレントディレクトリの取得に失敗しました。\n");
		exit(ERROR);
	}
	if (PATH_MAX < (strlen(files_dir_path) + strlen(FILES_DIR_NAME))) {
		printf("出題ファイルまでのパスの長さが PATE_MAX(4096) 以上です。");
		exit(ERROR);
	} else {
		strcat(files_dir_path, FILES_DIR_NAME);
	}
	if (!(files_dir = opendir(files_dir_path))) {
		printf("%s ディレクトリが存在しません。\n", files_dir_path);
		exit(ERROR);
	}
		// 出題ファイルが格納されているディレクトリまで移動(エラー処理書く)
		if ((-1) == chdir(files_dir_path)) {
			printf("%s ディレクトリへの移動に失敗しました。\n", files_dir_path);
			exit(ERROR);
		};
		for (files_dp = readdir(files_dir), cnt = 0; files_dp != NULL; files_dp = readdir(files_dir)){
			/*	. と .. は一覧に代入しない	*/
			if ((!strcmp(files_dp->d_name,".")) || (!strcmp(files_dp->d_name,".."))) {
				continue;
			}
			filelist_s[cnt].file_number = cnt + 1;
			strcpy(filelist_s[cnt].file_name, files_dp->d_name);
			cnt++;
		}
		closedir(files_dir);
		if (0 == cnt) {
			printf("出題用のファイルが存在しません。\n%s.\nに出題用のファイルを作成してください。\n", files_dir_path);
			exit(ERROR);
		};
		number_of_files = cnt;		// 出題ファイルの量を代入
	for (;;) {
		printf("memword start menu. 数値を入力して下さい。\n");
		printf("[  1] 暗記を始める。\n");
		printf("[  2] 出題ファイルを編集する。\n");
		printf(":");
		scanf_and_errorcheck("%hd", &user_input_num);
		getchar();				// 標準入力を空にする
		printf("\n");
		if (1 == user_input_num) {
			/*	出題ファイル選択ループ	*/
			for (;;) {
				printf("出題ファイルを数値で入力してください。(Please select a file and enter anumerical value)\n");
				/*	出題ファイル番号と出題ファイル名を出力	*/
				for (cnt = 0;cnt < number_of_files; cnt++) {
					printf("[%3d] %s\n", filelist_s[cnt].file_number, filelist_s[cnt].file_name);
				}
				printf(":");
				scanf_and_errorcheck("%hd", &user_input_num);
				getchar();				// 標準入力を空にする
				printf("\n");
				/*	入力エラーチェック	*/
				if (user_input_num > number_of_files) {
					printf("\n実際の問題の量以上の値か、負の値が入力されました。\n");
					printf("A number greater than the actual number of file was entered.\n");
				} else if (0 == user_input_num) {
					printf("\n[  0]は存在しません。1以上の数値を入力して下さい。 \n");
				} else {
					break;
				}
			}
			printf("%s\n", filelist_s[user_input_num - 1].file_name);
			if ((reading_fp = fopen(filelist_s[user_input_num - 1].file_name, "r")) == NULL) {
				printf("ファイルの読み込みに失敗しました。\n");
				printf("file open error.\n");
				exit(ERROR);
			}
		} else if (2 == user_input_num) {
			for (;;) {
				printf("編集するファイルを数値で入力してください。(Please select a file and enter anumerical value)\n");
				/*	出題ファイル番号と出題ファイル名を出力	*/
				for (cnt = 0;cnt < number_of_files; cnt++) {
					printf("[%3d] %s\n", filelist_s[cnt].file_number, filelist_s[cnt].file_name);
				}
				printf(":");
				scanf_and_errorcheck("%hd", &user_input_num);
				getchar();				// 標準入力を空にする
				printf("\n");
				/*	入力エラーチェック	*/
				if (user_input_num > number_of_files) {
					printf("\n実際の問題の量以上の値か、負の値が入力されました。\n");
					printf("A number greater than the actual number of file was entered.\n");
				} else if (0 == user_input_num) {
					printf("\n[  0]は存在しません。1以上の数値を入力して下さい。 \n");
				} else {
					break;
				}
			}
			printf("%s\n", filelist_s[user_input_num - 1].file_name);
			strcat(command_line_str, filelist_s[user_input_num - 1].file_name);
			if (-1 == system(command_line_str)) {
				printf("shell が利用可能な状態では無いです。\n");
			};
			strcpy(command_line_str, EDITER);
			continue;
		} else {
			printf("1か2以外が入力されました。\n");
			exit(ERROR);
		}
		user_input_num = 0;
		for (;;) {
			printf("出題の順番を数値で入力して下さい。\n");
			printf("[  1] 一行目から順番に出題する。\n");
			printf("[  2] ランダムに出題する。\n");
//	printf("[  3] 解答の文字数が少ない順に出題する。\n");
			printf(":");
			scanf_and_errorcheck("%hu", &user_input_num);
			getchar();	// 標準入力を空にする
			printf("\n");
			if (1 == user_input_num) {
				/*	解答と、出題を抽出	*/
				for (cnt1 = 0; !feof(reading_fp); cnt1++) {
					answer_and_question_s[cnt1].number = cnt1 + 1;
					for(cnt2 = 0 ;; cnt2++){
						answer_and_question_s[cnt1].answer[cnt2] = getc(reading_fp);
						if (answer_and_question_s[cnt1].answer[cnt2] == '\t') {
							answer_and_question_s[cnt1].answer[cnt2] = '\0';
							break;
						}
						if (feof(reading_fp)) break; 
					}
					for (cnt2 = 0; answer_and_question_s[cnt1].question[cnt2] != '\n'; cnt2++) {
						answer_and_question_s[cnt1].question[cnt2] = getc(reading_fp);
						if (answer_and_question_s[cnt1].question[cnt2] == '\n') {
							answer_and_question_s[cnt1].question[cnt2] = '\0';
							question_max++;			// 出題数を数える
							break;
						}
						if (feof(reading_fp)) break;
					}
				}
				fclose(reading_fp);
				break;
			} else if (2 == user_input_num) {
				srand((unsigned)time(NULL));
				/*	解答と、出題を抽出	*/
				for (cnt1 = 0; !feof(reading_fp); cnt1++) {
					for (cnt2 = 0 ;; cnt2++) {
						answer_and_question_s[cnt1].answer[cnt2] = getc(reading_fp);
						if (feof(reading_fp)) break; 
						if (answer_and_question_s[cnt1].answer[cnt2] == '\t') {
							answer_and_question_s[cnt1].answer[cnt2] = '\0';
							break;
						}
					}
					for (cnt2 = 0; answer_and_question_s[cnt1].question[cnt2] != '\n'; cnt2++) {
						answer_and_question_s[cnt1].question[cnt2] = getc(reading_fp);
						if (feof(reading_fp)) break; 
						if (answer_and_question_s[cnt1].question[cnt2] == '\n') {
							answer_and_question_s[cnt1].question[cnt2] = '\0';
							question_max++;			// 出題数を数える
							break;
						}
					}
					if (feof(reading_fp)) break; 
					answer_and_question_s[cnt1].number = cnt1 + 1;
					answer_and_question_s[cnt1].rand_key = rand();
				}
				fclose(reading_fp);
				for (cnt1 = 0; cnt1 < question_max; cnt1++) {
					for (cnt2 = 0; (cnt1 + cnt2) < question_max; cnt2++) {
						if (answer_and_question_s[cnt1].rand_key < answer_and_question_s[cnt1 + cnt2].rand_key) {
							answer_and_question_tmp = answer_and_question_s[cnt1];
							answer_and_question_s[cnt1] = answer_and_question_s[cnt1 + cnt2];
							answer_and_question_s[cnt1 + cnt2] = answer_and_question_tmp;
						}
					}
				}
				break;
			} else {
				printf("1か2以外が入力されました。\n");
			}
		}
		for (;;) {
			printf("出題数：%d\n", question_max);
			printf("全問出題しますか?(y/n)\n");
			printf(":");
			scanf_and_errorcheck("%c", &user_input_y_or_n);
			getchar();	// 標準入力を空にする
			printf("\n");
			if ('y' == user_input_y_or_n) {
				number_of_start_question = 0;
				number_of_end_question = question_max;
				break;
			} else if ('n' == user_input_y_or_n) {
				if (1 == user_input_num) {
					for(;;){
						printf("出題数：%d\n", question_max);
						printf("何問目から出題しますか?\n数値を入力して下さい:");
						scanf_and_errorcheck("%hd", &user_input_num);
						getchar();	// 標準入力を空にする
						/*	入力エラーチェック	*/
						if (user_input_num > question_max) {
							printf("\n実際の問題の量以上の値か、負の値が入力されました。\n");
							printf("A number greater than the actual number of file was entered.\n");
						} else if (0 == user_input_num) {
							printf("\n0問目は存在しません。1以上の数値を入力して下さい。 \n");
						} else {
							break;
						}
					}
					number_of_start_question = (user_input_num - 1);
					for (;;) {
						printf("\n出題開始行：%d\n", user_input_num);
						printf("出題数：%d\n", question_max);
						printf("何問目まで出題しますか?\n数値を入力して下さい:");
						scanf_and_errorcheck("%hd", &number_of_end_question);
						getchar();	// 標準入力を空にする
						/*	入力エラーチェック	*/
						if (number_of_end_question > question_max) {
							printf("\n実際の問題の量以上の値か、負の値が入力されました。\n");
							printf("A number greater than the actual number of file was entered.\n");
						} else if (number_of_start_question > number_of_end_question) {
							printf("\n出題開始行番号より小さい数値が入力されました。\n");
							printf("A number smaller than thaline number at which the question is to be stated has been entered.\n");
						} else {
							printf("\n");
							break;
						}
					}
				/*	random		*/
				} else if (2 == user_input_num) {
					number_of_start_question = 0;
					for (;;) {
						printf("全出題数：%d\n", question_max);
                                                printf("何問出題しますか?\n数値を入力して下さい\n");
						printf(":");
                                                scanf_and_errorcheck("%hd", &number_of_end_question);
                                                getchar();      // 標準入力を空にする
                                                /*      入力エラーチェック      */
                                                if (number_of_end_question > question_max) {
                                                        printf("\n実際の問題の量以上の値か、負の値が入力されました。\n");
                                                        printf("A number greater than the actual number of file was entered.\n");
                                                } else if (number_of_start_question > number_of_end_question) {
                                                        printf("\n出題開始行番号より小さい数値が入力されました。\n");
                                                        printf("A number smaller than thaline number at which the question is to be stated has been entered.\n");
                                                } else {
                                                        break;
                                                }
					}
				}
				break;
			} else {
				printf("y(yes) か n(no) を入力して下さい。\n\n");
			}
		}
		/*	出題	*/
		for (cnt = number_of_start_question, cnt_of_question = 1; cnt < number_of_end_question; cnt++, cnt_of_question++) {
			printf("question\t: #%hhu\nline number\t: #%lu\n", cnt_of_question, answer_and_question_s[cnt].number);
			printf("Q: %s\n", answer_and_question_s[cnt].question);
			printf(":");
			scanf_and_errorcheck("%[^\t\n]", user_input_answer);	// 空白も入力できるようにする
			getchar();		// 標準入力を空にする
			if (!strcmp(answer_and_question_s[cnt].answer, user_input_answer)) {
				printf("correct!!\n");
				printf("A: %s\n\n", answer_and_question_s[cnt].answer);
			} else {
				printf("miss!!\n");
				printf("A: %s\n\n", answer_and_question_s[cnt].answer);
				cnt_of_question--;
				cnt--;
			}
		}
		for (;;) {
			printf("出題が終わりました。プログラムを終了しますか?(y/n)\n");
			printf(":");
			scanf_and_errorcheck("%c", &user_input_y_or_n);
			getchar();		// 標準入力を空にする
			if ('y' == user_input_y_or_n) {
				break;
			} else if ('n' == user_input_y_or_n) {
				question_max = 0;
				break;
			} else {
				printf("y(yes) か n(no) を入力して下さい。\n\n");
			}
		}
		if ('y' == user_input_y_or_n) break;
	}
	exit(SUCCESS);
};
