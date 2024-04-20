import os, sys
import re
import struct
import zlib

warnings = []


def warn(value):
    warnings.append(value)


def clearWarnings():
    global warnings
    warnings = []


def printWarnings(sc):
    global warnings
    for wi in warnings:
        print('[WARNING]', sc, wi)
    clearWarnings()


#将cst解压成bin
def cst2bin(datcst):
    tag, sizcst, sizbin = struct.unpack_from('8sII', datcst)
    if tag != b'CatScene':
        raise Exception('Label Mismatch')
    datcst = datcst[16:]
    if sizcst != len(datcst):
        raise Exception('Size Ante Decompress Mismatch')
    datbin = zlib.decompress(datcst)
    if sizbin != len(datbin):
        raise Exception('Size Post Decompress Mismatch')
    return datbin


def bin2cst(datbin):
    datcst = zlib.compress(datbin)
    return b'CatScene' + struct.pack('II', len(datcst), len(datbin)) + datcst


class FormatCST:
    def __init__(self, fc):
        # if fc is not None:
            b = cst2bin(fc.read())
            #将bin文件头的四个信息（数据体的长度，语句组的个数，偏移表的偏移量，语句块的偏移量），分别存储在h0、h1、h2、h3。
            # 之后通过b[16:]，将指针移到存储语句组信息的开头
            (h0, self.h1, self.h2, self.h3), b = struct.unpack_from('4I', b), b[16:]
            #根据偏移量，分割语句组的个数、偏移表、语句块
            self.b1, b2, b3 = b[:self.h2], b[self.h2:self.h3], b[self.h3:]
            if h0 != len(b) or self.h1 * 8 != self.h2 or (self.h3 - self.h2) % 4 != 0:
                raise Exception('Integrity Constraint 0 Violated')

            it = struct.iter_unpack('II', self.b1)
            flag = True
            self.n1 = 0
            #这个循环用于统计语句数
            while flag:
                try:
                    #d10是语句组的语句数，d11是语句组的起始位置（相对于原始文本）。这里的next应该是FormatCST类重写的方法，能够返回十进制的数
                    d10, d11 = next(it)
                    if d11 != self.n1:
                        flag = False
                    self.n1 += d10
                except StopIteration:
                    break
            if not flag or self.n1 * 4 != self.h3 - self.h2:
                raise Exception('Integrity Constraint 1 Violated')

            it = struct.iter_unpack('I', b2)
            #d2用于存储每个语句的偏移（相对于语句块）
            d2 = []
            while True:
                try:
                    d2.append(*next(it))
                except StopIteration:
                    break
            if self.n1 != len(d2):
                raise Exception('Integrity Constraint 2 Violated')

            ofs = 0
            self.d3 = []
            for i in range(self.n1):
                if ofs < d2[i]:
                    warn('Unaccessible Fragment Offset 0x{0:08X}'.format(ofs))
                    ofs = d2[i]
                if ofs > d2[i]:
                    raise Exception('Overflow Offset 0x{0:08X}'.format(ofs))
                try:
                    #b3是整个语句块，ofs是d2的数组下标？
                    d30, d31, d32 = struct.unpack_from('3B', b3, ofs)
                except Exception:
                    raise Exception('Content Truncated')
                if d30 != 0x01:
                    raise Exception('Invalid Offset 0x{0:08X}'.format(ofs))
                if d31 not in (0x02, 0x20, 0x21, 0x30):
                    warn('Unknown Code 0x01{1:02X} Offset 0x{0:08X}'.format(ofs, d31))
                ofs += 3
                while d32 != 0x00:
                    try:
                        d32, = struct.unpack_from('B', b3, ofs)
                    except Exception:
                        raise Exception('Content Truncated')
                    ofs += 1
                self.d3.append(b3[d2[i]: ofs])
            if ofs < len(b3):
                warn('Unaccessible Fragment Offset 0x{0:08X}'.format(ofs))
        # else:
        #     self.create_empty()
        # else:
        #     self.h1 = self.h2 = self.h3 = self.n1 = 0
        #     self.b1 = b''
        #     self.d3 = []


    def iter(self):
        self.idx = -1
        self.fslc = False

    def next(self, skp=True):
        self.idx += 1
        if skp:
            while self.idx < self.n1:
                if self.fslc:
                    break
                d31, = struct.unpack_from('B', self.d3[self.idx], 1)
                if d31 in (0x20, 0x21):
                    break
                # if d31 == 0x30 and self.d3[self.idx][2:8] == b'scene\x20':
                #     break
                if d31 == 0x30 and self.d3[self.idx][2:6] == b'str\x20':
                    break
                if d31 == 0x30 and self.d3[self.idx][2:] == b'fselect\x00':
                    self.fslc = True
                self.idx += 1
        if self.idx >= self.n1:
            raise StopIteration

    def get(self, skp=True):
        if skp:
            return self.d3[self.idx][2:-1]
        else:
            return b'<\\x01><\\x' + bytes('{0:02X}'.format(self.d3[self.idx][1]), encoding='utf-8') + b'>' + self.d3[self.idx][2:-1] + b'<\\x00>'

    def rep(self, bn):
        self.d3[self.idx] = self.d3[self.idx][:2] + bn + b'\x00'

    def pac(self):
        b2, b3 = b'', b''
        ofs = 0
        for i in range(self.n1):
            b2 += struct.pack('I', ofs)
            b3 += self.d3[i]
            ofs += len(self.d3[i])
        b0 = struct.pack('4I', self.h3 + ofs, self.h1, self.h2, self.h3)
        return b0 + self.b1 + b2 + b3

    def create_empty(self):
        self.h1 = 0
        self.h2 = 0
        self.h3 = 0
        self.b1 = b''
        self.d3 = []

    def add_line(self, control_code, content):
        control_byte = bytes([int(control_code, 16)])
        line_data = b'\x01' + control_byte + content + b'\x00'
        self.d3.append(line_data)
        self.h1 += 1
        self.h2 += len(line_data)
        self.h3 += len(line_data)


