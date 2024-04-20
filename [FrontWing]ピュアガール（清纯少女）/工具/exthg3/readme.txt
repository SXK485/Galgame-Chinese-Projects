构建：
go mod init hg3
go get github.com/regomne/bstream
go build

解包示例：
hg3.exe -e -i row_hg3image/input.hg3 -o pngimage
hg3.exe -e -i row_hg3image/sys_click.hg3 -o pngimage

打包示例：
hg3.exe -p -i trs_pngimage/sys_title -o trs_hg3image/sys_title.hg3
hg3.exe -p -i trs_pngimage/sys_sound -o trs_hg3image/sys_sound.hg3
hg3.exe -p -i trs_pngimage/sys_dialog_qj -o trs_hg3image/sys_dialog_qj.hg3

注意：建议使用另一个Catsystem2-main进行hg3文件的解包和封包，用现在这个程序封回hg3图片，会出现多余的白边，原因不明

作者：https://zhuanlan.zhihu.com/p/635654709