## Modify the # of Gaussian mixtures in "lib/mix2_10.hed"

此部份的實驗為更改 Gaussian mixture models 的數量，觀察 accuracy 的隨之變化的情形，而初始值的參數以及 states 數量我採用上個實驗的最佳值 (# of states = 16) ，另外，我在這裡將欲增加 Gaussian mixtures 的 states 均設為第 2~16 個 state 。

一開始的設定是在 "零~九" 每個 MFCC 維度以 2 個 Gaussian mixtures 來表示，而 "sil" 以 3 個 Gaussian mixtures 來表示，此時的 accuracy 為 96.20% ，而增加 Gaussian mixtures 的方式在本實驗分為兩種策略：

1. Once-for-all: 一次加到指定的 Gaussian mixtures 。
2. Step-by-step: 再每次 Re-estimate 之前增加 1 個 Gaussian mixtures ，逐步遞增至指定數量。

下圖為兩種策略的 accuracy 結果 (x 軸為 "零~九" 的 Gaussian mixtures 數量， "sil" 則加 1)，可以發現有 Re-estimate 做 fune-tuning 的緣故， Step-by-step 比 Once-for-all 的方法來的好，增加到 14 個 Gaussian mixtures 時，可達到 98.50% 。

<img width="80%" src="https://github.com/Min-Sheng/NTU2019DSP/raw/master/hw2/experiments/gaussian_mixture_num/curves.png"/>
