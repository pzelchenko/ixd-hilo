sox $1.wav -c 1 -r 8000 $1_8k.wav
sox $1_8k.wav $1_cut.wav trim 0 10000s
./wav2c $1_cut.wav $1.h s_$1
