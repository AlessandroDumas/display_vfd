
DIM="1024x256"

./vfd | ffplay -f rawvideo -pixel_format gray8 -video_size $DIM -i -

