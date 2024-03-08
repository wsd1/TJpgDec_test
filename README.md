# TJpgDec_test
---

```
gcc tjpgd.c test.c -o test


# ls -la *.jpg
-rw-r--r-- 1 root root  1939 Mar  8 15:29 paneldemo.jpg
-rw-r--r-- 1 root root 13059 Mar  8 15:59 panelnoise.jpg

# ./test paneldemo.jpg 
Image size is 512 x 32.
9624 bytes of work ares is used.
Decompression succeeded.Cost 3ms

# ./test panelnoise.jpg 
Image size is 512 x 32.
9624 bytes of work ares is used.
Decompression succeeded.Cost 9ms



```
