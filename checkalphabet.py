t='''W 18
mw 16
Æ 15
M 13
OQUVXY 12
ADEFGNZ 11
BCHJKLRSTq 10
Pabdeghnopsuvxyzåø 9
ck 8
frt 7
j 6
l 5
I 4
i 2'''
# TODO: add numbers, punctuation, and non-English characters

allChars=[]
sizes = {}
ch2siz= {}

for i in t.splitlines():
    siz = int(i.split()[1])
    if siz not in sizes:
        sizes[siz]=[]
    for ch in i.split()[0]:
        allChars.append(ch)
        sizes[siz].append(ch)
        ch2siz[ch]=siz

for i in range(ord('a'), ord('z')):
    if chr(i) not in allChars:print('Missing letter:',chr(i))

for i in range(ord('A'), ord('Z')):
    if chr(i) not in allChars:print('Missing letter:',chr(i))

for i in set(allChars):
    if allChars.count(i)>1:print ('Duplicate character:', i)

sortedSizes=sorted(list(sizes.keys()), reverse=True)
for i in sortedSizes:
    print(i, ''.join(sizes[i]))

print('\n'*5)

buildLines=['']*11
lineSizes = [0]*11
for sz in sortedSizes:
    print('adding characters of size',sz)
    for ch in sizes[sz]:
        shortestLine=0
        largestSize=65 # One more than the maximum possible
        for pos in range(len(lineSizes)):
            if lineSizes[pos]<largestSize:
                shortestLine = pos
                largestSize = lineSizes[pos]
        buildLines[shortestLine] += ch
        lineSizes [shortestLine] += ch2siz[ch]

for i in lineSizes:
    if i>=64:
        raise ValueError ('Not enough space in the font')

print('\n Lines:')
for line in buildLines:
    print(line)

lineCount=0
for line in buildLines:
    print (' line', lineCount)
    pixelCount=0
    for ch in line:
        print (ch, 'on', lineCount, 'at', pixelCount, 'width', ch2siz[ch])
        pixelCount += ch2siz[ch]
    lineCount+=1

