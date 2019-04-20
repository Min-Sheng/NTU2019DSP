## Modify the model format in "lib/proto" to change the # of states

此部份的實驗為更改 states 數量，觀察 accuracy 的變化情形，在固定 baseline 的初始值 (Gaussian mean, variance, and transition probability values) 的情況下，逐步調整 states 數量，可繪出如下的曲線圖， accuracy 從 baseline 的 74.34% 上升至 95.97% 。

<img width="80%" src="https://github.com/Min-Sheng/NTU2019DSP/raw/master/hw2/experiments/state_num/curve.png"/>
