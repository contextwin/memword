#include <stdio.h>	
#include <limits.h>	// PATH_MAX(4096) NAME_MAX(255)	
#include <string.h>	// strncat() cert cで推奨か調べる, strcmp(), strcpy()
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
#include <err.h>


#define FILES_DIR_NAME	"/Files/"	// 出題ファイル格納ディレクトリ
#define FILES_MAX 256			// 取り扱う出題ファイル数の最大
#define STRINGS_MAX 1024		// 一つの文字列の最大

struct filelist_struct {		// 番号とファイル名の組み合わせ、後で動的配列確保に変更
	unsigned char file_number;
	char file_name[NAME_MAX];
};

struct answer_and_question {
	unsigned long	number;
	char answer[STRINGS_MAX];
	char question[STRINGS_MAX];
};
	

int main(int argc,char** argv)
{
	DIR *files_dir;					// Files ディレクトリ
	struct dirent *dp;				// ディレクトリのデータを扱う構造体
	FILE *reading_fp,				// 出題ファイルを格納
	     *cmd_fp;					// コマンドの出力を格納
	struct filelist_struct filelist_s[FILES_MAX];	// 出題ファイル一覧の構造体 (後で動的配列確保へ変更)
	struct answer_and_question answer_and_question_s[1024]; // 出題番号,解答,問題,の構造体,後で動的配列確保へ変更
	char files_dir_path[PATH_MAX], 			// 出題ファイルのパス
	     mkdir_path[6] = "mkdir ",				// Files ディレクトリ作成用
	     os_name[128],				// unameコマンドの結果を格納する、128の数値は適当
	     user_input_y_or_n,			// ユーザ入力の y か n を格納する
	     user_input_answer[STRINGS_MAX];		// ユーザの解答を格納
	unsigned char cnt, cnt_of_question, cnt1, cnt2, 			// ループ制御用変数
		      number_of_files;			// 出題ファイル数
	unsigned short user_input_num,			// ユーザーの入力した数値
		       number_of_start_question = 0,	// 開始時の出題の行番号
		       number_of_end_question = 0;		// 最後の出題の行番号
	unsigned int question_max = 0;			// 最大出題数 (あとで sizeof の割り算に変更)

	// os識別いるかいらないかわからん(今の仕様だといらない)
	if ((cmd_fp = popen("uname", "r")) == NULL) {
		err(EXIT_FAILURE, "%s", "uname");
	}

	if (fgets(os_name, sizeof(os_name), cmd_fp) != NULL) {
		// os が Linux だった場合
		if (!strcmp("Linux\n", os_name)){
		getcwd(files_dir_path, PATH_MAX);
		// 文字連結,オーバーフロー時のエラー処理を書くこと
		strncat(files_dir_path, FILES_DIR_NAME, PATH_MAX);
		// directory open
		files_dir = opendir(files_dir_path);
		// directory open

		if (!(files_dir = opendir(files_dir_path))) {
			printf("%s ディレクトリが存在しません。\n作成してもよろしいですか?(y/n)\n", files_dir_path);
			scanf("%c", &user_input_y_or_n);
			getchar();	// 標準入力を空にする

			if ('y' == user_input_y_or_n) {
				strncat(mkdir_path, files_dir_path, PATH_MAX);
				printf("%s\n", mkdir_path);
				system(mkdir_path);
			}

			exit(EXIT_FAILURE);
		}

		// 出題ファイルが格納されているディレクトリまで移動
		chdir(files_dir_path);

			for (dp = readdir(files_dir), cnt = 0; dp != NULL; dp = readdir(files_dir)){
				/*	. と .. は一覧に代入しない	*/
				if ((!strcmp(dp->d_name,".")) || (!strcmp(dp->d_name,".."))) {
					continue;
				}

				filelist_s[cnt].file_number = cnt + 1;
				strcpy(filelist_s[cnt].file_name, dp->d_name);
				cnt++;
			}

		if (0 == cnt) {
			printf("出題用のファイルが存在しません。\n%s.\nに出題用のファイルを作成してください。\n", files_dir_path);
			exit(EXIT_SUCCESS);
		};

		} else {
			err(EXIT_FAILURE, "%s", "uname unknoun");
		}

		printf("%s\n", files_dir_path);

		// directory open
		if (!(files_dir = opendir(files_dir_path))) {
			printf("%s ディレクトリが存在しません。\n作成してもよろしいですか?(y/n)\n", files_dir_path);
			scanf("%c", &user_input_y_or_n);
			getchar();	// 標準入力を空にする
			exit(EXIT_FAILURE);
		}

	} else {
		err(EXIT_FAILURE, "%s", "fgets");
	}

	number_of_files = cnt;		// 出題ファイルの量を代入

	/*	出題ファイル選択ループ	*/
	for (;;) {
		/*	出題ファイル番号と出題ファイル名を出力	*/
		for (cnt = 0;cnt < number_of_files; cnt++) {
			printf("[%d] %s\n", filelist_s[cnt].file_number, filelist_s[cnt].file_name);
		}

		printf("出題ファイルを選択数値で選択してください\n");
		printf("Please select afile and enter a numerical value: ");
		scanf("%hd", &user_input_num);		// 後で最適かどうか調べる
		getchar();				// 標準入力を空にする

		/*	入力エラーチェック	*/
		if (user_input_num > number_of_files) {
			printf("\n実際の出題ファイルの量以上の数値が入力されました。\n");
			printf("A number greater than the actual number of file was entered.\n");
			getchar();
		} else if (0 >= user_input_num) {
			printf("\n入力された数値が小さすぎます。\n");
			printf("A value that was too small was entered.\n");
			getchar();
		} else {
			break;
		}
	}

	if ((reading_fp = fopen(filelist_s[user_input_num - 1].file_name, "r")) == NULL) {
		printf("ファイルの読み込みに失敗しました。\n");
		printf("file open error.\n");
		exit(EXIT_FAILURE);
	}


	/*	解答と、出題を抽出	*/
	for (cnt1 = 0; !feof(reading_fp); cnt1++) {
		answer_and_question_s[cnt1].number = cnt1 + 1;
		
		for(cnt2 = 0 ;; cnt2++){
			answer_and_question_s[cnt1].answer[cnt2] = getc(reading_fp);

			if(answer_and_question_s[cnt1].answer[cnt2] == '\t') {
				answer_and_question_s[cnt1].answer[cnt2] = '\0';
				break;
			}
			if(feof(reading_fp)) break; 

		}

		for(cnt2 = 0; answer_and_question_s[cnt1].question[cnt2] != '\n'; cnt2++){
			answer_and_question_s[cnt1].question[cnt2] = getc(reading_fp);

			if(answer_and_question_s[cnt1].question[cnt2] == '\n') {
				answer_and_question_s[cnt1].question[cnt2] = '\0';
				question_max++;			// 出題数を数える
				break;
			}

			if(feof(reading_fp)) break;
		}
	}

	
	for (;;) {
		printf("出題数：%d\n", question_max);
		printf("全問出題しますか?(y/n)");
		scanf("%c", &user_input_y_or_n);
		getchar();	// 標準入力を空にする

		printf("\n");

		if ('y' == user_input_y_or_n) {
			number_of_start_question = 0;
			number_of_end_question = question_max;
			break;

		} else if ('n' == user_input_y_or_n) {

			for(;;){
				printf("出題数：%d\n", question_max);
				printf("何問目から出題しますか?\n数値を入力して下さい:");
				scanf("%hd", &user_input_num);
				getchar();	// 標準入力を空にする

				/*	入力エラーチェック	*/
				if (user_input_num > question_max) {
					printf("\n実際の問題の量以上の数値が入力されました。\n");
					printf("A number greater than the actual number of file was entered.\n");
					getchar();
				} else if (0 >= user_input_num) {
					printf("\n入力された数値が小さすぎます。\n");
					printf("A value that was too small was entered.\n");
					getchar();
				} else {
					break;
				}
			}

			number_of_start_question = (user_input_num - 1);

			for (;;) {
				printf("出題数：%d\n", question_max);
				printf("何問目まで出題しますか?\n数値を入力して下さい:");
				scanf("%hd", &number_of_end_question);
				getchar();	// 標準入力を空にする
	
				/*	入力エラーチェック	*/
				if (number_of_end_question > question_max) {
					printf("\n実際の問題の量以上の数値が入力されました。\n");
					printf("A number greater than the actual number of file was entered.\n");
					getchar();
				} else if (0 >= number_of_end_question) {
					printf("\n入力された数値が小さすぎます。\n");
					printf("A value that was too small was entered.\n");
					getchar();
				} else if (number_of_start_question > number_of_end_question) {
					printf("\n出題開始行番号より小さい数値が入力されました。\n");
					printf("A number smaller than thaline number at which the question is to be stated has been entered.\n");
					getchar();
				} else {
					break;
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
		scanf("%[^\t\n]", user_input_answer);	// 空白も入力できるようにする
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

	fclose(reading_fp);
	exit(EXIT_SUCCESS);
};
