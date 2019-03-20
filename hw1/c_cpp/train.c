// Training Procedure: Baum-Welch Algorithm
#include "hmm.h"
#include <math.h>
#include <stdlib.h>

Observ data[MAX_DATA_LINE];
Variable alpha[MAX_DATA_LINE], beta[MAX_DATA_LINE], gamma[MAX_DATA_LINE];
Epsilon epsilon[MAX_DATA_LINE];

// Forward algorithm
void forward_algorithm(HMM *hmm, Observ *observ, Variable *alpha)
{
    int i,j,t; //state index: i,j; sequence index: t
    alpha->seq_num = observ->seq_num;
    alpha->state_num = hmm->state_num;

    // Initialization
    for (i = 0; i < alpha->state_num; i++ ){
        alpha->variable[0][i] = (hmm->initial[i]) * (hmm->observation[observ->obs[0]][i]); // alpha[0][i] = pi[i] * b[o[0]][i]
                                                                                           // , 0<=i<=N-1
                                                                                           // Here the items start from 0 instead of 1, which is different from the lecture.
    }

    // Induction
    for (t = 0; t < alpha->seq_num - 1; t++) {
        for (j = 0; j < alpha->state_num; j++) {
            double sum = 0; // for record the sum of all {alpha[t][i] * a[i][j]}

            for (i = 0; i < hmm->state_num; i++) {
                sum += alpha->variable[t][i] * hmm->transition[i][j];
            }

            alpha->variable[t+1][j] = sum * hmm->observation[observ->obs[t+1]][j]; // alpha[t+1][j] = \sum_i=0^(N-1){alpha[t][i] * a[i][j]} * b[o[t+1]][j]
                                                                                  // , 0<=t<=T-2, 0<=j<=N-1
        }
    }

    // Termination
    double prob = 0;
    for (i = 0; i < hmm->state_num; i++) {
        prob += alpha->variable[observ->seq_num-1][i]; /// P(O|lambda) = \sum_i=0^(N-1){alpha[T-1][i]}
    }

    return;

}


// Backward algorithm
void backward_algorithm(HMM *hmm, Observ *observ, Variable *beta)
{
    int i,j,t; //state index: i,j; sequence index: t
    beta->seq_num = observ->seq_num;
    beta->state_num = hmm->state_num;

    // Initialization
    for (i = 0; i < beta->state_num; i++ ){
        beta->variable[beta->seq_num-1][i] = 1; // beta[T-1][i] = 1
                                  // , 0<=i<=N-1
                                  // Here the items start from 0 instead of 1, which is different from the lecture.
    }

    // Induction
    for (t = beta->seq_num - 2; t >-1 ; t--) {
        for (j = 0; j < beta->state_num; j++) {
            double sum = 0; // for record the sum of all {a[i][j] * b[o[t+1]][j] * beta[t+1][j]}

            for (i = 0; i < hmm->state_num; i++) {
                sum += hmm->transition[i][j] * hmm->observation[observ->obs[t+1]][j] * beta->variable[t+1][j];
            }

            beta->variable[t][j] = sum; // beta[t][i] = \sum_i=0^(N-1){a[i][j] * b[o[t+1]][j] * beta[t+1][j]}
                                        // , 0<=t<=T-2, 0<=i<=N-1
        }
    }

    return;

}

// Baum-Welch algorithm: calculate gamma and epsilon
void baum_welch(HMM *hmm, Observ *observ, Variable *alpha, Variable *beta, Variable *gamma, Epsilon *epsilon)
{

}

// Update parameters
void update_param(HMM *hmm, Observ *observ, Variable *delta, Epsilon *epsilon, int data_num)
{

}


int main(int argc, char *argv[])
{
    if (argc !=  5){
        printf("Error: Wrong Command format\n");
        printf("Format: ./train [iteration] [model_init.txt] [seq_model_0x.txt] [model_0x.txt]\n");
        exit(1);
    }

    int i;
    int data_num; //total number of lines in the data file

    const int iter = atoi(argv[1]);
    const char *model_init = argv[2];
    const char *train_data = argv[3];
    const char *model_file = argv[3];

    printf("iter: %d\n", iter);
    printf("model_init: %s\n", model_init);
    printf("train_data: %s\n", train_data);
    printf("model_file: %s\n", model_file);

    // Load initial HMM model
    HMM hmm_initial;
	loadHMM( &hmm_initial, model_init);
	dumpHMM( stderr, &hmm_initial );

    data_num = fetch_data(data, train_data);
    printf("data_num: %d\n", data_num);

    return 0;
}