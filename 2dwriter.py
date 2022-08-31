player_frames='stille-0 slag-0 slag-1 slag-2 slag-3 block-0 nede-0'.split()

player_name='nisse'
player_name2 = player_name
if player_name=='nisse':
    player_name2='nisse-ske'

some_cols={} # Nisse
some_cols['light red']=(0xba, 0x3e, 0x0c)
some_cols['dark red']=(0x9d, 0x34, 0x0d)
some_cols['light brown']=(0xb7, 0x9c, 0x42)
some_cols['dark brown']=(0xa8, 0x7e, 0x24)
some_cols['skin']=(0xc6, 0xac, 0x5d)
some_cols['light white']=(0xbf, 0xc4, 0xcf)
some_cols['dark white']=(0xa8, 0xab, 0xae)
some_cols['eye']=(0x90, 0x9f, 0xae)
some_cols['light jumper']=(0xaf, 0x99, 0xa7)
some_cols['dark jumper']=(0x8a, 0x6f, 0x71)
some_cols['light shoe']=(0xa3, 0x85, 0x38)
some_cols['dark shoe']=(0x9d, 0x6c, 0x67)
some_cols['green']= (0x00, 0xff, 0x00)

if (0):
    some_cols={} # Gullerod
    some_cols['light orange']=(0xf7, 0x58, 0x3d)
    some_cols['dark orange']= (0xe9, 0x55, 0x3b)
    some_cols['light gold']=(0xd8, 0xc5, 0x00)
    some_cols['dark gold']= (0xc0, 0xa9, 0x01)
    some_cols['green']=(0x31, 0xd8, 0x00)
    some_cols['black']=(0x4b, 0x3a, 0x45)
    some_cols['blue']=(0x00, 0x00, 0xff)
    some_cols['magenta']=(0xff, 0x00, 0xff)

vert_counts=[]
tri_counts=[]


cols2names={}
for col_name in some_cols:
    col_value=some_cols[col_name]
    cols2names[col_value]=col_name

def findCol(r,g,b):
    guess_col = None
    guess_error = 255*255*3
    for testcol in cols2names:
        delta=(pow(r-testcol[0],2)+pow(g-testcol[1],2)+pow(b-testcol[2],2))
        if delta<guess_error:
            guess_col=testcol
            guess_error=delta
    return guess_col

def maybeReplaceCol(candidate, replacements):
    for i in replacements:
        if candidate==i[0]:
            return i[1]
    return candidate

