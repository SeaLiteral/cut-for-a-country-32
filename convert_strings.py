import sys

language_ids='da en es'.split()
translations={}
escape_characters={'n':'\n','t':'\t'}
#base_name='translations/{NAME}.po'
m=[]

base_s_file = '''\
.section .data

.global translation_contents
translation_contents:
    .incbin "{FILENAME}"'''

def readCString(text):
    state='OUTSIDE'
    quote='"'
    result = ''
    for ch in text:
        if state=='OUTSIDE':
            if ch in ' \t':
                pass
            elif ch in '"\'':
                state='INSIDE'
                quote=ch
            elif ch=='#':
                state='COMMENT'
            else:
                raise ValueError ('Unknown character outside string: '+str(ch))
        elif state=='INSIDE':
            if ch=='\\':
                state='ESCAPED'
            elif ch==quote:
                state='OUTSIDE'
            else:
                result+=ch
        elif state=='ESCAPED':
            if ch in escape_characters:
                result+=escape_characters[ch]
            else:
                result+=ch
            state='INSIDE'
        elif state=='COMMENT':
            pass
        else:
            raise ValueError ('Unknown state: '+str(state))
    return result
                
def decodePo():
    # WIP PO decoder: it can't really handle plurals, genders or cases
    for language_id in language_ids:
        text=''
        with open (base_name.replace('{NAME}',language_id)) as f:
            text=f.read()
        translations[language_id]={}
        label_id=''
        label_value=''
        label_type=''
        for line in text.splitlines():
            if line.startswith('#'):
                pass
            elif line.startswith ('msgid '):
                #translations[language_id][label_key]=label_value
                label_type='msgid'
                label_key=readCString(line.partition(' ')[2])
            elif line.startswith('msgstr '):
                label_type='msgstr'
                label_value=readString(line.partition(' ')[2])
                translations[language_id][label_key]=label_value
            elif label_type=='msgid':
                label_key += readCString(line)
            elif label_type=='msgstr':
                label_value += readCString(line)
                translations[language_id][label_key]=label_value
        #translations[language_id][label_ley]=label_value

reps_text='''\
á a´
é e´
í i´
ó o´
ú u´
ñ n~
æ [
ø $
å a]'''.upper()

reps_dict={}
for i in reps_text.splitlines():
    parts=i.split(' ')
    reps_dict[parts[0]]=parts[1]

def encodeString(s):
    r=b''
    for i in s:
        if i in reps_dict:
            r += reps_dict[i].encode('Latin 1')
        elif i in ' ABCDEFGHIJKLMNOPQRSTUVWXYZ\n0123456789':
            r += i.encode('Latin 1')
        else:
            raise ValueError ('Character '+str(i) +
                              ' might be missing in the font!')
    return r

def decodeLanguageMess(fname, allowed_conditions=[]):
    text = ''
    language_names=[]
    translations['']={}
    current_label=''
    enable_strings=True
    with open (fname) as f:
        text=f.read()
    for line in text.splitlines():
        # print (line)
        if line.startswith('#') or line.isspace() or (line==''):
            continue
        if line.startswith ('.lang_'):
            parts=line.partition ('_')[2].partition(' ')
            translations[parts[0]]={}
            language_names.append((parts[0], readCString(parts[2])))
        elif line.startswith ('.'):
            enable_strings=True
            parts=line.partition (' ')
            current_label=parts[0][1:]
            if parts[2]: # conditional string
                conditions=parts[2].split()
                for i in conditions:
                    print(i)
                    found_condition = False
                    for j in i.split('|'):
                        if ((i[1:] not in allowed_conditions) and (i[0]=='!')):
                            found_condition = True
                        if ((i[1:] in allowed_conditions) and (i[0]=='?')):
                            found_condition = True
                    if not found_condition:
                        enable_strings=False
            current_language=''
            # print('set language')
        elif '=' in line and enable_strings:
            parts=line.partition('=')
            current_language = parts[0].strip()
            translations[current_language][current_label]=readCString(parts[2])
        elif enable_strings:
            translations[current_language][current_label]+=readCString(line)
    for target_language in translations:
        if target_language:
            for language_tuple in language_names:
                lang_id='lang_'+language_tuple[0]
                lang_name=language_tuple[1]
                translations[target_language][lang_id]=lang_name

def encodeArrayAndH():
    global m
    count_characters=0
    global out_h
    out_h = ''
    global out_b
    i=None
    out_b=[b'']*3 # number of languages
    for text_id in translations['da']:
        tr_da = encodeString (translations['da'][text_id])
        tr_en = encodeString (translations['en'][text_id])
        tr_es = encodeString (translations['es'][text_id])
        m.append((text_id,
                  count_characters,
                  tr_da,
                  tr_en,
                  tr_es))
        longest_length = max((len(i) for i in (tr_da, tr_en, tr_es)))+1
        if ( (longest_length==1) and (count_characters>=1) ):
            out_h+= '#define UI_'+text_id.upper()+' '+str(count_characters)+'\n'
        else:
            out_b[0]+=tr_da+ (b'\0'*(longest_length-len(tr_da)))
            out_b[1]+=tr_en+ (b'\0'*(longest_length-len(tr_en)))
            out_b[2]+=tr_es+ (b'\0'*(longest_length-len(tr_es)))
            out_h+= '#define UI_'+text_id.upper()+' '+str(count_characters)+'\n'
            count_characters += longest_length
    out_h += 'const int language_size = '+str(count_characters)+';\n'
    out_h += 'extern char translation_contents;\n'

def main ():
    last_arg=sys.argv[-1]
    fname_bin='translations/interface.bin'
    fname_h='translations/interface.h'
    fname_s='translations/interface.s'
    print(sys.argv)
    if (('python' not in last_arg) and ('.py' not in last_arg)):
        fname_bin = last_arg+'.bin'
        fname_h = last_arg+'.h'
        fname_s = last_arg+'.s'
    decodeLanguageMess('translations/interface.ini')
    encodeArrayAndH()
    print(''.join((i.decode('Latin 1').replace('\0','%') for i in out_b)))
    with open(fname_bin, 'wb') as f:
        f.write(b''.join(out_b))
    with open(fname_h, 'w') as f:
        f.write(''.join(out_h))
    out_s = base_s_file.replace('{FILENAME}', fname_bin)
    with open(fname_s, 'w') as f:
        f.write(''.join(out_s))

if __name__=='__main__':
    main()
