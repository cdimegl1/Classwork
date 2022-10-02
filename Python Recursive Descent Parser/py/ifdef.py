#!/usr/bin/env python3

import re
import sys
from collections import namedtuple
import json

'''
source
  : (TEXT | ifdef)*
  ;
ifdef
  : (IFDEF | IFNDEF) SYM
    source
    (ELIF SYM source)*
    (ELSE source)?
    ENDIF
  ;
'''

################################# Parser ################################

EOF = '$EOF'

def doParse(tokens):
    tokens.append([EOF, '<EOF>'])
    look = ''

    def error(msg):
        nonlocal look
        print(f'*** {msg} at {look[0]}')
        sys.exit(1)

    def advance():
        nonlocal look
        look = tokens.pop(0)

    def match(kind):
        nonlocal look
        if look[0] != kind:
            error(f'expecting {kind}')
        advance()

    def parse():
        nonlocal look
        asts = source()
        if look[0] != EOF: error('expecting EOF')
        return asts

    def source():
        nonlocal look
        asts = []
        while True:
            kind = look[0]
            if kind == 'TEXT':
                asts.append({ 'tag': 'TEXT', 'text': look[1]})
                match('TEXT')
            elif kind in [ 'IFDEF', 'IFNDEF' ]:
                asts.append(ifdef())
            else:
                break
        return asts

    def ifdef():
        nonlocal look
        defn = look[0]
        if not defn in ['IFDEF', 'IFNDEF']:
            error(f'expecting start of def at {defn}')
        match(defn)
        defSym = look[1]
        match('SYM')
        src = source()
        while look[0] == 'ELIF':
            match('ELIF')
            sym = look[1]
            match('SYM')
            elifSrc = source()
            src.append({'tag': 'ELIF', 'sym': sym, 'xkids': elifSrc})
        if look[0] == 'ELSE':
            match('ELSE')
            elseSrc = source()
            src.append({'tag': 'ELSE', 'xkids': elseSrc})
        match('ENDIF')
        return { 'tag': defn, 'sym': defSym, 'xkids': src }

    advance()
    return parse()

################################# Lexer #################################

def doScan(str):

    DEF_RE = re.compile(r'\s*\#(ifdef|ifndef|elif|else|endif)\b')
    SYM_DEFS = [ 'ifdef', 'ifndef', 'elif' ]
    SYM_RE = re.compile(r'\s+([_a-zA-Z]\w*)')

    toks = []
    text = ''
    str = re.sub(r'\n$', '', str)
    for line in str.split('\n'):
        mDef = DEF_RE.match(line)
        if mDef:
            if len(text) > 0:
                toks.append([ 'TEXT', text ])
                text = ''
            defn = mDef.group(1)
            toks.append([ defn.upper(), f'#{defn}' ])
            if defn in SYM_DEFS:
                mSym = SYM_RE.match(line[len(mDef.group()):])
                if mSym:
                    toks.append(['SYM', mSym.group(1)])
        else:
            text += line + '\n'
    if text:
        toks.append(['TEXT', text])
    return toks

def scan_stdin():
    return doScan(sys.stdin.read())

################################## Main #################################

def main():
    if len(sys.argv) != 2 or not sys.argv[1] in ['scan', 'parse']:
        print(f'usage: {sys.argv[0]} scan|parse\n', file=sys.stderr)
        sys.exit(1)
    tokens = scan_stdin()
    out = tokens if sys.argv[1] == 'scan' else doParse(tokens)
    print(json.dumps(out, separators=(',', ':'))) #no whitespace

if __name__ == "__main__":
    main()
