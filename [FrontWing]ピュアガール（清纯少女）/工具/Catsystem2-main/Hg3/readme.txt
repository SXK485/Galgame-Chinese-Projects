转化：将hg3图片放置于exe同级目录下，运行exe即可
合成：将含有png与ini文件夹放置于exe同级目录下，运行exe即可

1.很早之前完成的代码，效率较为低下
2.或许存在内存泄漏现象
3.两个exe的区别仅仅是解压时是否使用多线程，多线程程序将产生大量的系统占用

Conversion: Place the hg3 images in the same directory as the exe, and run the exe to convert them. 
Synthesis: Place the folder containing png and ini files in the same directory as the exe, and run the exe to synthesize them.

The code was completed a long time ago and is relatively inefficient.
There may be occurrences of memory leaks.
The only difference between the two executables is whether to use multithreading during extraction. The multithreading program will result in significant system resource usage.