pathcst = 'scene_cst'
pathbin = 'scene_bin'
pathtxt = 'scene_txt'
pathdst = 'scene_dst'


def depacst():
    liscst = os.listdir(pathcst)
    if not os.path.exists(pathbin):
        os.makedirs(pathbin)
    s0, s1 = 0, 0
    for sc in liscst:
        if not sc.endswith('.cst'):
            continue
        s1 += 1
        clearWarnings()

        f = open(os.path.join(pathcst, sc), 'rb')
        try:
            b = cst2bin(f.read())
        except Exception as e:
            print('[ERROR]', sc, e)
            continue
        finally:
            f.close()

        sb = sc[:-3] + 'bin'
        f = open(os.path.join(pathbin, sb), 'wb')
        f.write(b)
        f.close()

        s0 += 1
        printWarnings(sc)
    return (s0, s1)


def unpacst(skp=True):
    liscst = os.listdir(pathcst)
    if not os.path.exists(pathtxt):
        os.makedirs(pathtxt)
    s0, s1 = 0, 0
    for sc in liscst:
        if not sc.endswith('.cst'):
            continue
        s1 += 1
        clearWarnings()

        f = open(os.path.join(pathcst, sc), 'rb')
        try:
            c = FormatCST(f)
        except Exception as e:
            print('[ERROR]', sc, e)
            continue
        finally:
            f.close()

        st = sc[:-3] + 'txt'
        f = open(os.path.join(pathtxt, st), 'wb')
        c.iter()
        while True:
            try:
                c.next(skp)
                f.write(c.get(skp))
                f.write(b'\x0D\x0A')
            except StopIteration:
                break
        f.close()

        s0 += 1
        printWarnings(sc)
    return (s0, s1)


