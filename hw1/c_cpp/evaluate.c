#include "hmm.h"

int main(int argc, char*argv[])
{
    if (argc != 3 && argc != 4){
        printf("Wrong command format\n");
		printf("Format: ./evaluate [result1.txt] [testing_answer.txt] [(acc.txt)]\n");
		exit(1);
    }

    const char *result_file = argv[1];
	const char *answer_file = argv[2];
    const char *acc_file;
    

    if (argc == 4){
        acc_file = argv[3];
    }
    else{
        acc_file = "acc.txt";
    }

    int TP = 0, N = 0;
    char prediction[MAX_LINE];
    char GT[MAX_LINE];
    double prob;
    double acc = 0;

    FILE *fp_result, *fp_answer;
    fp_result = open_or_die(result_file, "r");
    fp_answer = open_or_die(answer_file, "r");
    
    while (!feof(fp_result) && !feof(fp_answer)){
        fscanf(fp_result, "%s %le", prediction, &prob);
        fscanf(fp_answer, "%s", GT);
        //printf("%s", prediction);
        //printf(" %s\n", GT);
        
        int idx = (int)(strrchr (prediction, '_')-prediction);

        if (idx > 5 ){
            if (strncmp(prediction, GT, idx) == 0) {
                TP++;
            }

        }else{
            if (strcmp(prediction, GT) == 0) {
                TP++;
            }
        }
        

        N++;
    }
    
    fclose(fp_result);
    fclose(fp_answer);

    printf("True Positive: %d\n", TP);
    printf("Total Number: %d\n", N);

    acc = (double)TP/(double)N;

    printf("Accuracy: %f\n", acc);
    FILE *fp = open_or_die(acc_file, "w");
    fprintf(fp, "%f ", acc);
    fclose(fp);

    return 0;
}