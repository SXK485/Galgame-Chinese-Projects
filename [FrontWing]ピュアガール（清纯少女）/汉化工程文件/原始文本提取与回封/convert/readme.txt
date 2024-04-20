python CatConvert.py 0	# cat to txt（纯文字）
python CatConvert.py 1	# txt to cat
python CatConvert.py 2	# cat to bin（所谓bin，就是cst去掉文件头后其余的数据）
python CatConvert.py 3	# cat to txt（带控制字符）
python CatConvert.py 4	# txt to cat（将带控制字符txt文本回封）

代码来源知乎，本人做了一点小改动，以更好地匹配人工、正文、标题以及选项。
提取后位于scene_txt目录的文本，用工具SExtractor提取和导入文本
匹配规则可以用“用于SExtractor的匹配规则”目录中我自己写的规则

我本人的操作：
①用GARbro解包scene.int
②解包后得到一堆cst脚本，丢到scene_cst目录，运行python CatConvert.py 3后，scene_txt目录得到解密后的带控制字符的txt脚本
③将scene_txt的所有txt复制到其他地方，作为SExtractor的工作目录
④将用于SExtractor的匹配规则中的内容，复制到SExtractor的reg.ini中
⑤打开SExtractor，引擎选择“BIN”，导出格式选择“json [ {name,messageRN}]”,选择匹配规则为“CST”，“BIN启用纯文本正则模式”，TXT编码选项“UTF-8”，点击“提取/写入”，json输出至工作目录orig目录
⑥将json放入json_jp，用GalTransl翻译得到json_cn，将json_cn中所有的文件复制到工作目录的trans目录
⑦打开SExtractor，设置保存原来的样子，再次点击“提取/写入，译文输出至工作目录new目录
⑧将new目录的所有txt文件复制回scene_txt，运行python CatConvert.py 4后，scene_dst目录将得到翻译好的cst脚本
⑨将翻译好的cst脚本放回游戏的scene目录，也可以再次封包成scene.int

作者：https://zhuanlan.zhihu.com/p/623697843
SExtractor项目地址：https://github.com/satan53x/SExtractor