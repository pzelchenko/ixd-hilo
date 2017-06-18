sox $1.wav -c 1 -r 4000 $1_4k.wav
sox $1_4k.wav $1_cut.wav trim 0 10000s
~/Desktop/Peter/wav2c-master/wav2c $1_cut.wav $1.h s_$1
