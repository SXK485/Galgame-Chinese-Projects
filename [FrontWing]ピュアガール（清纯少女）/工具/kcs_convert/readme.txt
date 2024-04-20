解包：将ksc文件放置于ksc目录下，在python环境下运行python exkcs.py，解包后的文件将输出至kcs_ext目录
封包：将已解包的kcs文件放置于kcs_ext目录下，在python环境下运行python packNewKcs.py，解包后的文件将输出至fes目录

注意：packNewKcs.py的代码只考虑封包sscript.kcs文件，如要处理其他文件请修改一下代码。
另外，该代码不完善，封包未考虑数据长度，修改文件请保持数据长度和原来一样，要么完善代码。

项目地址：https://github.com/regomne/chinesize