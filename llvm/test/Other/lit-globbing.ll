<<<<<<< HEAD
RUN: echo TA > %T/TA.txt
RUN: echo TB > %T/TB.txt
RUN: echo TAB > %T/TAB.txt

RUN: echo %T/TA* | FileCheck -check-prefix=STAR %s
RUN: echo %T/'TA'* | FileCheck -check-prefix=STAR %s
RUN: echo %T/T'A'* | FileCheck -check-prefix=STAR %s

RUN: echo %T/T?.txt | FileCheck -check-prefix=QUESTION %s
RUN: echo %T/'T'?.txt | FileCheck -check-prefix=QUESTION %s

RUN: echo %T/T??.txt | FileCheck -check-prefix=QUESTION2 %s
RUN: echo %T/'T'??.txt | FileCheck -check-prefix=QUESTION2 %s

RUN: echo 'T*' 'T?.txt' 'T??.txt' | FileCheck -check-prefix=QUOTEDARGS %s

STAR-NOT: TB.txt
STAR: {{(TA.txt.*TAB.txt|TAB.txt.*TA.txt)}}

QUESTION-NOT: TAB.txt
QUESTION: {{(TA.txt.*TB.txt|TB.txt.*TA.txt)}}

QUESTION2-NOT: TA.txt
QUESTION2-NOT: TB.txt
QUESTION2: TAB.txt

QUOTEDARGS-NOT: .txt
QUOTEDARGS: T* T?.txt T??.txt
=======
RUN: echo XXA > %T/XXA.txt
RUN: echo XXB > %T/XXB.txt
RUN: echo XXAB > %T/XXAB.txt

RUN: echo %T/XXA* | FileCheck -check-prefix=STAR %s
RUN: echo %T/'XXA'* | FileCheck -check-prefix=STAR %s
RUN: echo %T/XX'A'* | FileCheck -check-prefix=STAR %s

RUN: echo %T/XX?.txt | FileCheck -check-prefix=QUESTION %s
RUN: echo %T/'XX'?.txt | FileCheck -check-prefix=QUESTION %s

RUN: echo %T/XX??.txt | FileCheck -check-prefix=QUESTION2 %s
RUN: echo %T/'XX'??.txt | FileCheck -check-prefix=QUESTION2 %s

RUN: echo 'XX*' 'XX?.txt' 'XX??.txt' | FileCheck -check-prefix=QUOTEDARGS %s

STAR-NOT: XXB.txt
STAR: {{(XXA.txt.*XXAB.txt|XXAB.txt.*XXA.txt)}}

QUESTION-NOT: XXAB.txt
QUESTION: {{(XXA.txt.*XXB.txt|XXB.txt.*XXA.txt)}}

QUESTION2-NOT: XXA.txt
QUESTION2-NOT: XXB.txt
QUESTION2: XXAB.txt

QUOTEDARGS-NOT: .txt
QUOTEDARGS: XX* XX?.txt XX??.txt
>>>>>>> 5a321ee929f41e6ab136995f11c2d4e48e625077
