// Training Procedure: Baum-Welch Algorithm
#include "hmm.h"
#include <math.h>
#include <stdlib.h>

Observ data[MAX_SAMPLE_NUM];
Variable alpha[MAX_SAMPLE_NUM], beta[MAX_SAMPLE_NUM], _gamma[MAX_SAMPLE_NUM]; // each is N*T matrix array (total shape is M*N*T, where L means number of samples)
Epsilon epsilon[MAX_SAMPLE_NUM]; // it's a (T-1)*N*N matrix array (total shape is M*N*T, where M means number of samples)

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
    for (t = 0; t < alpha->seq_num - 1; t++){
        for (j = 0; j < alpha->state_num; j++) {
            double sum_alpha = 0; // for accumulate the sum of all {alpha[t][i] * a[i][j]}

            for (i = 0; i < alpha->state_num; i++){
                sum_alpha += alpha->variable[t][i] * hmm->transition[i][j];
            }
            
            alpha->variable[t+1][j] = sum_alpha * hmm->observation[observ->obs[t+1]][j]; // alpha[t+1][j] = \sum_i=0^(N-1){alpha[t][i] * a[i][j]} * b[o[t+1]][j]
                                                                                         // , 0<=t<=T-2, 0<=j<=N-1
        }
    }

    // Termination
    //double prob = 0;
    //for (i = 0; i < hmm->state_num; i++){
    //    prob += alpha->variable[observ->seq_num-1][i]; /// P(O|lambda) = \sum_i=0^(N-1){alpha[T-1][i]}
    //}
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
    for (t = beta->seq_num - 2; t >-1 ; t--){
        for (j = 0; j < beta->state_num; j++){
            double sum_beta = 0; // for accumulate the sum of all {a[i][j] * b[o[t+1]][j] * beta[t+1][j]}

            for (i = 0; i < hmm->state_num; i++){
                sum_beta += hmm->transition[i][j] * hmm->observation[observ->obs[t+1]][j] * beta->variable[t+1][j];
            }

            beta->variable[t][j] = sum_beta; // beta[t][i] = \sum_i=0^(N-1){a[i][j] * b[o[t+1]][j] * beta[t+1][j]}
                                             // , 0<=t<=T-2, 0<=i<=N-1
        }
    }
}

// Baum-Welch algorithm: calculate gamma and epsilon
void baum_welch(HMM *hmm, Observ *observ, Variable *alpha, Variable *beta, Variable *_gamma, Epsilon *epsilon)
{
    int i,j, t;
    _gamma->seq_num = observ->seq_num;
    _gamma->state_num = hmm->state_num;
    epsilon->seq_num = observ->seq_num;
    epsilon->state_num = hmm->state_num;

    
    for (t = 0; t < observ->seq_num-1; t++){
        // P(q_t=i | O, lambda) = gamma[t][i]
        // gamma[t][i] = (alpha[t][i]*beta[t][i])/(\sum_i=0^(N-1){alpha[t][i]*beta[t][i]})
    
        // P(q_t=i, q_(t+1)=j | O, lambda) = epsilon[t][i][j]
        // epsilon[t][i][j] = (alpha[t][i]*a[i][j]*b[o[t+1]][j]*beta[t+1][j])/(\sum_i=0^(N-1)\sum_j=0^(N-1){alpha[t][i]*a[i][j]*b[o[t+1]][j]*beta[t+1][i]})
        double sum_gamma=0 , sum_epsilon = 0;
        for (i = 0; i< hmm->state_num; i++){
            _gamma->variable[t][i] = alpha->variable[t][i]*beta->variable[t][i];
            sum_gamma += _gamma->variable[t][i];

            for (j = 0; j<hmm->state_num; j++){
                epsilon->variable[t][i][j] = alpha->variable[t][i]*hmm->transition[i][j]*\
                                                hmm->observation[observ->obs[t+1]][j]*beta->variable[t+1][j];
                sum_epsilon+= epsilon->variable[t][i][j];
            }
        }

        // Normalization
        for (i=0; i<hmm->state_num; i++){
            _gamma->variable[t][i] /= sum_gamma;
            for (j=0; j<hmm->state_num; j++){
                epsilon->variable[t][i][j] /= sum_epsilon;
            }
        }
    }
}

// Update parameters
void update_param(HMM *hmm, Observ *observ, Variable *_gamma, Epsilon *epsilon, int sample_num)
{

}


int main(int argc, char *argv[])
{
    if (argc !=  5){
        printf("Error: Wrong Command format\n");
        printf("Format: ./train [iteration] [model_init.txt] [seq_model_0x.txt] [model_0x.txt]\n");
        exit(1);
    }

    int i, j;
    int sample_num; //total number of samples in the data file

    const int iter = atoi(argv[1]);
    const char *model_init = argv[2];
    const char *train_data = argv[3];
    const char *model_file = argv[4];

    printf("iter: %d\n", iter);
    printf("model_init: %s\n", model_init);
    printf("train_data: %s\n", train_data);
    printf("model_file: %s\n", model_file);

    // Load initial HMM model
    HMM hmm_initial;
	loadHMM( &hmm_initial, model_init);
	dumpHMM( stderr, &hmm_initial );

    sample_num = fetch_data(data, train_data);
    printf("data_num: %d\n", sample_num);

    // Train the model
    for (i = 0; i < iter; i++){
        printf("\n----- iteration: #%d -----\n", i+1);
        for (j = 0 ;j < sample_num; j++){
            forward_algorithm(&hmm_initial, &data[j], &alpha[j]);
            backward_algorithm(&hmm_initial, &data[j], &beta[j]);
            baum_welch(&hmm_initial, &data[j], &alpha[j], &beta[j], &_gamma[j], &epsilon[j]);
        }
        update_param(&hmm_initial, data, _gamma, epsilon, sample_num);
        dumpHMM(stderr, &hmm_initial);
    }

    // Save model
    //printf("Save HMM model file: %s\n", model_file);
    //FILE *fp = open_or_die(model_file, "w");
    //dumpHMM(fp, &hmm_initial);
    //fclose(fp);

    return 0;
}