def repacst(skp=True):
    liscst = os.listdir(pathcst)
    listxt = os.listdir(pathtxt)
    if not os.path.exists(pathdst):
        os.makedirs(pathdst)
    s0, s1 = 0, 0
    for st in listxt:
        if not st.endswith('.txt'):
            continue
        sc = st[:-3] + 'cst'
        if sc not in liscst:
            print('[WARNING] Original CST File Missing: ' + sc)
            continue
        s1 += 1
        clearWarnings()

        f = open(os.path.join(pathcst, sc), 'rb')
        try:
            c = FormatCST(f)
        except Exception as e:
            print('[ERROR]', sc, e)
            continue
        finally:
            f.close()

        st = sc[:-3] + 'txt'
        f = open(os.path.join(pathtxt, st), 'rb')
        c.iter()
        flag = True
        s = 0
        if skp:
            while True:
                try:
                    c.next(skp)
                    bn = b''
                    # print("Current c before replacement:", c.get(skp=False))
                    while True:
                        bi = f.read(1)
                        di, = struct.unpack('B', bi)
                        if di < 0x20:
                            break
                        bn += bi
                    if di != 0x0D:
                        flag = False
                        break
                    bi = f.read(1)
                    di, = struct.unpack('B', bi)
                    if di != 0x0A:
                        flag = False
                        break
                    c.rep(bn)
                    s += 1
                    # print("Current c after replacement:", c.get(skp=False))
                except StopIteration:
                    break
                except Exception:
                    warn('Lack of Text')
                    break
            f.close()
            if not flag:
                print('[ERROR]', st, 'Invalid Byte 0x{0:02X}'.format(di))
                continue
        else:
            while True:
                try:
                    c.next(skp)
                    line = f.readline()
                    if not line:
                        break
                    # print("Current c before replacement:", c.get(skp=False))
                    # Only process lines with specific control codes
                    if line.startswith(b'<\\x01><\\x30>'):
                        # content = re.search(rb'<\\x01><\\x30>(.*)<\\x00>', line)
                        # content = re.search(rb'<\\x01><\\x30>(.*?scene .+?)<\\x00>', line)
                        content = re.search(rb'<\\x01><\\x30>(.*?str .+?)<\\x00>', line)
                        if content:
                            print(content.group(1).decode('gbk'))
                            c.rep(content.group(1))
                        else:
                            # If the first regular expression fails, try to match the second one
                            content = re.search(rb'<\\x01><\\x30>(\d+ .+?)<\\x00>', line)
                            if content:
                                print(content.group(1).decode('gbk'))
                                c.rep(content.group(1))
                            # else:
                            #     content = re.search(rb'<\\x01><\\x30>(.*?str .+?)<\\x00>', line)
                            #     if content:
                            #         print(content.group(1).decode('gbk'))
                            #         c.rep(content.group(1))
                    # Process lines with <\x01><\x21> or <\x01><\x20>
                    elif line.startswith(b'<\\x01><\\x21>') or line.startswith(b'<\\x01><\\x20>'):
                        content = re.search(rb'<\\x01><\\x..>(.*)<\\x00>', line)
                        if content:
                            # print(content.group(1).decode('gbk'))
                            c.rep(content.group(1))
                    # print("Current c after replacement:", c.get(skp=False))
                except StopIteration:
                    break
                except Exception as e:
                    warn('Error processing line: ' + str(e))
                    break
            f.close()

        # print("Final c before writing to file:", c.pac())
        f = open(os.path.join(pathdst, sc), 'wb')
        f.write(bin2cst(c.pac()))
        f.close()

        s0 += 1
        printWarnings(sc)
    return (s0, s1)

if __name__ == '__main__':
    liscst = os.listdir(pathcst)
    if len(sys.argv) >= 2:
        tag = int(sys.argv[1])
    else:
        tag = 4
    if tag not in (0, 1, 2, 3, 4):
        print('Invalid Parametre')
        sys.exit()
    if tag == 0:
        s0, s1 = unpacst()
    if tag == 1:
        s0, s1 = repacst()
    if tag == 2:
        s0, s1 = depacst()
    if tag == 3:
        s0, s1 = unpacst(False)
    if tag == 4:
        s0, s1 = repacst(False)
    print('%d / %d completed' % (s0, s1))