def read_frame(frame_name, frame_id=None):
    if not frame_id:
        frame_id=frame_name.lower().partition(':')[2].replace('-','_').replace('.ply','')
    frame_name=frame_name.replace(':','-')
    frame_id='_'+frame_id
    
    vert_props_raw=[]
    verts={}
    split_values={} # values split into parts, before quantizing
    faces=[]
    inPly=''
    vert_count=None
    face_count=None
    state='START'
    counted_verts=0
    counted_faces=0


    if ('light jumper' in some_cols):
        player_1_replacements=[(some_cols['green'],
                                some_cols['light red'])]
        player_2_replacements=player_1_replacements+[(some_cols['light red'],
                                                      some_cols['light jumper']),
                                                     (some_cols['dark red'],
                                                      some_cols['dark jumper']),
                                                     (some_cols['light jumper'],
                                                      some_cols['light red']),
                                                     (some_cols['dark jumper'],
                                                      some_cols['dark red'])]
    else:
        player_1_replacements=[]
        player_2_replacements=[]

    general_factor=40
    if 'gullerod' in frame_id:
        general_factor = 50

    cats=[('colour', ('red','green','blue'))]
    icats={}
    for i in cats:
        for j in i[1]:
            icats[j]=i[0]
        split_values[i[0]]=[]

    with open(frame_name) as fin:
        inPly=fin.read()

    for line in inPly.splitlines():
        if state=='START':
            if line=='ply':
                state='GOT_PLY'
        elif state=='GOT_PLY':
            if line.startswith('comment '):
                pass
            elif line.startswith('format '):
                pass
            elif line.startswith ('element '):
                words=line.split()
                if words[1]=='vertex':
                    state='GET_VERT_PROPS'
                    vert_count=int(words[2])
                elif words[1]=='face':
                    state='GET_FACE_PROPS'
                    face_count=int(words[2])
            else:
                raise ValueError('Line in header starting with '+line.split()[0])
        elif state=='GET_VERT_PROPS':
            if line.startswith ('element '):
                words=line.split()
                if words[1]=='face':
                    if face_count!=None:
                        raise ValueError('Face count was set already')
                    state='GET_FACE_PROPS'
                    face_count=int(words[2])
                elif words[1]=='vertex':
                    raise ValueError ('Duplicate vertex count')
            elif line.startswith('property '):
                words=line.split()
                if len(words)==3:
                    vert_props_raw.append((words[2], words[1]))
                    if words[2] not in icats:
                        verts[words[2]]=[]
        elif state=='GET_FACE_PROPS':
            if line.startswith ('element '):
                words=line.split()
                if words[1]=='vertex':
                    if vert_count!=None:
                        raise ValueError('Vertex count was set already')
                    state='GET_FACE_PROPS'
                    vert_count=int(words[2])
                elif words[1]=='face':
                    raise ValueError ('Duplicate face count')
            elif line.startswith('property '):
                pass # maybe I should check this
            elif line=='end_header':
                state='GET_ALL_VERTS'
        elif state=='GET_ALL_VERTS':
            words=line.split()
            if len(words)!=len(vert_props_raw):
                raise IndexError('Vertex was expected to have '+
                                 str(len(vert_props_raw))+
                                 ' properties but had '+
                                 str(len(words))+
                                 ' instead')
            named_values={}
            for n in range(len(words)):
                raw_value=words[n]
                new_value=raw_value
                prop_name=vert_props_raw[n][0]
                prop_type=vert_props_raw[n][1]
                if prop_type=='float':
                    new_value=round(float(raw_value)*general_factor)
                    if prop_name=='z':
                        new_value=round(float(raw_value)*60)
                else:
                    new_value=int(raw_value)
                if prop_name not in icats:
                    verts[prop_name].append(new_value)
                else:
                    named_values[prop_name]=new_value
            for category in cats:
                cat_name=category[0]
                elem_values=[]
                for elem in category[1]:
                    elem_val=named_values[elem]
                    elem_values.append(elem_val)
                split_values[cat_name].append(findCol(elem_values[0], elem_values[1], elem_values[2]))
            counted_verts+=1
            if counted_verts>=vert_count:
                state='GET_ALL_FACES'
        elif state=='GET_ALL_FACES':
            words=line.split()
            local_vertcount=len(words)-1
            if(local_vertcount!=int(words[0])):
                print(line)
                raise IndexError('Undecodabale face: mismatched length')
            face_verts=tuple(int(i) for i in words[1:])
            faces.append(face_verts)

    #print(len(set(split_values['colour'])))
    palette={}
    ipal=[]
    colour_count=0
    for col in sorted(list(set(split_values['colour']))):
        palette[col]=colour_count
        ipal.append(col)
        colour_count+=1

    col_indices=[]
    for col in split_values['colour']:
        col_indices.append(palette[col])


    #for i in ipal:
    #    print(str(i)+',')


    new_verts=[]
    for i in range(len(verts['x'])):
        new_x=verts['x'][i]
        new_y=verts['y'][i]
        new_z=verts['z'][i]
        new_col=col_indices[i]
        new_pos=i
        new_verts.append((new_z, new_col, new_y, new_x, new_pos))

    new_verts=sorted(new_verts)
    new_z_values=[i[0] for i in new_verts]
    new_col_values=[i[1] for i in new_verts]
    new_y_values=[i[2] for i in new_verts]
    new_x_values=[i[3] for i in new_verts]

    old2new_verts={}

    for new in range(len(new_verts)):
        old=new_verts[new][4]
        old2new_verts[old]=new

    new_faces=[]
    for face in faces:
        new_face=[old2new_verts[i] for i in face]
        new_faces.append(tuple(new_face))
    new_faces=sorted(new_faces)
    #print(min(new_x_values))

    output_c=''

    #print ('start x')
    output_c+=('uint8_t '+player_name+'_verts_x'+
               frame_id+'[]={'+(str(new_x_values))[1:-1]+'};\n\n')
    #print ('end x\n\nstart y')
    output_c+=('uint8_t '+player_name+'_verts_y'+
               frame_id+'[]={'+(str(new_y_values))[1:-1]+'};\n\n')
    output_c+=('// uint8_t '+player_name+'_verts_z'+
               frame_id+'[]={'+(str(new_z_values))[1:-1]+'};\n\n')
    #print ('end y\n\nstart red')

    out_faces_c='uint8_t '+player_name+'_trirefs'+frame_id+'[]={'
    line_tris=0
    triangle_separators=['\n','    ', '    ','    ']
    for i in new_faces:
        out_faces_c+=(triangle_separators[line_tris])+(','.join((str(n) for n in i))+',')
        line_tris+=1
        if (line_tris>=len(triangle_separators)):
            line_tris=0
    out_faces_c=out_faces_c[:-1]+'\n};\n'


    out_colrefs='uint8_t '+player_name+'_colrefs'+frame_id+'[]={\n'
    out_colrefs+=', '.join([str(i) for i in new_col_values])
    out_colrefs+='\n};\n'

    out_palette=''
    ipal2=[maybeReplaceCol(i, player_1_replacements) for i in ipal]
    if 'stille-0' in frame_name:#frame_id=='_stille_0':
        out_palette+='uint8_t '+player_name+'_red[]={'+(str([i[0] for i in ipal2])[1:-1])+'};\n\n'
        out_palette+='uint8_t '+player_name+'_green[]={'+(str([i[1] for i in ipal2])[1:-1])+'};\n\n'
        out_palette+='uint8_t '+player_name+'_blue[]={'+(str([i[2] for i in ipal2])[1:-1])+'};\n\n'
        ipal3=[maybeReplaceCol(i, player_2_replacements) for i in ipal]
        out_palette+='uint8_t '+player_name+'_2_red[]={'+(str([i[0] for i in ipal3])[1:-1])+'};\n\n'
        out_palette+='uint8_t '+player_name+'_2_green[]={'+(str([i[1] for i in ipal3])[1:-1])+'};\n\n'
        out_palette+='uint8_t '+player_name+'_2_blue[]={'+(str([i[2] for i in ipal3])[1:-1])+'};\n\n'
    print(frame_name)

    #print(output_c)
    #print(out_faces_c)
    #print(out_colrefs)

    ipal2=[maybeReplaceCol(i, player_1_replacements) for i in ipal]
    output_ini='start red\n'+(str([i[0] for i in ipal])[1:-1])+'\nend red\n\n'
    output_ini+='start green\n'+(str([i[1] for i in ipal])[1:-1])+'\nend green\n\n'
    output_ini+='start blue\n'+(str([i[2] for i in ipal])[1:-1])+'\nend blue\n\n'

    colours_rled=[]
    prev_col=None
    prev_count=0
    for col in new_col_values:
        if col==prev_col: prev_count+=1
        elif prev_col:
            colours_rled.append((prev_count, prev_col))#+=[prev_count, prev_col]
            prev_count=1
        prev_col=col


    lengths_out=('int '+player_name+'_vert_count'+frame_id+'='+str(len(verts['x']))+
                 ';\nint '+player_name+'_tri_count'+frame_id+'='+str(len(faces))+';')
    vert_counts.append(len(verts['x']))
    tri_counts.append(len(faces))

    frame_description=frame_id + ': ' + frame_name + '\n\n'
    
    out_all=(out_palette +
             '// '+frame_description +
             output_c + out_faces_c + out_colrefs + lengths_out+
             '\n\n')
    return((out_all, output_ini))

if __name__=='__main__':
    frame_data_c=''
    for i in player_frames:
        #frame='nisse/nisse-ske:'+i+'.ply'
        frame=player_name+'/'+player_name2+':'+i+'.ply'
        frame_data_c+=read_frame(frame)[0]
    #print(frame_data_c)
    with open ('nisse-ske-c.h', 'w') as f: f.write(frame_data_c)
