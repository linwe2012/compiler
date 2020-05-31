import os
import re
pattern = re.compile(r'^[0-9]\.')
outfile = './merge.md'

with open(outfile, 'wt', encoding='utf-8') as ofs:
    ofs.write('**Table of Contents**\n[TOC]\n\n')
    for f in os.listdir('.'):
        if pattern.search(f) != None:
            print('found file: ', f)
            with open(f, 'rt', encoding='utf-8') as ifs:
                ofs.write(ifs.read())
                ofs.write('\n')

        