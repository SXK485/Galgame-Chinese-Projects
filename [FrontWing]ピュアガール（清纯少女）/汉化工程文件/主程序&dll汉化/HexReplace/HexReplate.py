import csv
import codecs
import os

def find_and_replace(file_path, csv_path):
    # Load replacements from CSV
    replacements = []
    with open(csv_path, newline='', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            jp_hex = codecs.encode(row['JP_Name'].encode('shift_jis'), 'hex').decode()
            cn_hex = codecs.encode(row['CN_Name'].encode('gbk'), 'hex').decode()
            pad_hex = row.get("Padding", '00')  # 获取 Padding 字段，如果没有则默认为 '00'
            cn_hex += pad_hex * ((len(jp_hex) - len(cn_hex)) // 2)

            
            if len(jp_hex) < len(cn_hex):
                print(f"Warning: {row['CN_Name']} ({cn_hex}) is longer than {row['JP_Name']} ({jp_hex})")
            else:
                # Pad CN hex to have same length as JP hex
                # cn_hex = cn_hex.ljust(len(jp_hex), pad_char)
                replacements.append((jp_hex, cn_hex))
    
    # Load binary file
    with open(file_path, 'rb') as f:
        data = f.read()
    
    # Perform replacements
    data_hex = codecs.encode(data, 'hex').decode()
    for jp_hex, cn_hex in replacements:
        data_hex = data_hex.replace(jp_hex, cn_hex)
    
    # Save modified data back to file
    modified_data = codecs.decode(data_hex.encode(), 'hex')
    with open(file_path, 'wb') as f:
        f.write(modified_data)

# Test this function
# find_and_replace('cs2conf.dll', 'translations.csv')
find_and_replace('puregirl_cn.exe', 'translations.csv')